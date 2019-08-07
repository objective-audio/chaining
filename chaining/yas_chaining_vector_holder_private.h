//
//  yas_chaining_array_holder_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>
#include "yas_chaining_chain.h"

namespace yas::chaining::vector {
template <typename T>
event make_fetched_event(std::vector<T> const &elements) {
    return event{fetched_event<T>{.elements = elements}};
}

template <typename T>
event make_any_event(std::vector<T> const &elements) {
    return event{any_event<T>{.elements = elements}};
}

template <typename T>
event make_inserted_event(T const &element, std::size_t const idx) {
    return event{inserted_event<T>{.element = element, .index = idx}};
}

template <typename T>
event make_erased_event(std::size_t const idx) {
    return event{erased_event<T>{.index = idx}};
}

template <typename T>
event make_replaced_event(T const &element, std::size_t const idx) {
    return event{replaced_event<T>{.element = element, .index = idx}};
}

template <typename T>
event make_relayed_event(T const &element, std::size_t const idx, typename T::SendType const &relayed) {
    return event{relayed_event<T>{.element = element, .index = idx, .relayed = relayed}};
}

template <typename T>
struct holder<T>::impl : sender<event>::impl, weakable_impl {
    std::vector<T> _raw;
    std::vector<wrapper_ptr> _observers;

    template <typename Element = T, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace_all(std::vector<T> vec) {
        this->_replace(std::move(vec), this->_element_chaining());
    }

    template <typename Element = T, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace_all(std::vector<T> vec) {
        this->_replace(std::move(vec), nullptr);
    }

    template <typename Element = T, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace(T element, std::size_t const idx) {
        this->_replace(std::move(element), idx, this->_element_chaining());
    }

    template <typename Element = T, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace(T element, std::size_t const idx) {
        this->_replace(std::move(element), idx, nullptr);
    }

    template <typename Element = T, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert(T element, std::size_t const idx) {
        this->_insert(std::move(element), idx, this->_element_chaining());
    }

    template <typename Element = T, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert(T element, std::size_t const idx) {
        this->_insert(std::move(element), idx, nullptr);
    }

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(make_fetched_event(this->_raw), joint.identifier());
    }

    template <typename Element = T, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    chaining_f _element_chaining() {
        auto weak_holder_impl = to_weak(std::dynamic_pointer_cast<holder<T>::impl>(shared_from_this()));
        return [weak_holder_impl](T &element, wrapper_ptr &wrapper) {
            auto weak_element = to_weak(element);
            wrapper_wptr weak_wrapper = wrapper;
            wrapper->observer = element.sendable()
                                    ->chain_unsync()
                                    .perform([weak_holder_impl, weak_wrapper, weak_element](auto const &relayed) {
                                        auto holder_impl = weak_holder_impl.lock();
                                        auto element = weak_element.lock();
                                        wrapper_ptr wrapper = weak_wrapper.lock();
                                        if (holder_impl && wrapper && element) {
                                            if (auto idx = yas::index(holder_impl->_observers, wrapper)) {
                                                holder_impl->broadcast(make_relayed_event(*element, *idx, relayed));
                                            }
                                        }
                                    })
                                    .end();
        };
    }

    void _replace(std::vector<T> &&vec, chaining_f chaining) {
        for (wrapper_ptr &wrapper : this->_observers) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
        }

        this->_observers.clear();
        this->_raw.clear();

        if (chaining) {
            for (T &element : vec) {
                wrapper_ptr wrapper = std::make_shared<observer_wrapper>();
                chaining(element, wrapper);
                this->_observers.emplace_back(std::move(wrapper));
            }
        }

        this->_raw = std::move(vec);

        this->broadcast(make_any_event(this->_raw));
    }

    void _replace(T &&element, std::size_t const idx, chaining_f chaining) {
        if (idx < this->_observers.size()) {
            if (auto &observer = this->_observers.at(idx)->observer) {
                observer->invalidate();
            }
        }

        if (chaining) {
            wrapper_ptr wrapper = std::make_shared<observer_wrapper>();
            chaining(element, wrapper);
            this->_observers.at(idx) = std::move(wrapper);
        }

        this->_raw.at(idx) = std::move(element);

        this->broadcast(make_replaced_event(this->_raw.at(idx), idx));
    }

    void _insert(T &&element, std::size_t const idx, chaining_f chaining) {
        if (chaining) {
            wrapper_ptr wrapper = std::make_shared<observer_wrapper>();
            chaining(element, wrapper);
            this->_observers.insert(this->_observers.begin() + idx, std::move(wrapper));
        }

        this->_raw.insert(this->_raw.begin() + idx, std::move(element));

        this->broadcast(make_inserted_event(this->_raw.at(idx), idx));
    }
};

namespace utils {
    template <typename T, enable_if_base_of_sender_t<T, std::nullptr_t> = nullptr>
    void replace_all(vector::holder<T> &holder, std::vector<T> vec) {
        auto impl_ptr = holder.template impl_ptr<typename vector::holder<T>::impl>();
        impl_ptr->_replace(std::move(vec), impl_ptr->_element_chaining());
    }

