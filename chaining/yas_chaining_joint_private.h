//
//  yas_chaining_joint_private.h
//

#pragma once

#include <vector>

namespace yas::chaining {
template <typename P>
joint_handler_f<P> const &any_joint::handler(std::size_t const idx) const {
    return this->any_handler(idx)->get<P>();
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
    if (auto lock = std::unique_lock<std::mutex>(this->_send_mutex, std::try_to_lock); lock.owns_lock()) {
        if (this->_handlers.size() > 0) {
            if (auto const &handler = this->handler<T>(0)) {
                handler(value, *this);
            } else {
                throw std::runtime_error("handler type do not match.");
            }
        } else if (!this->_pushed) {
            throw std::runtime_error("handler not pushed. must call the end.");
        }
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
void joint<T>::push_handler(joint_handler_f<P> handler) {
    this->_handlers.emplace_back(any_handler::make_shared(std::move(handler)));
    this->_pushed = true;
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
any_handler_ptr const &joint<T>::any_handler(std::size_t const idx) const {
    return this->_handlers.at(idx);
}

template <typename T>
joint_ptr<T> make_joint(std::weak_ptr<sender<T> const> weak_sender) {
    return std::make_shared<joint<T>>(std::move(weak_sender));
}
}  // namespace yas::chaining
