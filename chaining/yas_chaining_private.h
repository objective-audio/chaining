//
//  yas_chaining_private.h
//

#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>
#include "yas_fast_each.h"

namespace yas::chaining {

#pragma mark - chaining::output

template <typename T>
struct output<T>::impl : base::impl {
    weak<receiver<T>> _weak_receiver;

    impl(weak<receiver<T>> &&weak_receiver) : _weak_receiver(std::move(weak_receiver)) {
    }

    void output_value(T const &value) {
        if (auto receiver = this->_weak_receiver.lock()) {
            receiver.chainable().receive_value(value);
        }
    }
};

template <typename T>
output<T>::output(weak<receiver<T>> weak_receiver) : base(std::make_shared<impl>(std::move(weak_receiver))) {
}

template <typename T>
output<T>::output(std::nullptr_t) : base(nullptr) {
}

template <typename T>
void output<T>::output_value(T const &value) {
    impl_ptr<impl>()->output_value(value);
}

#pragma mark - chaining::receiver_chainable

template <typename T>
receiver_chainable<T>::receiver_chainable(std::shared_ptr<impl> impl) : protocol(std::move(impl)) {
}

template <typename T>
receiver_chainable<T>::receiver_chainable(std::nullptr_t) : protocol(nullptr) {
}

template <typename T>
void receiver_chainable<T>::receive_value(T const &value) {
    impl_ptr<impl>()->receive_value(value);
}

template <typename T>
output<T> receiver_chainable<T>::make_output() {
    return impl_ptr<impl>()->make_output();
}

#pragma mark - chaining::receiver

template <typename T>
struct chaining::receiver<T>::impl : base::impl, chaining::receiver_chainable<T>::impl {
    std::function<void(T const &)> handler;

    impl(std::function<void(T const &)> &&handler) : handler(std::move(handler)) {
    }

    void receive_value(T const &value) override {
        this->handler(value);
    }

    output<T> make_output() override {
        return output<T>{to_weak(cast<chaining::receiver<T>>())};
    }
};

template <typename T>
chaining::receiver<T>::receiver(std::function<void(T const &)> handler)
    : base(std::make_shared<impl>(std::move(handler))) {
}

template <typename T>
chaining::receiver<T>::receiver(std::function<void(void)> handler)
    : receiver([handler = std::move(handler)](auto const &) { handler(); }) {
}

template <typename T>
chaining::receiver<T>::receiver(std::nullptr_t) : base(nullptr) {
}

template <typename T>
chaining::receiver<T>::~receiver() = default;

template <typename T>
chaining::receiver_chainable<T> chaining::receiver<T>::chainable() {
    if (!this->_chainable) {
        this->_chainable = chaining::receiver_chainable<T>{impl_ptr<typename chaining::receiver_chainable<T>::impl>()};
    }
    return this->_chainable;
}

#pragma mark - chaining::typed_observer

template <typename Begin>
struct typed_observer<Begin>::impl : observer::impl {
    impl(chaining::joint<Begin> &&joint) : _joint(std::move(joint)) {
    }

    void sync() override {
        this->_joint.sync();
    }

    chaining::joint<Begin> _joint;
};

template <typename Begin>
typed_observer<Begin>::typed_observer(chaining::joint<Begin> joint)
    : observer(std::make_shared<impl>(std::move(joint))) {
}

template <typename Begin>
typed_observer<Begin>::typed_observer(std::nullptr_t) : observer(nullptr) {
}

template <typename Begin>
typed_observer<Begin>::~typed_observer() = default;

template <typename Begin>
chaining::joint<Begin> &typed_observer<Begin>::joint() {
    return impl_ptr<impl>()->_joint;
}

#pragma mark - joint

template <typename T>
struct joint<T>::impl : joint_base::impl {
    weak<sender_base<T>> _weak_sender;

    impl(weak<sender_base<T>> &&weak_sender) : _weak_sender(std::move(weak_sender)) {
    }

    void send_value(T const &value) {
        if (this->_handlers.size() > 0) {
            this->_handlers.front().template get<std::function<void(T const &)>>()(value);
        } else {
            std::runtime_error("handler not found. must call the end.");
        }
    }

    void sync() override {
        if (auto sender = this->_weak_sender.lock()) {
            sender.chainable().sync(this->identifier());
        }

        for (auto &sub_joint : this->_sub_joints) {
            sub_joint.sync();
        }
    }

