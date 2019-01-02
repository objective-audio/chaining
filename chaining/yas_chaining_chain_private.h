//
//  yas_chaining_private.h
//

#pragma once

#include <cpp_utils/yas_fast_each.h>
#include <optional>
#include <vector>
#include "yas_chaining_observer.h"
#include "yas_chaining_receiver.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
struct chain<Out, In, Begin, Syncable>::impl : base::impl {
    joint<Begin> _joint;
    std::function<Out(In const &)> _handler;

    impl(joint<Begin> &&joint, std::function<Out(In const &)> &&handler)
        : _joint(std::move(joint)), _handler(std::move(handler)) {
    }

    template <typename F>
    chain<return_t<F>, In, Begin, Syncable> to(F &&to_handler) {
        return chaining::chain<return_t<F>, In, Begin, Syncable>(
            std::move(this->_joint), [to_handler = std::move(to_handler), handler = std::move(this->_handler)](
                                         In const &value) mutable { return to_handler(handler(value)); });
    }

    template <typename T = Out, enable_if_tuple_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(chain<Out, In, Begin, Syncable> &chain) {
        return chain;
    }

    template <typename T = Out, enable_if_pair_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(chain<Out, In, Begin, Syncable> &chain) {
        return this->to([](Out const &pair) { return std::make_tuple(pair.first, pair.second); });
    }

    template <typename T = Out, disable_if_tuple_t<T, std::nullptr_t> = nullptr,
              disable_if_pair_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(chain<Out, In, Begin, Syncable> &) {
        return this->to([](Out const &value) { return std::make_tuple(value); });
    }

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable,
              disable_if_tuple_t<SubOut, std::nullptr_t> = nullptr, typename MainOut = Out,
              disable_if_tuple_t<MainOut, std::nullptr_t> = nullptr>
    auto tuple(chain<SubOut, SubIn, SubBegin, SubSyncable> &&sub_chain) {
        using opt_tuple_t = std::tuple<std::optional<Out>, std::optional<SubOut>>;

        chaining::joint<Begin> &joint = this->_joint;
        auto weak_joint = to_weak(joint);
        std::size_t const next_idx = joint.handlers_size() + 1;

        auto sub_imp = sub_chain.template impl_ptr<typename chain<SubOut, SubIn, SubBegin, SubSyncable>::impl>();
        auto &sub_joint = sub_imp->_joint;

        sub_joint.template push_handler<SubIn>(
            [handler = sub_imp->_handler, weak_joint, next_idx](SubIn const &value) mutable {
                if (auto joint = weak_joint.lock()) {
                    joint.template handler<opt_tuple_t>(next_idx)(opt_tuple_t(std::nullopt, handler(value)));
                }
            });

        joint.template push_handler<In>([handler = this->_handler, weak_joint, next_idx](In const &value) mutable {
            if (auto joint = weak_joint.lock()) {
                joint.template handler<opt_tuple_t>(next_idx)(opt_tuple_t(handler(value), std::nullopt));
            }
        });

        joint.add_sub_joint(std::move(sub_joint));

        return chain<opt_tuple_t, opt_tuple_t, Begin, Syncable | SubSyncable>(
            joint, [](opt_tuple_t const &value) { return value; });
    }

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    auto combine_pair(chaining::chain<Out, In, Begin, Syncable> &chain,
                      chaining::chain<SubOut, SubIn, SubBegin, SubSyncable> &&sub_chain) {
        using opt_pair_t = std::pair<std::optional<Out>, std::optional<SubOut>>;

        return chain.pair(std::move(sub_chain))
            .to([opt_pair = opt_pair_t{}](opt_pair_t const &value) mutable {
                if (value.first) {
                    opt_pair.first = *value.first;
                }
                if (value.second) {
                    opt_pair.second = *value.second;
                }
                return opt_pair;
            })
            .guard([](opt_pair_t const &pair) { return pair.first && pair.second; })
            .to([](opt_pair_t const &pair) { return std::make_pair(*pair.first, *pair.second); });
    }

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable,
              disable_if_tuple_t<SubOut, std::nullptr_t> = nullptr, typename MainOut = Out,
              disable_if_tuple_t<MainOut, std::nullptr_t> = nullptr>
    auto combine(chaining::chain<Out, In, Begin, Syncable> &chain,
                 chaining::chain<SubOut, SubIn, SubBegin, SubSyncable> &&sub_chain) {
        return this->combine_pair(chain, std::move(sub_chain));
    }

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable,
              enable_if_tuple_t<SubOut, std::nullptr_t> = nullptr, typename MainOut = Out,
              enable_if_tuple_t<MainOut, std::nullptr_t> = nullptr>
    auto combine(chaining::chain<Out, In, Begin, Syncable> &chain,
                 chaining::chain<SubOut, SubIn, SubBegin, SubSyncable> &&sub_chain) {
        return this->combine_pair(chain, std::move(sub_chain)).to([](std::pair<Out, SubOut> const &pair) {
            return std::tuple_cat(static_cast<typename std::remove_const<Out>::type>(pair.first),
                                  static_cast<typename std::remove_const<SubOut>::type>(pair.second));
        });
    }