    template <typename T, disable_if_base_of_sender_t<T, std::nullptr_t> = nullptr>
    void replace_all(vector::holder<T> &holder, std::vector<T> vec) {
        auto impl_ptr = holder.template impl_ptr<typename vector::holder<T>::impl>();
        impl_ptr->_replace(std::move(vec), nullptr);
    }
}  // namespace utils

template <typename T>
typename holder<T>::vector_t const &holder<T>::raw() const {
    return this->template impl_ptr<impl>()->_raw;
}

template <typename T>
typename holder<T>::vector_t &holder<T>::raw() {
    return this->template impl_ptr<impl>()->_raw;
}

template <typename T>
T const &holder<T>::at(std::size_t const idx) const {
    return this->raw().at(idx);
}

template <typename T>
std::size_t holder<T>::size() const {
    return this->raw().size();
}

template <typename T>
typename holder<T>::chain_t holder<T>::holder<T>::chain() const {
    return this->template impl_ptr<impl>()->chain_sync();
}

template <typename T>
holder<T>::holder(std::vector<T> vec) : sender<event>(std::make_shared<impl>()) {
    this->_prepare(std::move(vec));
}

template <typename T>
holder<T>::holder(std::shared_ptr<impl> &&ptr) : sender<event>(std::move(ptr)) {
}

template <typename T>
holder<T>::~holder() = default;

template <typename T>
void holder<T>::replace(std::vector<T> vec) {
    utils::replace_all(*this, std::move(vec));
}

template <typename T>
void holder<T>::replace(T value, std::size_t const idx) {
    this->template impl_ptr<impl>()->replace(std::move(value), idx);
}

template <typename T>
void holder<T>::push_back(T value) {
    auto impl_ptr = this->template impl_ptr<impl>();
    std::size_t const idx = impl_ptr->_raw.size();
    impl_ptr->insert(std::move(value), idx);
}

template <typename T>
void holder<T>::insert(T value, std::size_t const idx) {
    this->template impl_ptr<impl>()->insert(std::move(value), idx);
}

template <typename T>
T holder<T>::erase_at(std::size_t const idx) {
    auto impl_ptr = this->template impl_ptr<impl>();
    if (impl_ptr->_observers.size() > idx) {
        if (wrapper_ptr &wrapper = impl_ptr->_observers.at(idx)) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
        }
        yas::erase_at(impl_ptr->_observers, idx);
    }

    T removed = impl_ptr->_raw.at(idx);
    yas::erase_at(impl_ptr->_raw, idx);

    impl_ptr->broadcast(make_erased_event<T>(idx));

    return removed;
}

template <typename T>
void holder<T>::clear() {
    auto impl_ptr = this->template impl_ptr<impl>();
    for (wrapper_ptr &wrapper : impl_ptr->_observers) {
        if (wrapper) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
        }
    }

    impl_ptr->_raw.clear();
    impl_ptr->_observers.clear();

    impl_ptr->broadcast(make_any_event(impl_ptr->_raw));
}

template <typename T>
void holder<T>::receive_value(vector::event const &event) {
    switch (event.type()) {
        case event_type::fetched: {
            auto const &fetched = event.get<vector::fetched_event<T>>();
            this->template impl_ptr<impl>()->replace_all(fetched.elements);
        } break;
        case event_type::any: {
            auto const &any = event.get<vector::any_event<T>>();
            this->template impl_ptr<impl>()->replace_all(any.elements);
        } break;
        case event_type::inserted: {
            auto const &inserted = event.get<vector::inserted_event<T>>();
            this->insert(inserted.element, inserted.index);
        } break;
        case event_type::erased: {
            auto const &erased = event.get<vector::erased_event<T>>();
            this->erase_at(erased.index);
        } break;
        case event_type::replaced: {
            auto const &replaced = event.get<vector::replaced_event<T>>();
            this->replace(replaced.element, replaced.index);
        } break;
        case event_type::relayed:
            break;
    }
}

template <typename T>
std::shared_ptr<weakable_impl> holder<T>::weakable_impl_ptr() const {
    return this->template impl_ptr<impl>();
}

template <typename T>
bool holder<T>::is_equal(sender<event> const &rhs) const {
    auto lhs_impl = this->template impl_ptr<impl>();
    auto rhs_impl = rhs.template impl_ptr<impl>();
    if (lhs_impl && rhs_impl) {
        return lhs_impl->_raw == rhs_impl->_raw;
    } else {
        return false;
    }
}

template <typename T>
void holder<T>::_prepare(std::vector<T> &&vec) {
    this->template impl_ptr<impl>()->replace_all(std::move(vec));
}

template <typename T>
std::shared_ptr<holder<T>> holder<T>::make_shared() {
    return make_shared(vector_t{});
}

template <typename T>
std::shared_ptr<holder<T>> holder<T>::make_shared(vector_t vec) {
    return std::shared_ptr<holder<T>>(new holder<T>{std::move(vec)});
}
}  // namespace yas::chaining::vector
