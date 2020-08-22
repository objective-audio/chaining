//
//  yas_chaining_array_holder_private.h
//

#pragma once

#include <chaining/yas_chaining_chain.h>
#include <cpp_utils/yas_stl_utils.h>

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

template <typename T, enable_if_shared_ptr_t<T, std::nullptr_t> = nullptr>
event make_relayed_event(T const &element, std::size_t const idx, typename T::element_type::send_type const &relayed) {
    return event{relayed_event<T>{.element = element, .index = idx, .relayed = relayed}};
}

template <typename T>
typename holder<T>::vector_t const &holder<T>::raw() const {
    return this->_raw;
}

template <typename T>
typename holder<T>::vector_t &holder<T>::raw() {
    return this->_raw;
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
typename holder<T>::chain_t holder<T>::holder<T>::chain() {
    return this->chain_sync();
}

template <typename T>
holder<T>::holder() {
}

template <typename T>
void holder<T>::replace(std::vector<T> vec) {
    this->_replace_all(std::move(vec));
}

template <typename T>
void holder<T>::replace(T value, std::size_t const idx) {
    this->_replace(std::move(value), idx);
}

template <typename T>
void holder<T>::push_back(T value) {
    std::size_t const idx = this->_raw.size();
    this->_insert(std::move(value), idx);
}

template <typename T>
void holder<T>::insert(T value, std::size_t const idx) {
    this->_insert(std::move(value), idx);
}

template <typename T>
T holder<T>::erase_at(std::size_t const idx) {
    if (this->_observers.size() > idx) {
        if (typename holder<T>::wrapper_ptr &wrapper = this->_observers.at(idx)) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
        }
        yas::erase_at(this->_observers, idx);
    }

    T removed = this->_raw.at(idx);
    yas::erase_at(this->_raw, idx);

    this->broadcast(make_erased_event<T>(idx));

    return removed;
}

template <typename T>
void holder<T>::clear() {
    for (typename holder<T>::wrapper_ptr &wrapper : this->_observers) {
        if (wrapper) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
        }
    }

    this->_raw.clear();
    this->_observers.clear();

    this->broadcast(make_any_event(this->_raw));
}

template <typename T>
void holder<T>::receive_value(vector::event const &event) {
    switch (event.type()) {
        case event_type::fetched: {
            auto const &fetched = event.get<vector::fetched_event<T>>();
            this->_replace_all(fetched.elements);
        } break;
        case event_type::any: {
            auto const &any = event.get<vector::any_event<T>>();
            this->_replace_all(any.elements);
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
void holder<T>::fetch_for(any_joint const &joint) const {
    this->send_value_to_target(make_fetched_event(this->raw()), joint.identifier());
}

template <typename T>
void holder<T>::_prepare(std::vector<T> &&vec) {
    this->_replace_all(std::move(vec));
}

template <typename T>
typename holder<T>::chaining_f holder<T>::_element_chaining() {
    auto weak_holder = to_weak(std::dynamic_pointer_cast<typename vector::holder<T>>(this->shared_from_this()));
    return [weak_holder](T &element, wrapper_ptr &wrapper) {
        auto weak_element = to_weak(element);
        typename holder<T>::wrapper_wptr weak_wrapper = wrapper;
        wrapper->observer = element->chain_unsync()
                                .perform([weak_holder, weak_wrapper, weak_element](auto const &relayed) {
                                    auto holder = weak_holder.lock();
                                    auto element = weak_element.lock();
                                    wrapper_ptr wrapper = weak_wrapper.lock();
                                    if (holder && wrapper && element) {
                                        if (auto idx = yas::index(holder->_observers, wrapper)) {
                                            holder->broadcast(make_relayed_event(element, *idx, relayed));
                                        }
                                    }
                                })
                                .end();
    };
}

template <typename T>
void holder<T>::_replace(std::vector<T> &&vec, chaining_f chaining) {
    for (auto &wrapper : this->_observers) {
        if (any_observer_ptr &observer = wrapper->observer) {
            observer->invalidate();
        }
    }

    this->_observers.clear();
    this->_raw.clear();

    if (chaining) {
        for (T &element : vec) {
            auto wrapper = std::make_shared<observer_wrapper>();
            chaining(element, wrapper);
            this->_observers.emplace_back(std::move(wrapper));
        }
    }

    this->_raw = std::move(vec);

    this->broadcast(make_any_event(this->_raw));
}

template <typename T>
void holder<T>::_replace(T &&element, std::size_t const idx, chaining_f chaining) {
    if (idx < this->_observers.size()) {
        if (auto &observer = this->_observers.at(idx)->observer) {
            observer->invalidate();
        }
    }

    if (chaining) {
        auto wrapper = std::make_shared<observer_wrapper>();
        chaining(element, wrapper);
        this->_observers.at(idx) = std::move(wrapper);
    }

    this->_raw.at(idx) = std::move(element);

    this->broadcast(make_replaced_event(this->_raw.at(idx), idx));
}

template <typename T>
void holder<T>::_insert(T &&element, std::size_t const idx, chaining_f chaining) {
    if (chaining) {
        auto wrapper = std::make_shared<observer_wrapper>();
        chaining(element, wrapper);
        this->_observers.insert(this->_observers.begin() + idx, std::move(wrapper));
    }

    this->_raw.insert(this->_raw.begin() + idx, std::move(element));

    this->broadcast(make_inserted_event(this->_raw.at(idx), idx));
}

template <typename T>
holder_ptr<T> holder<T>::make_shared() {
    return make_shared(vector_t{});
}

template <typename T>
holder_ptr<T> holder<T>::make_shared(vector_t vec) {
    auto shared = std::shared_ptr<holder<T>>(new holder<T>{});
    shared->_prepare(std::move(vec));
    return shared;
}
}  // namespace yas::chaining::vector
