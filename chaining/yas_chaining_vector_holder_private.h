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
struct immutable_holder<T>::impl : sender<event>::impl {
    struct observer_wrapper {
        any_observer observer = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(T &, wrapper_ptr &)>;

    void prepare(std::vector<T> &&vec) {
        this->replace(std::move(vec));
    }

    template <typename Element = T, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace(std::vector<T> &&vec) {
        this->_replace(std::move(vec), this->_element_chaining());
    }

    template <typename Element = T, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace(std::vector<T> &&vec) {
        this->_replace(std::move(vec), nullptr);
    }

    template <typename Element = T, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace(T &&element, std::size_t const idx) {
        this->_replace(std::move(element), idx, this->_element_chaining());
    }

    template <typename Element = T, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace(T &&element, std::size_t const idx) {
        this->_replace(std::move(element), idx, nullptr);
    }

    void push_back(T &&element) {
        std::size_t const idx = this->_raw.size();
        this->insert(std::move(element), idx);
    }

    template <typename Element = T, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert(T &&element, std::size_t const idx) {
        this->_insert(std::move(element), idx, this->_element_chaining());
    }

    template <typename Element = T, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert(T &&element, std::size_t const idx) {
        this->_insert(std::move(element), idx, nullptr);
    }

    T erase_at(std::size_t const idx) {
        if (this->_observers.size() > idx) {
            if (wrapper_ptr &wrapper = this->_observers.at(idx)) {
                if (any_observer &observer = wrapper->observer) {
                    observer.invalidate();
                }
            }
            yas::erase_at(this->_observers, idx);
        }

        T removed = this->_raw.at(idx);
        yas::erase_at(this->_raw, idx);

        this->broadcast(make_erased_event<T>(idx));

        return removed;
    }

    void clear() {
        for (wrapper_ptr &wrapper : this->_observers) {
            if (wrapper) {
                if (any_observer &observer = wrapper->observer) {
                    observer.invalidate();
                }
            }
        }

        this->_raw.clear();
        this->_observers.clear();

        this->broadcast(make_any_event(this->_raw));
    }

    std::vector<T> &raw() {
        return this->_raw;
    }

    virtual bool is_equal(std::shared_ptr<base::impl> const &rhs) const override {
        if (auto rhs_impl = std::dynamic_pointer_cast<typename immutable_holder<T>::impl>(rhs)) {
            return this->_raw == rhs_impl->_raw;
        } else {
            return false;
        }
    }

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(make_fetched_event(this->_raw), joint.identifier());
    }

   private:
    std::vector<T> _raw;
    std::vector<wrapper_ptr> _observers;

    template <typename Element = T, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    chaining_f _element_chaining() {
        auto weak_holder = to_weak(this->template cast<immutable_holder<T>>());
        return [weak_holder](T &element, wrapper_ptr &wrapper) {
            auto weak_element = to_weak(element);
            wrapper_wptr weak_wrapper = wrapper;
            wrapper->observer = element.sendable()
                                    .chain_unsync()
                                    .perform([weak_holder, weak_wrapper, weak_element](auto const &relayed) {
                                        auto holder = weak_holder.lock();
                                        auto element = weak_element.lock();
                                        wrapper_ptr wrapper = weak_wrapper.lock();
                                        if (holder && wrapper && element) {
                                            auto holder_impl = holder.template impl_ptr<impl>();
                                            if (auto idx = yas::index(holder_impl->_observers, wrapper)) {
                                                holder_impl->broadcast(make_relayed_event(element, *idx, relayed));
                                            }
                                        }
                                    })
                                    .end();
        };
    }

    void _replace(std::vector<T> &&vec, chaining_f chaining) {
        for (wrapper_ptr &wrapper : this->_observers) {
            if (any_observer &observer = wrapper->observer) {
                observer.invalidate();
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
                observer.invalidate();
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

template <typename T>
immutable_holder<T>::immutable_holder(std::shared_ptr<impl> &&imp) : sender<event>(std::move(imp)) {
}

template <typename T>
immutable_holder<T>::immutable_holder(std::nullptr_t) : sender<event>(nullptr) {
}

template <typename T>
typename immutable_holder<T>::vector_t const &immutable_holder<T>::raw() const {
    return this->template impl_ptr<impl>()->raw();
}

template <typename T>
T const &immutable_holder<T>::at(std::size_t const idx) const {
    return this->raw().at(idx);
}

template <typename T>
std::size_t immutable_holder<T>::size() const {
    return this->raw().size();
}

template <typename T>
typename immutable_holder<T>::chain_t immutable_holder<T>::immutable_holder<T>::chain() {
    return this->template impl_ptr<impl>()->chain_sync();
}

template <typename T>
holder<T>::holder() : immutable_holder<T>(std::make_shared<immutable_impl>()) {
}

template <typename T>
holder<T>::holder(std::vector<T> vec) : immutable_holder<T>(std::make_shared<immutable_impl>()) {
    this->template impl_ptr<immutable_impl>()->prepare(std::move(vec));
}

template <typename T>
holder<T>::holder(std::nullptr_t) : immutable_holder<T>(nullptr) {
}

template <typename T>
holder<T>::~holder() = default;

template <typename T>
void holder<T>::replace(std::vector<T> vec) {
    this->template impl_ptr<immutable_impl>()->replace(std::move(vec));
}

template <typename T>
void holder<T>::replace(T value, std::size_t const idx) {
    this->template impl_ptr<immutable_impl>()->replace(std::move(value), idx);
}

template <typename T>
void holder<T>::push_back(T value) {
    this->template impl_ptr<immutable_impl>()->push_back(std::move(value));
}

template <typename T>
void holder<T>::insert(T value, std::size_t const idx) {
    this->template impl_ptr<immutable_impl>()->insert(std::move(value), idx);
}

template <typename T>
T holder<T>::erase_at(std::size_t const idx) {
    return this->template impl_ptr<immutable_impl>()->erase_at(idx);
}

template <typename T>
void holder<T>::clear() {
    this->template impl_ptr<immutable_impl>()->clear();
}
}  // namespace yas::chaining::vector