    template <std::size_t N, typename T, typename TupleOut = Out, enable_if_tuple_t<TupleOut, std::nullptr_t> = nullptr>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, receiver<T> &receiver) {
        return chain.perform([weak_receiver = to_weak(receiver)](Out const &value) {
            if (chaining::receiver<T> receiver = weak_receiver.lock()) {
                receiver.receivable().receive_value(std::get<N>(value));
            }
        });
    }

    template <std::size_t N, typename T, typename ArrayOut = Out, enable_if_array_t<ArrayOut, std::nullptr_t> = nullptr>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, receiver<T> &receiver) {
        return chain.perform([weak_receiver = to_weak(receiver)](Out const &value) {
            if (chaining::receiver<T> receiver = weak_receiver.lock()) {
                receiver.receivable().receive_value(std::get<N>(value));
            }
        });
    }

    template <std::size_t N, typename T, typename NonOut = Out, disable_if_tuple_t<NonOut, std::nullptr_t> = nullptr,
              disable_if_array_t<NonOut, std::nullptr_t> = nullptr>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, receiver<T> &receiver) {
        return chain.perform([weak_receiver = to_weak(receiver)](Out const &value) {
            if (chaining::receiver<T> receiver = weak_receiver.lock()) {
                receiver.receivable().receive_value(value);
            }
        });
    }

    template <typename T, std::size_t N>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, std::array<receiver<T>, N> &receivers) {
        std::vector<weak<chaining::receiver<T>>> weak_receivers;
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
                if (chaining::receiver<T> receiver = weak_receivers.at(idx).lock()) {
                    receiver.receivable().receive_value(values.at(idx));
                }
            }
        });
    }

    template <typename T>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, std::vector<receiver<T>> &receivers) {
        std::size_t const count = receivers.size();

        std::vector<weak<chaining::receiver<T>>> weak_receivers;
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
                if (chaining::receiver<T> receiver = weak_receivers.at(idx).lock()) {
                    receiver.receivable().receive_value(values.at(idx));
                }
            }
        });
    }

    chaining::observer<Begin> _end() {
        this->_joint.template push_handler<In>([handler = this->_handler](In const &value) { handler(value); });
        return observer<Begin>(this->_joint);
    }

    chaining::observer<Begin> end() {
        return this->_end();
    }

    chaining::observer<Begin> broadcast() {
        static_assert(Syncable, "Syncable must be true.");

        auto observer = this->_end();
        observer.broadcast();
        return observer;
    }
};

template <typename Out, typename In, typename Begin, bool Syncable>
chain<Out, In, Begin, Syncable>::chain(joint<Begin> joint)
    : chain(std::move(joint), [](Begin const &value) { return value; }) {
}