    void push_handler(yas::any &&handler) {
        this->_handlers.emplace_back(std::move(handler));
    }

    yas::any handler(std::size_t const idx) {
        return this->_handlers.at(idx);
    }

    std::size_t handlers_size() {
        return this->_handlers.size();
    }

    void add_sub_joint(joint_base &&sub_joint) {
        this->_sub_joints.emplace_back(std::move(sub_joint));
    }

   private:
    std::vector<yas::any> _handlers;
    std::vector<joint_base> _sub_joints;
};

template <typename T>
joint<T>::joint(weak<sender_base<T>> weak_sender) : joint_base(std::make_shared<impl>(std::move(weak_sender))) {
}

template <typename T>
joint<T>::joint(std::nullptr_t) : joint_base(nullptr) {
}

template <typename T>
joint<T>::~joint() {
    if (impl_ptr() && impl_ptr().unique()) {
        if (auto sender = impl_ptr<impl>()->_weak_sender.lock()) {
            sender.chainable().erase_joint(this->identifier());
        }
        impl_ptr().reset();
    }
}

template <typename T>
void joint<T>::input_value(T const &value) {
    impl_ptr<impl>()->send_value(value);
}

template <typename T>
template <bool Syncable>
chain<T, T, T, Syncable> joint<T>::begin() {
    return chain<T, T, T, Syncable>(*this);
}

template <typename T>
template <typename P>
void joint<T>::push_handler(std::function<void(P const &)> handler) {
    impl_ptr<impl>()->push_handler(std::move(handler));
}

template <typename T>
std::size_t joint<T>::handlers_size() const {
    return impl_ptr<impl>()->handlers_size();
}

template <typename T>
template <typename P>
std::function<void(P const &)> const &joint<T>::handler(std::size_t const idx) const {
    return impl_ptr<impl>()->handler(idx).template get<std::function<void(P const &)>>();
}

template <typename T>
void joint<T>::add_sub_joint(joint_base sub_joint) {
    impl_ptr<impl>()->add_sub_joint(std::move(sub_joint));
}

#pragma mark - sender_chainable

template <typename T>
sender_chainable<T>::sender_chainable(std::shared_ptr<impl> impl) : protocol(std::move(impl)) {
}

template <typename T>
sender_chainable<T>::sender_chainable(std::nullptr_t) : protocol(nullptr) {
}

template <typename T>
void sender_chainable<T>::erase_joint(std::uintptr_t const key) {
    impl_ptr<impl>()->erase_joint(key);
}

template <typename T>
void sender_chainable<T>::sync(std::uintptr_t const key) {
    impl_ptr<impl>()->sync(key);
}

#pragma mark - sender_base

template <typename T>
struct sender_base<T>::impl : base::impl, sender_chainable<T>::impl {
    std::unordered_map<std::uintptr_t, weak<joint<T>>> joints;

    void erase_joint(std::uintptr_t const key) override {
        this->joints.erase(key);
    }

    void sync(std::uintptr_t const key) override {
    }

    template <bool Syncable>
    chain<T, T, T, Syncable> begin(sender_base<T> &sender) {
        chaining::joint<T> joint{to_weak(sender)};
        this->joints.insert(std::make_pair(joint.identifier(), to_weak(joint)));
        return joint.template begin<Syncable>();
    }
};

template <typename T>
sender_base<T>::sender_base(std::shared_ptr<impl> &&impl) : base(std::move(impl)) {
}

template <typename T>
sender_base<T>::sender_base(std::nullptr_t) : base(nullptr) {
}

template <typename T>
sender_chainable<T> sender_base<T>::chainable() {
    if (!this->_chainable) {
        this->_chainable = sender_chainable<T>{impl_ptr<typename sender_chainable<T>::impl>()};
    }
    return this->_chainable;
}

#pragma mark - notifier

template <typename T>
struct notifier<T>::impl : sender_base<T>::impl {
    void send_value(T const &value) {
        if (auto lock = std::unique_lock<std::mutex>(this->_send_mutex, std::try_to_lock); lock.owns_lock()) {
            for (auto &pair : this->joints) {
                if (auto joint = pair.second.lock()) {
                    joint.input_value(value);
                }
            }
        }
    }

