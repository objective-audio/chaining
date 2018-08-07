//
//  yas_chaining_private.h
//

#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>
#include "yas_fast_each.h"
#include "yas_timer.h"

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
    impl(chaining::input<Begin> &&input) : _input(std::move(input)) {
    }

    void sync() override {
        this->_input.sync();
    }

    chaining::input<Begin> _input;
};

template <typename Begin>
typed_observer<Begin>::typed_observer(chaining::input<Begin> input)
    : observer(std::make_shared<impl>(std::move(input))) {
}

template <typename Begin>
typed_observer<Begin>::typed_observer(std::nullptr_t) : observer(nullptr) {
}

template <typename Begin>
typed_observer<Begin>::~typed_observer() = default;

template <typename Begin>
chaining::input<Begin> &typed_observer<Begin>::input() {
    return impl_ptr<impl>()->_input;
}

#pragma mark - input

template <typename T>
struct input<T>::impl : input_base::impl {
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

        for (auto &sub_input : this->_sub_inputs) {
            sub_input.sync();
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

    void add_sub_input(input_base &&sub_input) {
        this->_sub_inputs.emplace_back(std::move(sub_input));
    }

   private:
    std::vector<yas::any> _handlers;
    std::vector<input_base> _sub_inputs;
};

template <typename T>
input<T>::input(weak<sender_base<T>> weak_sender) : input_base(std::make_shared<impl>(std::move(weak_sender))) {
}

template <typename T>
input<T>::input(std::nullptr_t) : input_base(nullptr) {
}

template <typename T>
input<T>::~input() {
    if (impl_ptr() && impl_ptr().unique()) {
        if (auto sender = impl_ptr<impl>()->_weak_sender.lock()) {
            sender.chainable().erase_input(this->identifier());
        }
        impl_ptr().reset();
    }
}

template <typename T>
void input<T>::input_value(T const &value) {
    impl_ptr<impl>()->send_value(value);
}

template <typename T>
template <bool Syncable>
node<T, T, T, Syncable> input<T>::begin() {
    return node<T, T, T, Syncable>(*this);
}

template <typename T>
template <typename P>
void input<T>::push_handler(std::function<void(P const &)> handler) {
    impl_ptr<impl>()->push_handler(std::move(handler));
}

template <typename T>
std::size_t input<T>::handlers_size() const {
    return impl_ptr<impl>()->handlers_size();
}

template <typename T>
template <typename P>
std::function<void(P const &)> const &input<T>::handler(std::size_t const idx) const {
    return impl_ptr<impl>()->handler(idx).template get<std::function<void(P const &)>>();
}

template <typename T>
void input<T>::add_sub_input(input_base sug_input) {
    impl_ptr<impl>()->add_sub_input(std::move(sug_input));
}

#pragma mark - sender_chainable

template <typename T>
sender_chainable<T>::sender_chainable(std::shared_ptr<impl> impl) : protocol(std::move(impl)) {
}

template <typename T>
sender_chainable<T>::sender_chainable(std::nullptr_t) : protocol(nullptr) {
}

template <typename T>
void sender_chainable<T>::erase_input(std::uintptr_t const key) {
    impl_ptr<impl>()->erase_input(key);
}

template <typename T>
void sender_chainable<T>::sync(std::uintptr_t const key) {
    impl_ptr<impl>()->sync(key);
}

#pragma mark - sender_base

template <typename T>
struct sender_base<T>::impl : base::impl, sender_chainable<T>::impl {
    std::unordered_map<std::uintptr_t, weak<input<T>>> inputs;

    void erase_input(std::uintptr_t const key) override {
        this->inputs.erase(key);
    }

    void sync(std::uintptr_t const key) override {
    }