template <typename Out, typename In, typename Begin, bool Syncable>
chain<Out, In, Begin, Syncable>::chain(joint<Begin> joint, std::function<Out(In const &)> handler)
    : base(std::make_shared<impl>(std::move(joint), std::move(handler))) {
}

template <typename Out, typename In, typename Begin, bool Syncable>
chain<Out, In, Begin, Syncable>::chain(std::nullptr_t) : base(nullptr) {
}

template <typename Out, typename In, typename Begin, bool Syncable>
chain<Out, In, Begin, Syncable>::~chain() = default;

template <typename Out, typename In, typename Begin, bool Syncable>
auto chain<Out, In, Begin, Syncable>::normalize() {
    auto imp = impl_ptr<impl>();
    chaining::joint<Begin> &joint = imp->_joint;
    auto weak_joint = to_weak(joint);
    std::size_t const next_idx = joint.handlers_size() + 1;

    joint.template push_handler<In>([handler = imp->_handler, weak_joint, next_idx](In const &value) mutable {
        auto const result = handler(value);
        if (auto joint = weak_joint.lock()) {
            joint.template handler<Out>(next_idx)(result);
        }
    });

    return chain<Out, Out, Begin, Syncable>(joint, [](Out const &value) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
chain<Out, In, Begin, Syncable> chain<Out, In, Begin, Syncable>::perform(
    std::function<void(Out const &)> perform_handler) {
    auto imp = impl_ptr<impl>();
    return chain<Out, In, Begin, Syncable>(
        std::move(imp->_joint),
        [perform_handler = std::move(perform_handler), handler = std::move(imp->_handler)](In const &value) {
            Out result = handler(value);
            perform_handler(result);
            return result;
        });
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <std::size_t N, typename T>
chain<Out, In, Begin, Syncable> chain<Out, In, Begin, Syncable>::receive(receiver<T> &receiver) {
    return impl_ptr<impl>()->template receive<N>(*this, receiver);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T, std::size_t N>
chain<Out, In, Begin, Syncable> chain<Out, In, Begin, Syncable>::receive(std::array<receiver<T>, N> receivers) {
    return impl_ptr<impl>()->template receive<T, N>(*this, receivers);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T>
chain<Out, In, Begin, Syncable> chain<Out, In, Begin, Syncable>::receive(std::vector<receiver<T>> receivers) {
    return impl_ptr<impl>()->template receive<T>(*this, receivers);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T>
chain<Out, In, Begin, Syncable> chain<Out, In, Begin, Syncable>::receive(std::initializer_list<receiver<T>> receivers) {
    std::vector<receiver<T>> vector{receivers};
    return impl_ptr<impl>()->template receive<T>(*this, vector);
}

template <typename Out, typename In, typename Begin, bool Syncable>
chain<Out, In, Begin, Syncable> chain<Out, In, Begin, Syncable>::receive_null(receiver<> &receiver) {
    return this->perform([weak_receiver = to_weak(receiver)](Out const &value) {
        if (chaining::receiver<> receiver = weak_receiver.lock()) {
            receiver.receivable().receive_value(nullptr);
        }
    });
}

template <typename Out, typename In, typename Begin, bool Syncable>
chain<Out, Out, Begin, Syncable> chain<Out, In, Begin, Syncable>::guard(
    std::function<bool(Out const &value)> guarding) {
    auto imp = impl_ptr<impl>();
    chaining::joint<Begin> &joint = imp->_joint;
    auto weak_joint = to_weak(joint);
    std::size_t const next_idx = joint.handlers_size() + 1;

    joint.template push_handler<In>(
        [handler = imp->_handler, weak_joint, next_idx, filter_handler = std::move(guarding)](In const &value) mutable {
            auto const result = handler(value);
            if (filter_handler(result)) {
                if (auto joint = weak_joint.lock()) {
                    joint.template handler<Out>(next_idx)(result);
                }
            }
        });

    return chain<Out, Out, Begin, Syncable>(joint, [](Out const &value) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename F>
auto chain<Out, In, Begin, Syncable>::to(F handler) {
    return impl_ptr<impl>()->to(std::move(handler));
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T>
auto chain<Out, In, Begin, Syncable>::to_value(T value) {
    return impl_ptr<impl>()->to([value = std::move(value)](Out const &) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto chain<Out, In, Begin, Syncable>::to_null() {
    return impl_ptr<impl>()->to([](Out const &) { return nullptr; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto chain<Out, In, Begin, Syncable>::to_tuple() {
    return impl_ptr<impl>()->template to_tuple<>(*this);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename SubIn, typename SubBegin, bool SubSyncable>
chain<Out, Out, Begin, Syncable | SubSyncable> chain<Out, In, Begin, Syncable>::merge(
    chain<Out, SubIn, SubBegin, SubSyncable> sub_chain) {
    auto imp = impl_ptr<impl>();
    chaining::joint<Begin> &joint = imp->_joint;
    auto weak_joint = to_weak(joint);
    std::size_t const next_idx = joint.handlers_size() + 1;

    auto sub_imp = sub_chain.template impl_ptr<typename chain<Out, SubIn, SubBegin, SubSyncable>::impl>();
    auto &sub_joint = sub_imp->_joint;

    sub_joint.template push_handler<SubIn>(
        [handler = sub_imp->_handler, weak_joint, next_idx](SubIn const &value) mutable {
            if (auto joint = weak_joint.lock()) {
                joint.template handler<Out>(next_idx)(handler(value));
            }
        });

    joint.template push_handler<In>([handler = imp->_handler, weak_joint, next_idx](In const &value) mutable {
        if (auto joint = weak_joint.lock()) {
            joint.template handler<Out>(next_idx)(handler(value));
        }
    });

    joint.add_sub_joint(std::move(sub_joint));

    return chain<Out, Out, Begin, Syncable | SubSyncable>(joint, [](Out const &value) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
auto chain<Out, In, Begin, Syncable>::pair(chain<SubOut, SubIn, SubBegin, SubSyncable> sub_chain) {
    using opt_pair_t = std::pair<std::optional<Out>, std::optional<SubOut>>;

    auto imp = impl_ptr<impl>();
    chaining::joint<Begin> &joint = imp->_joint;
    auto weak_joint = to_weak(joint);
    std::size_t const next_idx = joint.handlers_size() + 1;

    auto sub_imp = sub_chain.template impl_ptr<typename chain<SubOut, SubIn, SubBegin, SubSyncable>::impl>();
    auto &sub_joint = sub_imp->_joint;

    sub_joint.template push_handler<SubIn>(
        [handler = sub_imp->_handler, weak_joint, next_idx](SubIn const &value) mutable {
            if (auto joint = weak_joint.lock()) {
                joint.template handler<opt_pair_t>(next_idx)(opt_pair_t{std::nullopt, handler(value)});
            }
        });

    joint.template push_handler<In>([handler = imp->_handler, weak_joint, next_idx](In const &value) mutable {
        if (auto joint = weak_joint.lock()) {
            joint.template handler<opt_pair_t>(next_idx)(opt_pair_t(handler(value), std::nullopt));
        }
    });

    joint.add_sub_joint(std::move(sub_joint));

    return chain<opt_pair_t, opt_pair_t, Begin, Syncable | SubSyncable>(joint,
                                                                        [](opt_pair_t const &value) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
auto chain<Out, In, Begin, Syncable>::combine(chain<SubOut, SubIn, SubBegin, SubSyncable> sub_chain) {
    return impl_ptr<impl>()->combine(*this, std::move(sub_chain));
}

template <typename Out, typename In, typename Begin, bool Syncable>
observer<Begin> chain<Out, In, Begin, Syncable>::end() {
    return impl_ptr<impl>()->end();
}

template <typename Out, typename In, typename Begin, bool Syncable>
observer<Begin> chain<Out, In, Begin, Syncable>::sync() {
    return impl_ptr<impl>()->broadcast();
}
}  // namespace yas::chaining