    chaining::receiver<T> &receiver() {
        if (!this->_receiver) {
            this->_receiver = chaining::receiver<T>{
                [weak_notifier = to_weak(this->template cast<chaining::notifier<T>>())](T const &value) {
                    if (auto notifier = weak_notifier.lock()) {
                        notifier.notify(value);
                    }
                }};
        }
        return this->_receiver;
    }

   private:
    std::mutex _send_mutex;
    chaining::receiver<T> _receiver{nullptr};
};

template <typename T>
notifier<T>::notifier() : sender_base<T>(std::make_shared<impl>()) {
}

template <typename T>
notifier<T>::notifier(std::shared_ptr<impl> &&impl) : sender_base<T>(std::move(impl)) {
}

template <typename T>
notifier<T>::notifier(std::nullptr_t) : sender_base<T>(nullptr) {
}

template <typename T>
void notifier<T>::notify(T const &value) {
    this->template impl_ptr<impl>()->send_value(value);
}

template <typename T>
chain<T, T, T, false> notifier<T>::chain() {
    return this->template impl_ptr<impl>()->template begin<false>(*this);
}

template <typename T>
receiver<T> &notifier<T>::receiver() {
    return this->template impl_ptr<impl>()->receiver();
}

#pragma mark - fetcher

template <typename T>
struct fetcher<T>::impl : sender_base<T>::impl {
    std::function<opt_t<T>(void)> _sync_handler;

    impl(std::function<opt_t<T>(void)> &&handler) : _sync_handler(std::move(handler)) {
    }

    void sync(std::uintptr_t const key) override {
        if (auto value = this->_sync_handler()) {
            if (auto joint = this->joints.at(key).lock()) {
                joint.input_value(*value);
            }
        }
    }

    void sync() {
        if (auto lock = std::unique_lock<std::mutex>(this->_sync_mutex, std::try_to_lock); lock.owns_lock()) {
            if (auto value = this->_sync_handler()) {
                for (auto &pair : this->joints) {
                    if (auto joint = pair.second.lock()) {
                        joint.input_value(*value);
                    }
                }
            }
        }
    }

    chaining::receiver<> &receiver() {
        if (!this->_receiver) {
            this->_receiver =
                chaining::receiver<>{[weak_fetcher = to_weak(this->template cast<chaining::fetcher<T>>())] {
                    if (auto fetcher = weak_fetcher.lock()) {
                        fetcher.fetch();
                    }
                }};
        }
        return this->_receiver;
    }

   private:
    std::mutex _sync_mutex;
    chaining::receiver<> _receiver{nullptr};
};

template <typename T>
fetcher<T>::fetcher(std::function<opt_t<T>(void)> handler)
    : sender_base<T>(std::make_shared<impl>(std::move(handler))) {
}

template <typename T>
fetcher<T>::fetcher(std::nullptr_t) : sender_base<T>(nullptr) {
}

template <typename T>
void fetcher<T>::fetch() const {
    this->template impl_ptr<impl>()->sync();
}

template <typename T>
chaining::chain<T, T, T, true> fetcher<T>::chain() {
    return this->template impl_ptr<impl>()->template begin<true>(*this);
}

template <typename T>
chaining::receiver<> &fetcher<T>::receiver() {
    return this->template impl_ptr<impl>()->receiver();
}

#pragma mark - property

template <typename T>
struct holder<T>::impl : sender_base<T>::impl {
    impl(T &&value) : _value(std::move(value)) {
    }

    T &value() {
        return this->_value;
    }

    void set_value(T &&value) {
        if (auto lock = std::unique_lock<std::mutex>(this->_set_mutex, std::try_to_lock); lock.owns_lock()) {
            if (this->_value != value) {
                this->_value = std::move(value);

                for (auto &pair : this->joints) {
                    if (auto joint = pair.second.lock()) {
                        joint.input_value(this->_value);
                    }
                }
            }
        }
    }

    void sync(std::uintptr_t const key) override {
        if (auto joint = this->joints.at(key).lock()) {
            joint.input_value(this->_value);
        }
    }

    chaining::receiver<T> &receiver() {
        if (!this->_receiver) {
            this->_receiver = chaining::receiver<T>{
                [weak_property = to_weak(this->template cast<chaining::holder<T>>())](T const &value) {
                    if (auto property = weak_property.lock()) {
                        property.set_value(value);
                    }
                }};
        }
        return this->_receiver;
    }

