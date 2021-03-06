//
//  yas_chaining_private.h
//

#pragma once

#include <chaining/yas_chaining_observer.h>
#include <cpp_utils/yas_fast_each.h>

#include <optional>
#include <vector>

namespace yas::chaining {
template <typename Out, typename Begin, bool Syncable>
struct chain<Out, Begin, Syncable>::impl {
    joint_ptr<Begin> _joint;

    impl(joint_ptr<Begin> &&joint) : _joint(std::move(joint)) {
    }

    chain<Out, Begin, Syncable> perform(std::function<void(Out const &)> &&perform_handler) {
        std::size_t const next_idx = this->_joint->handlers_size() + 1;

        auto handler = [next_idx, perform_handler = std::move(perform_handler)](Out const &value, any_joint &joint) {
            perform_handler(value);
            if (next_idx < joint.handlers_size()) {
                if (auto const &next_handler = joint.handler<Out>(next_idx)) {
                    next_handler(value, joint);
                }
            }
        };

        this->_joint->template push_handler<Out>(std::move(handler));

        return chain<Out, Begin, Syncable>(std::move(this->_joint));
    }

    template <typename F>
    chain<return_t<F>, Begin, Syncable> to(F &&to_handler) {
        auto &joint = this->_joint;
        std::size_t const next_idx = joint->handlers_size() + 1;

        auto handler = [next_idx, to_handler = std::move(to_handler)](Out const &value, any_joint &joint) mutable {
            return_t<F> next_value = to_handler(value);
            if (next_idx < joint.handlers_size()) {
                if (auto const &next_handler = joint.template handler<return_t<F>>(next_idx)) {
                    next_handler(next_value, joint);
                }
            }
        };

        joint->template push_handler<Out>(std::move(handler));

        return chaining::chain<return_t<F>, Begin, Syncable>(std::move(this->_joint));
    }

    template <typename T = Out, enable_if_tuple_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(chain<Out, Begin, Syncable> &&) {
        return chaining::chain<Out, Begin, Syncable>(std::move(this->_joint));
    }

    template <typename T = Out, enable_if_pair_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(chain<Out, Begin, Syncable> &&) {
        return this->to([](Out const &pair) { return std::make_tuple(pair.first, pair.second); });
    }

    template <typename T = Out, disable_if_tuple_t<T, std::nullptr_t> = nullptr,
              disable_if_pair_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(chain<Out, Begin, Syncable> &&) {
        return this->to([](Out const &value) { return std::make_tuple(value); });
    }

    template <std::size_t N, typename T, enable_if_base_of_receiver_t<T, std::nullptr_t> = nullptr,
              typename NonOut = Out, disable_if_tuple_t<NonOut, std::nullptr_t> = nullptr,
              disable_if_array_t<NonOut, std::nullptr_t> = nullptr>
    auto send_to(chaining::chain<Out, Begin, Syncable> &chain, std::shared_ptr<T> const &receiver) {
        return chain.perform([weak_receiver = to_weak(receiver)](Out const &value) {
            if (std::shared_ptr<T> receiver = weak_receiver.lock()) {
                receiver->receive_value(value);
            }
        });
    }

    template <std::size_t N, typename T, enable_if_base_of_receiver_t<T, std::nullptr_t> = nullptr,
              typename TupleOut = Out, enable_if_tuple_t<TupleOut, std::nullptr_t> = nullptr>
    auto send_to(chaining::chain<Out, Begin, Syncable> &chain, std::shared_ptr<T> const &receiver) {
        return chain.perform([weak_receiver = to_weak(receiver)](Out const &value) {
            if (std::shared_ptr<T> receiver = weak_receiver.lock()) {
                receiver->receive_value(std::get<N>(value));
            }
        });
    }

    template <std::size_t N, typename T, enable_if_base_of_receiver_t<T, std::nullptr_t> = nullptr,
              typename ArrayOut = Out, enable_if_array_t<ArrayOut, std::nullptr_t> = nullptr>
    auto send_to(chaining::chain<Out, Begin, Syncable> &chain, std::shared_ptr<T> const &receiver) {
        return chain.perform([weak_receiver = to_weak(receiver)](Out const &value) {
            if (std::shared_ptr<T> receiver = weak_receiver.lock()) {
                receiver->receive_value(std::get<N>(value));
            }
        });
    }

