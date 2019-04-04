//
//  yas_chaining_joint_private.h
//

#pragma once

#include <vector>

namespace yas::chaining {
template <typename P>
std::function<void(P const &)> const &any_joint::handler(std::size_t const idx) const {
    return *std::any_cast<std::function<void(P const &)>>(&impl_ptr<impl>()->handler(idx));
}

template <typename P>
any_joint::handler_f<P> const &any_joint::handler2(std::size_t const idx) const {
    return *std::any_cast<handler_f<P>>(&impl_ptr<impl>()->handler(idx));
}

template <typename T>
struct joint<T>::impl : any_joint::impl {
    weak<sender<T>> _weak_sender;

    impl(weak<sender<T>> &&weak_sender) : _weak_sender(std::move(weak_sender)) {
    }

    void call_first(T const &value) {
        if (this->_handlers.size() > 0) {
            any_joint joint = cast<any_joint>();
            joint.handler2<T>(0)(value, joint);
        } else {
            std::runtime_error("handler not found. must call the end.");
        }
    }

    void invalidate() override {
        for (any_joint &sub_joint : this->_sub_joints) {
            sub_joint.invalidate();
        }

        if (sender<T> sender = this->_weak_sender.lock()) {
            sender.sendable().erase_joint(this->identifier());
        }

        this->_weak_sender = nullptr;
        this->_handlers.clear();
        this->_sub_joints.clear();
    }

    void fetch() override {
        if (auto sender = this->_weak_sender.lock()) {
            sender.sendable().fetch_for(cast<joint<T>>());
        }

        for (auto &sub_joint : this->_sub_joints) {
            sub_joint.fetch();
        }
    }

    void push_handler(std::any &&handler) {
        this->_handlers.emplace_back(std::move(handler));
    }

    std::any const &handler(std::size_t const idx) override {
        return this->_handlers.at(idx);
    }

    std::size_t handlers_size() {
        return this->_handlers.size();
    }

    void add_sub_joint(any_joint &&sub_joint) {
        this->_sub_joints.emplace_back(std::move(sub_joint));
    }

   private:
    std::vector<std::any> _handlers;
    std::vector<any_joint> _sub_joints;
};

template <typename T>
joint<T>::joint(weak<sender<T>> weak_sender) : any_joint(std::make_shared<impl>(std::move(weak_sender))) {
}

template <typename T>
joint<T>::joint(std::nullptr_t) : any_joint(nullptr) {
}

template <typename T>
joint<T>::~joint() {
    if (impl_ptr() && impl_ptr().unique()) {
        if (auto sender = impl_ptr<impl>()->_weak_sender.lock()) {
            sender.sendable().erase_joint(this->identifier());
        }
        impl_ptr().reset();
    }
}

template <typename T>
void joint<T>::call_first(T const &value) {
    impl_ptr<impl>()->call_first(value);
}

template <typename T>
void joint<T>::invalidate() {
    impl_ptr<impl>()->invalidate();
}

template <typename T>
template <typename P>
void joint<T>::push_handler(any_joint::handler_f<P> handler) {
    impl_ptr<impl>()->push_handler(std::move(handler));
}

template <typename T>
std::size_t joint<T>::handlers_size() const {
    return impl_ptr<impl>()->handlers_size();
}

template <typename T>
void joint<T>::add_sub_joint(any_joint sub_joint) {
    impl_ptr<impl>()->add_sub_joint(std::move(sub_joint));
}
}  // namespace yas::chaining