    virtual bool is_equal(std::shared_ptr<base::impl> const &rhs) const override {
        if (auto rhs_impl = std::dynamic_pointer_cast<typename holder<T>::impl>(rhs)) {
            return this->_value == rhs_impl->_value;
        } else {
            return false;
        }
    }

   private:
    T _value;
    std::mutex _set_mutex;
    chaining::receiver<T> _receiver{nullptr};
};

template <typename T>
holder<T>::holder(T value) : sender_base<T>(std::make_shared<impl>(std::move(value))) {
}

template <typename T>
holder<T>::holder(std::nullptr_t) : sender_base<T>(nullptr) {
}

template <typename T>
holder<T>::~holder() = default;

template <typename T>
T const &holder<T>::value() const {
    return this->template impl_ptr<impl>()->value();
}

template <typename T>
T &holder<T>::value() {
    return this->template impl_ptr<impl>()->value();
}

template <typename T>
void holder<T>::set_value(T value) {
    this->template impl_ptr<impl>()->set_value(std::move(value));
}

template <typename T>
chain<T, T, T, true> holder<T>::chain() {
    return this->template impl_ptr<impl>()->template begin<true>(*this);
}

template <typename T>
receiver<T> &holder<T>::receiver() {
    return this->template impl_ptr<impl>()->receiver();
}

#pragma mark - chain

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
        using opt_tuple_t = std::tuple<opt_t<Out>, opt_t<SubOut>>;

        chaining::joint<Begin> &joint = this->_joint;
        auto weak_joint = to_weak(joint);
        std::size_t const next_idx = joint.handlers_size() + 1;

        auto sub_imp = sub_chain.template impl_ptr<typename chain<SubOut, SubIn, SubBegin, SubSyncable>::impl>();
        auto &sub_joint = sub_imp->_joint;

        sub_joint.template push_handler<SubIn>(
            [handler = sub_imp->_handler, weak_joint, next_idx](SubIn const &value) mutable {
                if (auto joint = weak_joint.lock()) {
                    joint.template handler<opt_tuple_t>(next_idx)(opt_tuple_t(nullopt, handler(value)));
                }
            });

        joint.template push_handler<In>([handler = this->_handler, weak_joint, next_idx](In const &value) mutable {
            if (auto joint = weak_joint.lock()) {
                joint.template handler<opt_tuple_t>(next_idx)(opt_tuple_t(handler(value), nullopt));
            }
        });

        joint.add_sub_joint(std::move(sub_joint));