    template <typename T, std::size_t N, enable_if_base_of_receiver_t<T, std::nullptr_t> = nullptr>
    auto send_to(chaining::chain<Out, Begin, Syncable> &chain, std::array<std::shared_ptr<T>, N> const &receivers) {
        std::vector<std::weak_ptr<T>> weak_receivers;
        weak_receivers.reserve(N);

        auto each = make_fast_each(N);
        while (yas_each_next(each)) {
            auto const &idx = yas_each_index(each);
            weak_receivers.emplace_back(to_weak(receivers.at(idx)));
        }

        return chain.perform([weak_receivers = std::move(weak_receivers)](Out const &values) mutable {
            auto each = make_fast_each(N);
            while (yas_each_next(each)) {
                auto const &idx = yas_each_index(each);
                if (std::shared_ptr<T> receiver = weak_receivers.at(idx).lock()) {
                    receiver->receive_value(values.at(idx));
                }
            }
        });
    }

    template <typename T, enable_if_base_of_receiver_t<T, std::nullptr_t> = nullptr>
    auto send_to(chaining::chain<Out, Begin, Syncable> &chain, std::vector<std::shared_ptr<T>> const &receivers) {
        std::size_t const count = receivers.size();

        std::vector<std::weak_ptr<T>> weak_receivers;
        weak_receivers.reserve(count);

        auto each = make_fast_each(count);
        while (yas_each_next(each)) {
            auto const &idx = yas_each_index(each);
            weak_receivers.emplace_back(to_weak(receivers.at(idx)));
        }

        return chain.perform([weak_receivers = std::move(weak_receivers)](Out const &values) {
            std::size_t const count = std::min(values.size(), weak_receivers.size());
            auto each = make_fast_each(count);
            while (yas_each_next(each)) {
                auto const &idx = yas_each_index(each);
                if (std::shared_ptr<T> receiver = weak_receivers.at(idx).lock()) {
                    receiver->receive_value(values.at(idx));
                }
            }
        });
    }

    template <typename SubBegin, bool SubSyncable>
    chain<Out, Begin, Syncable | SubSyncable> merge(chain<Out, SubBegin, SubSyncable> &&sub_chain) {
        chaining::joint_ptr<Begin> &joint = this->_joint;
        auto weak_joint = to_weak(joint);
        std::size_t const next_idx = joint->handlers_size() + 1;

        auto &sub_joint = sub_chain._impl->_joint;

        sub_joint->template push_handler<Out>([weak_joint, next_idx](Out const &value, any_joint &) mutable {
            if (auto joint = weak_joint.lock()) {
                joint->template handler<Out>(next_idx)(value, *joint);
            }
        });

        joint->template push_handler<Out>([next_idx](Out const &value, any_joint &joint) mutable {
            joint.template handler<Out>(next_idx)(value, joint);
        });

        joint->add_sub_joint(std::move(sub_joint));

        return chain<Out, Begin, Syncable | SubSyncable>(joint);
    }

    template <typename SubOut, typename SubBegin, bool SubSyncable>
    chain<opt_pair_t<Out, SubOut>, Begin, Syncable | SubSyncable> pair(
        chain<SubOut, SubBegin, SubSyncable> &&sub_chain) {
        chaining::joint_ptr<Begin> &joint = this->_joint;
        auto weak_joint = to_weak(joint);
        std::size_t const next_idx = joint->handlers_size() + 1;

        auto &sub_joint = sub_chain._impl->_joint;

        sub_joint->template push_handler<SubOut>([weak_joint, next_idx](SubOut const &value, any_joint &) mutable {
            if (any_joint_ptr joint = weak_joint.lock()) {
                joint->template handler<opt_pair_t<Out, SubOut>>(next_idx)(opt_pair_t<Out, SubOut>{std::nullopt, value},
                                                                           *joint);
            }
        });

        joint->template push_handler<Out>([next_idx](Out const &value, any_joint &joint) mutable {
            joint.template handler<opt_pair_t<Out, SubOut>>(next_idx)(opt_pair_t<Out, SubOut>{value, std::nullopt},
                                                                      joint);
        });

        joint->add_sub_joint(std::move(sub_joint));

        return chain<opt_pair_t<Out, SubOut>, Begin, Syncable | SubSyncable>(std::move(joint));
    }

