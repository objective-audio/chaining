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

template <typename T, enable_if_shared_ptr_t<T, std::nullptr_t> = nullptr>
event make_relayed_event(T const &element, std::size_t const idx, typename T::element_type::SendType const &relayed) {
    return event{relayed_event<T>{.element = element, .index = idx, .relayed = relayed}};
}

template <typename T>
struct holder<T>::impl : sender<event>::impl {
    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(T &, wrapper_ptr &)>;

    std::vector<T> _raw;
    std::vector<wrapper_ptr> _observers;

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(make_fetched_event(this->_raw), joint.identifier());
    }
};

namespace utils {
    template <typename T, enable_if_base_of_sender_t<T, std::nullptr_t> = nullptr>
    typename vector::holder<std::shared_ptr<T>>::impl::chaining_f element_chaining(
        vector::holder<std::shared_ptr<T>> &holder) {
        auto weak_holder = to_weak(holder.shared_from_this());
        return [weak_holder](std::shared_ptr<T> &element,
                             typename vector::holder<std::shared_ptr<T>>::impl::wrapper_ptr &wrapper) {
            auto weak_element = to_weak(element);
            typename vector::holder<std::shared_ptr<T>>::impl::wrapper_wptr weak_wrapper = wrapper;
            wrapper->observer =
                element->sendable()
                    ->chain_unsync()
                    .perform([weak_holder, weak_wrapper, weak_element](auto const &relayed) {
                        auto holder = weak_holder.lock();
                        auto element = weak_element.lock();
                        typename vector::holder<std::shared_ptr<T>>::impl::wrapper_ptr wrapper = weak_wrapper.lock();
                        if (holder && wrapper && element) {
                            auto holder_impl =
                                holder->template impl_ptr<typename vector::holder<std::shared_ptr<T>>::impl>();
                            if (auto idx = yas::index(holder_impl->_observers, wrapper)) {
                                holder_impl->broadcast(make_relayed_event(element, *idx, relayed));
                            }
                        }
                    })
                    .end();
        };
    }

    template <typename T>
    void _replace(vector::holder<T> &holder, std::vector<T> &&vec,
                  typename vector::holder<T>::impl::chaining_f chaining) {
        auto impl_ptr = holder.template impl_ptr<typename vector::holder<T>::impl>();

        for (auto &wrapper : impl_ptr->_observers) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
        }

        impl_ptr->_observers.clear();
        impl_ptr->_raw.clear();

        if (chaining) {
            for (T &element : vec) {
                auto wrapper = std::make_shared<typename vector::holder<T>::impl::observer_wrapper>();
                chaining(element, wrapper);
                impl_ptr->_observers.emplace_back(std::move(wrapper));
            }
        }

        impl_ptr->_raw = std::move(vec);

        impl_ptr->broadcast(make_any_event(impl_ptr->_raw));
    }

    template <typename T>
    void _replace(vector::holder<T> &holder, T &&element, std::size_t const idx,
                  typename vector::holder<T>::impl::chaining_f chaining) {
        auto impl_ptr = holder.template impl_ptr<typename vector::holder<T>::impl>();

        if (idx < impl_ptr->_observers.size()) {
            if (auto &observer = impl_ptr->_observers.at(idx)->observer) {
                observer->invalidate();
            }
        }

        if (chaining) {
            auto wrapper = std::make_shared<typename vector::holder<T>::impl::observer_wrapper>();
            chaining(element, wrapper);
            impl_ptr->_observers.at(idx) = std::move(wrapper);
        }

        impl_ptr->_raw.at(idx) = std::move(element);

        impl_ptr->broadcast(make_replaced_event(impl_ptr->_raw.at(idx), idx));
    }

    template <typename T>
    void _insert(vector::holder<T> &holder, T &&element, std::size_t const idx,
                 typename vector::holder<T>::impl::chaining_f chaining) {
        auto impl_ptr = holder.template impl_ptr<typename vector::holder<T>::impl>();

        if (chaining) {
            auto wrapper = std::make_shared<typename vector::holder<T>::impl::observer_wrapper>();
            chaining(element, wrapper);
            impl_ptr->_observers.insert(impl_ptr->_observers.begin() + idx, std::move(wrapper));
        }

        impl_ptr->_raw.insert(impl_ptr->_raw.begin() + idx, std::move(element));

        impl_ptr->broadcast(make_inserted_event(impl_ptr->_raw.at(idx), idx));
    }

    template <typename T, enable_if_base_of_sender_t<T, std::nullptr_t> = nullptr>
    void replace_all(vector::holder<std::shared_ptr<T>> &holder, std::vector<std::shared_ptr<T>> vec) {
        _replace(holder, std::move(vec), utils::element_chaining(holder));
    }

    template <typename T>
    void replace_all(vector::holder<T> &holder, std::vector<T> vec) {
        _replace(holder, std::move(vec), nullptr);
    }

    template <typename T, enable_if_base_of_sender_t<T, std::nullptr_t> = nullptr>
    void replace(vector::holder<std::shared_ptr<T>> &holder, std::shared_ptr<T> element, std::size_t const idx) {
        _replace(holder, std::move(element), idx, utils::element_chaining(holder));
    }

    template <typename T>
    void replace(vector::holder<T> &holder, T element, std::size_t const idx) {
        _replace(holder, std::move(element), idx, nullptr);
    }

    template <typename T, enable_if_base_of_sender_t<T, std::nullptr_t> = nullptr>
    void insert(vector::holder<std::shared_ptr<T>> &holder, std::shared_ptr<T> element, std::size_t const idx) {
        _insert(holder, std::move(element), idx, utils::element_chaining(holder));
    }

    template <typename T>
    void insert(vector::holder<T> &holder, T element, std::size_t const idx) {
        _insert(holder, std::move(element), idx, nullptr);
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
holder<T>::holder() : sender<event>(std::make_shared<impl>()) {
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
    utils::replace(*this, std::move(value), idx);
}

template <typename T>
void holder<T>::push_back(T value) {
    auto impl_ptr = this->template impl_ptr<impl>();
    std::size_t const idx = impl_ptr->_raw.size();
    utils::insert(*this, std::move(value), idx);
}

template <typename T>
void holder<T>::insert(T value, std::size_t const idx) {
    utils::insert(*this, std::move(value), idx);
}

template <typename T>
T holder<T>::erase_at(std::size_t const idx) {
    auto impl_ptr = this->template impl_ptr<impl>();
    if (impl_ptr->_observers.size() > idx) {
        if (typename vector::holder<T>::impl::wrapper_ptr &wrapper = impl_ptr->_observers.at(idx)) {
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
    for (typename vector::holder<T>::impl::wrapper_ptr &wrapper : impl_ptr->_observers) {
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
            utils::replace_all(*this, fetched.elements);
        } break;
        case event_type::any: {
            auto const &any = event.get<vector::any_event<T>>();
            utils::replace_all(*this, any.elements);
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
    utils::replace_all(*this, std::move(vec));
}

template <typename T>
std::shared_ptr<holder<T>> holder<T>::make_shared() {
    return make_shared(vector_t{});
}

template <typename T>
std::shared_ptr<holder<T>> holder<T>::make_shared(vector_t vec) {
    auto shared = std::shared_ptr<holder<T>>(new holder<T>{});
    shared->_prepare(std::move(vec));
    return shared;
}
}  // namespace yas::chaining::vector