        return chain<opt_tuple_t, opt_tuple_t, Begin, Syncable | SubSyncable>(
            joint, [](opt_tuple_t const &value) { return value; });
    }

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    auto combine_pair(chaining::chain<Out, In, Begin, Syncable> &chain,
                      chaining::chain<SubOut, SubIn, SubBegin, SubSyncable> &&sub_chain) {
        using opt_pair_t = std::pair<opt_t<Out>, opt_t<SubOut>>;

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
        return chain.perform([output = receiver.chainable().make_output()](Out const &value) mutable {
            output.output_value(std::get<N>(value));
        });
    }

    template <std::size_t N, typename T, typename ArrayOut = Out, enable_if_array_t<ArrayOut, std::nullptr_t> = nullptr>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, receiver<T> &receiver) {
        return chain.perform([output = receiver.chainable().make_output()](Out const &value) mutable {
            output.output_value(std::get<N>(value));
        });
    }

    template <std::size_t N, typename T, typename NonOut = Out, disable_if_tuple_t<NonOut, std::nullptr_t> = nullptr,
              disable_if_array_t<NonOut, std::nullptr_t> = nullptr>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, receiver<T> &receiver) {
        return chain.perform(
            [output = receiver.chainable().make_output()](Out const &value) mutable { output.output_value(value); });
    }

    template <typename T, std::size_t N>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, std::array<receiver<T>, N> &receivers) {
        std::vector<chaining::output<T>> outputs;
        outputs.reserve(N);

        auto each = make_fast_each(N);
        while (yas_each_next(each)) {
            auto const &idx = yas_each_index(each);
            outputs.emplace_back(receivers.at(idx).chainable().make_output());
        }

        return chain.perform([outputs = std::move(outputs)](Out const &values) mutable {
            auto each = make_fast_each(N);
            while (yas_each_next(each)) {
                auto const &idx = yas_each_index(each);
                outputs.at(idx).output_value(values.at(idx));
            }
        });
    }

    template <typename T>
    auto receive(chaining::chain<Out, In, Begin, Syncable> &chain, std::vector<receiver<T>> &receivers) {
        std::size_t const count = receivers.size();

        std::vector<chaining::output<T>> outputs;
        outputs.reserve(count);

        auto each = make_fast_each(count);
        while (yas_each_next(each)) {
            auto const &idx = yas_each_index(each);
            outputs.emplace_back(receivers.at(idx).chainable().make_output());
        }

        return chain.perform([outputs = std::move(outputs)](Out const &values) mutable {
            std::size_t const count = std::min(values.size(), outputs.size());
            auto each = make_fast_each(count);
            while (yas_each_next(each)) {
                auto const &idx = yas_each_index(each);
                outputs.at(idx).output_value(values.at(idx));
            }
        });
    }

    chaining::typed_observer<Begin> _end() {
        this->_joint.template push_handler<In>([handler = this->_handler](In const &value) { handler(value); });
        return typed_observer<Begin>(this->_joint);
    }

    chaining::typed_observer<Begin> end() {
        return this->_end();
    }

    chaining::typed_observer<Begin> sync() {
        static_assert(Syncable, "Syncable must be true.");

        auto observer = this->_end();
        observer.sync();
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
auto chain<Out, In, Begin, Syncable>::perform(std::function<void(Out const &)> perform_handler) {
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
auto chain<Out, In, Begin, Syncable>::receive(receiver<T> &receiver) {
    return impl_ptr<impl>()->template receive<N>(*this, receiver);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T, std::size_t N>
auto chain<Out, In, Begin, Syncable>::receive(std::array<receiver<T>, N> receivers) {
    return impl_ptr<impl>()->template receive<T, N>(*this, receivers);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T>
auto chain<Out, In, Begin, Syncable>::receive(std::vector<receiver<T>> receivers) {
    return impl_ptr<impl>()->template receive<T>(*this, receivers);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T>
[[nodiscard]] auto chain<Out, In, Begin, Syncable>::receive(std::initializer_list<receiver<T>> receivers) {
    std::vector<receiver<T>> vector{receivers};
    return impl_ptr<impl>()->template receive<T>(*this, vector);
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto chain<Out, In, Begin, Syncable>::receive_null(receiver<std::nullptr_t> &receiver) {
    return this->perform(
        [output = receiver.chainable().make_output()](Out const &value) mutable { output.output_value(nullptr); });
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto chain<Out, In, Begin, Syncable>::guard(std::function<bool(Out const &value)> guarding) {
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
auto chain<Out, In, Begin, Syncable>::merge(chain<Out, SubIn, SubBegin, SubSyncable> sub_chain) {
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
    using opt_pair_t = std::pair<opt_t<Out>, opt_t<SubOut>>;

    auto imp = impl_ptr<impl>();
    chaining::joint<Begin> &joint = imp->_joint;
    auto weak_joint = to_weak(joint);
    std::size_t const next_idx = joint.handlers_size() + 1;

    auto sub_imp = sub_chain.template impl_ptr<typename chain<SubOut, SubIn, SubBegin, SubSyncable>::impl>();
    auto &sub_joint = sub_imp->_joint;

    sub_joint.template push_handler<SubIn>(
        [handler = sub_imp->_handler, weak_joint, next_idx](SubIn const &value) mutable {
            if (auto joint = weak_joint.lock()) {
                joint.template handler<opt_pair_t>(next_idx)(opt_pair_t{nullopt, handler(value)});
            }
        });

    joint.template push_handler<In>([handler = imp->_handler, weak_joint, next_idx](In const &value) mutable {
        if (auto joint = weak_joint.lock()) {
            joint.template handler<opt_pair_t>(next_idx)(opt_pair_t(handler(value), nullopt));
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
typed_observer<Begin> chain<Out, In, Begin, Syncable>::end() {
    return impl_ptr<impl>()->end();
}

template <typename Out, typename In, typename Begin, bool Syncable>
typed_observer<Begin> chain<Out, In, Begin, Syncable>::sync() {
    return impl_ptr<impl>()->sync();
}
}  // namespace yas::chaining