    template <typename SubOut, typename SubBegin, bool SubSyncable>
    auto combine_pair(chaining::chain<Out, Begin, Syncable> &chain,
                      chaining::chain<SubOut, SubBegin, SubSyncable> &&sub_chain) {
        return chain.pair(std::move(sub_chain))
            .to([opt_pair = opt_pair_t<Out, SubOut>{}](opt_pair_t<Out, SubOut> const &value) mutable {
                if (value.first) {
                    opt_pair.first = *value.first;
                }
                if (value.second) {
                    opt_pair.second = *value.second;
                }
                return opt_pair;
            })
            .guard([](opt_pair_t<Out, SubOut> const &pair) { return pair.first && pair.second; })
            .to([](opt_pair_t<Out, SubOut> const &pair) { return std::make_pair(*pair.first, *pair.second); });
    }

    template <typename SubOut, typename SubBegin, bool SubSyncable,
              disable_if_tuple_t<SubOut, std::nullptr_t> = nullptr, typename MainOut = Out,
              disable_if_tuple_t<MainOut, std::nullptr_t> = nullptr>
    auto combine(chaining::chain<Out, Begin, Syncable> &chain,
                 chaining::chain<SubOut, SubBegin, SubSyncable> &&sub_chain) {
        return this->combine_pair(chain, std::move(sub_chain));
    }

    template <typename SubOut, typename SubBegin, bool SubSyncable, enable_if_tuple_t<SubOut, std::nullptr_t> = nullptr,
              typename MainOut = Out, enable_if_tuple_t<MainOut, std::nullptr_t> = nullptr>
    auto combine(chaining::chain<Out, Begin, Syncable> &chain,
                 chaining::chain<SubOut, SubBegin, SubSyncable> &&sub_chain) {
        return this->combine_pair(chain, std::move(sub_chain)).to([](std::pair<Out, SubOut> const &pair) {
            return std::tuple_cat(static_cast<typename std::remove_const<Out>::type>(pair.first),
                                  static_cast<typename std::remove_const<SubOut>::type>(pair.second));
        });
    }

    chaining::observer_ptr<Begin> _end() {
        this->_joint->template push_handler<Out>([](Out const &, any_joint &) {});
        return observer<Begin>::make_shared(this->_joint);
    }

    chaining::observer_ptr<Begin> end() {
        return this->_end();
    }