    template <bool Syncable>
    node<T, T, T, Syncable> begin(sender_base<T> &sender) {
        chaining::input<T> input{to_weak(sender)};
        this->inputs.insert(std::make_pair(input.identifier(), to_weak(input)));
        return input.template begin<Syncable>();
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
            for (auto &pair : this->inputs) {
                if (auto input = pair.second.lock()) {
                    input.input_value(value);
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
node<T, T, T, false> notifier<T>::chain() {
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
            if (auto input = this->inputs.at(key).lock()) {
                input.input_value(*value);
            }
        }
    }

    void sync() {
        if (auto lock = std::unique_lock<std::mutex>(this->_sync_mutex, std::try_to_lock); lock.owns_lock()) {
            if (auto value = this->_sync_handler()) {
                for (auto &pair : this->inputs) {
                    if (auto input = pair.second.lock()) {
                        input.input_value(*value);
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
chaining::node<T, T, T, true> fetcher<T>::chain() {
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

                for (auto &pair : this->inputs) {
                    if (auto input = pair.second.lock()) {
                        input.input_value(this->_value);
                    }
                }
            }
        }
    }

    void sync(std::uintptr_t const key) override {
        if (auto input = this->inputs.at(key).lock()) {
            input.input_value(this->_value);
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
node<T, T, T, true> holder<T>::chain() {
    return this->template impl_ptr<impl>()->template begin<true>(*this);
}

template <typename T>
receiver<T> &holder<T>::receiver() {
    return this->template impl_ptr<impl>()->receiver();
}

#pragma mark - node

template <typename Out, typename In, typename Begin, bool Syncable>
struct node<Out, In, Begin, Syncable>::impl : base::impl {
    input<Begin> _input;
    std::function<Out(In const &)> _handler;

    impl(input<Begin> &&input, std::function<Out(In const &)> &&handler)
        : _input(std::move(input)), _handler(std::move(handler)) {
    }

    template <typename F>
    node<return_t<F>, In, Begin, Syncable> to(F &&to_handler) {
        return chaining::node<return_t<F>, In, Begin, Syncable>(
            std::move(this->_input), [to_handler = std::move(to_handler), handler = std::move(this->_handler)](
                                         In const &value) mutable { return to_handler(handler(value)); });
    }

    template <typename T = Out, enable_if_tuple_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(node<Out, In, Begin, Syncable> &node) {
        return node;
    }

    template <typename T = Out, enable_if_pair_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(node<Out, In, Begin, Syncable> &node) {
        return this->to([](Out const &pair) { return std::make_tuple(pair.first, pair.second); });
    }

    template <typename T = Out, disable_if_tuple_t<T, std::nullptr_t> = nullptr,
              disable_if_pair_t<T, std::nullptr_t> = nullptr>
    auto to_tuple(node<Out, In, Begin, Syncable> &node) {
        return this->to([](Out const &value) { return std::make_tuple(value); });
    }

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable,
              disable_if_tuple_t<SubOut, std::nullptr_t> = nullptr, typename MainOut = Out,
              disable_if_tuple_t<MainOut, std::nullptr_t> = nullptr>
    auto tuple(node<SubOut, SubIn, SubBegin, SubSyncable> &&sub_node) {
        using opt_tuple_t = std::tuple<opt_t<Out>, opt_t<SubOut>>;

        chaining::input<Begin> &input = this->_input;
        auto weak_input = to_weak(input);
        std::size_t const next_idx = input.handlers_size() + 1;

        auto sub_imp = sub_node.template impl_ptr<typename node<SubOut, SubIn, SubBegin, SubSyncable>::impl>();
        auto &sub_input = sub_imp->_input;

        sub_input.template push_handler<SubIn>(
            [handler = sub_imp->_handler, weak_input, next_idx](SubIn const &value) mutable {
                if (auto input = weak_input.lock()) {
                    input.template handler<opt_tuple_t>(next_idx)(opt_tuple_t(nullopt, handler(value)));
                }
            });

        input.template push_handler<In>([handler = this->_handler, weak_input, next_idx](In const &value) mutable {
            if (auto input = weak_input.lock()) {
                input.template handler<opt_tuple_t>(next_idx)(opt_tuple_t(handler(value), nullopt));
            }
        });

        input.add_sub_input(std::move(sub_input));

        return node<opt_tuple_t, opt_tuple_t, Begin, Syncable | SubSyncable>(
            input, [](opt_tuple_t const &value) { return value; });
    }

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
    auto combine_pair(chaining::node<Out, In, Begin, Syncable> &node,
                      chaining::node<SubOut, SubIn, SubBegin, SubSyncable> &&sub_node) {
        using opt_pair_t = std::pair<opt_t<Out>, opt_t<SubOut>>;

        return node.pair(std::move(sub_node))
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
    auto combine(chaining::node<Out, In, Begin, Syncable> &node,
                 chaining::node<SubOut, SubIn, SubBegin, SubSyncable> &&sub_node) {
        return this->combine_pair(node, std::move(sub_node));
    }

    template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable,
              enable_if_tuple_t<SubOut, std::nullptr_t> = nullptr, typename MainOut = Out,
              enable_if_tuple_t<MainOut, std::nullptr_t> = nullptr>
    auto combine(chaining::node<Out, In, Begin, Syncable> &node,
                 chaining::node<SubOut, SubIn, SubBegin, SubSyncable> &&sub_node) {
        return this->combine_pair(node, std::move(sub_node)).to([](std::pair<Out, SubOut> const &pair) {
            return std::tuple_cat(static_cast<typename std::remove_const<Out>::type>(pair.first),
                                  static_cast<typename std::remove_const<SubOut>::type>(pair.second));
        });
    }

    template <std::size_t N, typename T, typename TupleOut = Out, enable_if_tuple_t<TupleOut, std::nullptr_t> = nullptr>
    auto receive(chaining::node<Out, In, Begin, Syncable> &node, receiver<T> &receiver) {
        return node.perform([output = receiver.chainable().make_output()](Out const &value) mutable {
            output.output_value(std::get<N>(value));
        });
    }

    template <std::size_t N, typename T, typename ArrayOut = Out, enable_if_array_t<ArrayOut, std::nullptr_t> = nullptr>
    auto receive(chaining::node<Out, In, Begin, Syncable> &node, receiver<T> &receiver) {
        return node.perform([output = receiver.chainable().make_output()](Out const &value) mutable {
            output.output_value(std::get<N>(value));
        });
    }

    template <std::size_t N, typename T, typename NonOut = Out, disable_if_tuple_t<NonOut, std::nullptr_t> = nullptr,
              disable_if_array_t<NonOut, std::nullptr_t> = nullptr>
    auto receive(chaining::node<Out, In, Begin, Syncable> &node, receiver<T> &receiver) {
        return node.perform(
            [output = receiver.chainable().make_output()](Out const &value) mutable { output.output_value(value); });
    }

    template <typename T, std::size_t N>
    auto receive(chaining::node<Out, In, Begin, Syncable> &node, std::array<receiver<T>, N> &receivers) {
        std::vector<chaining::output<T>> outputs;
        outputs.reserve(N);

        auto each = make_fast_each(N);
        while (yas_each_next(each)) {
            auto const &idx = yas_each_index(each);
            outputs.emplace_back(receivers.at(idx).chainable().make_output());
        }

        return node.perform([outputs = std::move(outputs)](Out const &values) mutable {
            auto each = make_fast_each(N);
            while (yas_each_next(each)) {
                auto const &idx = yas_each_index(each);
                outputs.at(idx).output_value(values.at(idx));
            }
        });
    }

    template <typename T>
    auto receive(chaining::node<Out, In, Begin, Syncable> &node, std::vector<receiver<T>> &receivers) {
        std::size_t const count = receivers.size();

        std::vector<chaining::output<T>> outputs;
        outputs.reserve(count);

        auto each = make_fast_each(count);
        while (yas_each_next(each)) {
            auto const &idx = yas_each_index(each);
            outputs.emplace_back(receivers.at(idx).chainable().make_output());
        }

        return node.perform([outputs = std::move(outputs)](Out const &values) mutable {
            std::size_t const count = std::min(values.size(), outputs.size());
            auto each = make_fast_each(count);
            while (yas_each_next(each)) {
                auto const &idx = yas_each_index(each);
                outputs.at(idx).output_value(values.at(idx));
            }
        });
    }

    chaining::typed_observer<Begin> _end() {
        this->_input.template push_handler<In>([handler = this->_handler](In const &value) { handler(value); });
        return typed_observer<Begin>(this->_input);
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
node<Out, In, Begin, Syncable>::node(input<Begin> input)
    : node(std::move(input), [](Begin const &value) { return value; }) {
}

template <typename Out, typename In, typename Begin, bool Syncable>
node<Out, In, Begin, Syncable>::node(input<Begin> input, std::function<Out(In const &)> handler)
    : base(std::make_shared<impl>(std::move(input), std::move(handler))) {
}

template <typename Out, typename In, typename Begin, bool Syncable>
node<Out, In, Begin, Syncable>::node(std::nullptr_t) : base(nullptr) {
}

template <typename Out, typename In, typename Begin, bool Syncable>
node<Out, In, Begin, Syncable>::~node() = default;

template <typename Out, typename In, typename Begin, bool Syncable>
auto node<Out, In, Begin, Syncable>::normalize() {
    auto imp = impl_ptr<impl>();
    chaining::input<Begin> &input = imp->_input;
    auto weak_input = to_weak(input);
    std::size_t const next_idx = input.handlers_size() + 1;

    input.template push_handler<In>([handler = imp->_handler, weak_input, next_idx](In const &value) mutable {
        auto const result = handler(value);
        if (auto input = weak_input.lock()) {
            input.template handler<Out>(next_idx)(result);
        }
    });

    return node<Out, Out, Begin, Syncable>(input, [](Out const &value) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto node<Out, In, Begin, Syncable>::perform(std::function<void(Out const &)> perform_handler) {
    auto imp = impl_ptr<impl>();
    return node<Out, In, Begin, Syncable>(
        std::move(imp->_input),
        [perform_handler = std::move(perform_handler), handler = std::move(imp->_handler)](In const &value) {
            Out result = handler(value);
            perform_handler(result);
            return result;
        });
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <std::size_t N, typename T>
auto node<Out, In, Begin, Syncable>::receive(receiver<T> &receiver) {
    return impl_ptr<impl>()->template receive<N>(*this, receiver);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T, std::size_t N>
auto node<Out, In, Begin, Syncable>::receive(std::array<receiver<T>, N> receivers) {
    return impl_ptr<impl>()->template receive<T, N>(*this, receivers);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T>
auto node<Out, In, Begin, Syncable>::receive(std::vector<receiver<T>> receivers) {
    return impl_ptr<impl>()->template receive<T>(*this, receivers);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T>
[[nodiscard]] auto node<Out, In, Begin, Syncable>::receive(std::initializer_list<receiver<T>> receivers) {
    std::vector<receiver<T>> vector{receivers};
    return impl_ptr<impl>()->template receive<T>(*this, vector);
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto node<Out, In, Begin, Syncable>::receive_null(receiver<std::nullptr_t> &receiver) {
    return this->perform(
        [output = receiver.chainable().make_output()](Out const &value) mutable { output.output_value(nullptr); });
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto node<Out, In, Begin, Syncable>::guard(std::function<bool(Out const &value)> guarding) {
    auto imp = impl_ptr<impl>();
    chaining::input<Begin> &input = imp->_input;
    auto weak_input = to_weak(input);
    std::size_t const next_idx = input.handlers_size() + 1;

    input.template push_handler<In>(
        [handler = imp->_handler, weak_input, next_idx, filter_handler = std::move(guarding)](In const &value) mutable {
            auto const result = handler(value);
            if (filter_handler(result)) {
                if (auto input = weak_input.lock()) {
                    input.template handler<Out>(next_idx)(result);
                }
            }
        });

    return node<Out, Out, Begin, Syncable>(input, [](Out const &value) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename F>
auto node<Out, In, Begin, Syncable>::to(F handler) {
    return impl_ptr<impl>()->to(std::move(handler));
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename T>
auto node<Out, In, Begin, Syncable>::to_value(T value) {
    return impl_ptr<impl>()->to([value = std::move(value)](Out const &) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto node<Out, In, Begin, Syncable>::to_null() {
    return impl_ptr<impl>()->to([](Out const &) { return nullptr; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
auto node<Out, In, Begin, Syncable>::to_tuple() {
    return impl_ptr<impl>()->template to_tuple<>(*this);
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename SubIn, typename SubBegin, bool SubSyncable>
auto node<Out, In, Begin, Syncable>::merge(node<Out, SubIn, SubBegin, SubSyncable> sub_node) {
    auto imp = impl_ptr<impl>();
    chaining::input<Begin> &input = imp->_input;
    auto weak_input = to_weak(input);
    std::size_t const next_idx = input.handlers_size() + 1;

    auto sub_imp = sub_node.template impl_ptr<typename node<Out, SubIn, SubBegin, SubSyncable>::impl>();
    auto &sub_input = sub_imp->_input;

    sub_input.template push_handler<SubIn>(
        [handler = sub_imp->_handler, weak_input, next_idx](SubIn const &value) mutable {
            if (auto input = weak_input.lock()) {
                input.template handler<Out>(next_idx)(handler(value));
            }
        });

    input.template push_handler<In>([handler = imp->_handler, weak_input, next_idx](In const &value) mutable {
        if (auto input = weak_input.lock()) {
            input.template handler<Out>(next_idx)(handler(value));
        }
    });

    input.add_sub_input(std::move(sub_input));

    return node<Out, Out, Begin, Syncable | SubSyncable>(input, [](Out const &value) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
auto node<Out, In, Begin, Syncable>::pair(node<SubOut, SubIn, SubBegin, SubSyncable> sub_node) {
    using opt_pair_t = std::pair<opt_t<Out>, opt_t<SubOut>>;

    auto imp = impl_ptr<impl>();
    chaining::input<Begin> &input = imp->_input;
    auto weak_input = to_weak(input);
    std::size_t const next_idx = input.handlers_size() + 1;

    auto sub_imp = sub_node.template impl_ptr<typename node<SubOut, SubIn, SubBegin, SubSyncable>::impl>();
    auto &sub_input = sub_imp->_input;

    sub_input.template push_handler<SubIn>(
        [handler = sub_imp->_handler, weak_input, next_idx](SubIn const &value) mutable {
            if (auto input = weak_input.lock()) {
                input.template handler<opt_pair_t>(next_idx)(opt_pair_t{nullopt, handler(value)});
            }
        });

    input.template push_handler<In>([handler = imp->_handler, weak_input, next_idx](In const &value) mutable {
        if (auto input = weak_input.lock()) {
            input.template handler<opt_pair_t>(next_idx)(opt_pair_t(handler(value), nullopt));
        }
    });

    input.add_sub_input(std::move(sub_input));

    return node<opt_pair_t, opt_pair_t, Begin, Syncable | SubSyncable>(input,
                                                                       [](opt_pair_t const &value) { return value; });
}

template <typename Out, typename In, typename Begin, bool Syncable>
template <typename SubOut, typename SubIn, typename SubBegin, bool SubSyncable>
auto node<Out, In, Begin, Syncable>::combine(node<SubOut, SubIn, SubBegin, SubSyncable> sub_node) {
    return impl_ptr<impl>()->combine(*this, std::move(sub_node));
}

template <typename Out, typename In, typename Begin, bool Syncable>
typed_observer<Begin> node<Out, In, Begin, Syncable>::end() {
    return impl_ptr<impl>()->end();
}

template <typename Out, typename In, typename Begin, bool Syncable>
typed_observer<Begin> node<Out, In, Begin, Syncable>::sync() {
    return impl_ptr<impl>()->sync();
}
}  // namespace yas::chaining
