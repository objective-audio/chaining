//
//  yas_chaining_joint_private.h
//

#pragma once

#include <vector>

namespace yas::chaining {
template <typename P>
any_joint::handler_f<P> const &any_joint::handler(std::size_t const idx) const {
    return *std::any_cast<handler_f<P>>(&this->any_handler(idx));
}

template <typename T>
joint<T>::joint(std::weak_ptr<sender<T> const> &&weak_sender) : any_joint(), _weak_sender(std::move(weak_sender)) {
}

template <typename T>
joint<T>::~joint() {
    if (auto sender = this->_weak_sender.lock()) {
        sender->erase_joint(this->identifier());
    }
}

template <typename T>
void joint<T>::call_first(T const &value) {
    if (this->_handlers.size() > 0) {
        this->handler<T>(0)(value, *this);
    } else {
        throw std::runtime_error("handler not found. must call the end.");
    }
}

template <typename T>
void joint<T>::invalidate() {
    for (any_joint_ptr &sub_joint : this->_sub_joints) {
        sub_joint->invalidate();
    }

    if (auto sender = this->_weak_sender.lock()) {
        sender->erase_joint(this->identifier());
    }

    this->_weak_sender.reset();
    this->_handlers.clear();
    this->_sub_joints.clear();
}

template <typename T>
template <typename P>
void joint<T>::push_handler(any_joint::handler_f<P> handler) {
    this->_handlers.emplace_back(std::move(handler));
}

template <typename T>
std::size_t joint<T>::handlers_size() const {
    return this->_handlers.size();
}

template <typename T>
void joint<T>::add_sub_joint(any_joint_ptr sub_joint) {
    this->_sub_joints.emplace_back(std::move(sub_joint));
}

template <typename T>
void joint<T>::fetch() {
    if (auto sender = this->_weak_sender.lock()) {
        sender->fetch_for(*this);
    }

    for (any_joint_ptr &sub_joint : this->_sub_joints) {
        sub_joint->fetch();
    }
}

template <typename T>
std::any const &joint<T>::any_handler(std::size_t const idx) const {
    return this->_handlers.at(idx);
}

template <typename T>
joint_ptr<T> make_joint(std::weak_ptr<sender<T> const> weak_sender) {
    return std::make_shared<joint<T>>(std::move(weak_sender));
}
}  // namespace yas::chaining