    chaining::observer_ptr<Begin> fetch() {
        static_assert(Syncable, "Syncable must be true.");

        auto observer = this->_end();
        observer->fetch();
        return observer;
    }
};

template <typename Out, typename Begin, bool Syncable>
chain<Out, Begin, Syncable>::chain(joint_ptr<Begin> joint) : _impl(std::make_unique<chain::impl>(std::move(joint))) {
}

template <typename Out, typename Begin, bool Syncable>
chain<Out, Begin, Syncable> chain<Out, Begin, Syncable>::perform(std::function<void(Out const &)> perform_handler) {
    return this->_impl->perform(std::move(perform_handler));
}

template <typename Out, typename Begin, bool Syncable>
template <std::size_t N, typename T>
chain<Out, Begin, Syncable> chain<Out, Begin, Syncable>::send_to(std::shared_ptr<T> const &receiver) {
    return this->_impl->template send_to<N>(*this, receiver);
}

template <typename Out, typename Begin, bool Syncable>
template <typename T, std::size_t N>
[[nodiscard]] chain<Out, Begin, Syncable> chain<Out, Begin, Syncable>::send_to(
    std::array<std::shared_ptr<T>, N> const &receivers) {
    return this->_impl->template send_to<T, N>(*this, receivers);
}

template <typename Out, typename Begin, bool Syncable>
template <typename T>
chain<Out, Begin, Syncable> chain<Out, Begin, Syncable>::send_to(std::vector<std::shared_ptr<T>> const &receivers) {
    return this->_impl->template send_to<T>(*this, receivers);
}

template <typename Out, typename Begin, bool Syncable>
template <typename T>
chain<Out, Begin, Syncable> chain<Out, Begin, Syncable>::send_to(
    std::initializer_list<std::shared_ptr<T>> const &receivers) {
    std::vector<std::shared_ptr<T>> vector{receivers};
    return this->_impl->template send_to<T>(*this, vector);
}

template <typename Out, typename Begin, bool Syncable>
chain<Out, Begin, Syncable> chain<Out, Begin, Syncable>::send_null_to(std::shared_ptr<receiver<>> const &receiver) {
    return this->perform([weak_receiver = to_weak(receiver)](Out const &) {
        if (std::shared_ptr<chaining::receiver<>> receiver = weak_receiver.lock()) {
            receiver->receive_value(nullptr);
        }
    });
}

template <typename Out, typename Begin, bool Syncable>
chain<Out, Begin, Syncable> chain<Out, Begin, Syncable>::guard(std::function<bool(Out const &)> guard_handler) {
    auto &joint = this->_impl->_joint;
    std::size_t const next_idx = joint->handlers_size() + 1;

    auto handler = [next_idx, guard_handler = std::move(guard_handler)](Out const &value, any_joint &joint) {
        if (guard_handler(value)) {
            if (auto const &next_handler = joint.handler<Out>(next_idx)) {
                next_handler(value, joint);
            }
        }
    };

    joint->template push_handler<Out>(std::move(handler));

    return chain<Out, Begin, Syncable>(std::move(joint));
}

template <typename Out, typename Begin, bool Syncable>
template <typename F>
chain<return_t<F>, Begin, Syncable> chain<Out, Begin, Syncable>::to(F handler) {
    return this->_impl->to(std::move(handler));
}

template <typename Out, typename Begin, bool Syncable>
template <typename T>
chain<T, Begin, Syncable> chain<Out, Begin, Syncable>::to_value(T value) {
    return this->_impl->to([value = std::move(value)](Out const &) { return value; });
}

template <typename Out, typename Begin, bool Syncable>
chain<std::nullptr_t, Begin, Syncable> chain<Out, Begin, Syncable>::to_null() {
    return this->_impl->to([](Out const &) { return nullptr; });
}

template <typename Out, typename Begin, bool Syncable>
auto chain<Out, Begin, Syncable>::to_tuple() {
    return this->_impl->template to_tuple<>(std::move(*this));
}

template <typename Out, typename Begin, bool Syncable>
template <typename SubBegin, bool SubSyncable>
chain<Out, Begin, Syncable | SubSyncable> chain<Out, Begin, Syncable>::merge(
    chain<Out, SubBegin, SubSyncable> &&sub_chain) {
    return this->_impl->merge(std::move(sub_chain));
}

template <typename Out, typename Begin, bool Syncable>
template <typename SubOut, typename SubBegin, bool SubSyncable>
chain<opt_pair_t<Out, SubOut>, Begin, Syncable | SubSyncable> chain<Out, Begin, Syncable>::pair(
    chain<SubOut, SubBegin, SubSyncable> &&sub_chain) {
    return this->_impl->pair(std::move(sub_chain));
}

template <typename Out, typename Begin, bool Syncable>
template <typename SubOut, typename SubBegin, bool SubSyncable>
auto chain<Out, Begin, Syncable>::combine(chain<SubOut, SubBegin, SubSyncable> &&sub_chain) {
    return this->_impl->combine(*this, std::move(sub_chain));
}

template <typename Out, typename Begin, bool Syncable>
observer_ptr<Begin> chain<Out, Begin, Syncable>::end() {
    return this->_impl->end();
}

template <typename Out, typename Begin, bool Syncable>
observer_ptr<Begin> chain<Out, Begin, Syncable>::sync() {
    return this->_impl->fetch();
}
}  // namespace yas::chaining
