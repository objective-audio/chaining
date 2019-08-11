//
//  yas_chaining_map_holder_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>
#include "yas_chaining_chain.h"

namespace yas::chaining::multimap {
template <typename Key, typename Value>
event make_fetched_event(std::multimap<Key, Value> const &elements) {
    return event{fetched_event<Key, Value>{.elements = elements}};
}

template <typename Key, typename Value>
event make_any_event(std::multimap<Key, Value> const &elements) {
    return event{any_event<Key, Value>{.elements = elements}};
}

template <typename Key, typename Value>
event make_inserted_event(std::multimap<Key, Value> const &elements) {
    return event{inserted_event<Key, Value>{.elements = elements}};
}

template <typename Key, typename Value>
event make_erased_event(std::multimap<Key, Value> const &elements) {
    return event{erased_event<Key, Value>{.elements = elements}};
}

template <typename Key, typename Value>
event make_replaced_event(Key const &key, Value const &value) {
    return event{replaced_event<Key, Value>{.key = key, .value = value}};
}

template <typename Key, typename Value, enable_if_shared_ptr_t<Value, std::nullptr_t> = nullptr>
event make_relayed_event(Key const &key, Value const &value, typename Value::element_type::SendType const &relayed) {
    return event{relayed_event<Key, Value>{.key = key, .value = value, .relayed = relayed}};
}

#pragma mark - multimap::holder::impl

template <typename Key, typename Value>
struct holder<Key, Value>::impl {
    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
        Value *value = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(Key const &, Value &, wrapper_ptr &)>;

    std::multimap<Key, Value> _raw;
    std::multimap<Key, wrapper_ptr> _observers;
};

namespace utils {
    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    typename multimap::holder<Key, std::shared_ptr<Value>>::impl::chaining_f _element_chaining(
        holder<Key, std::shared_ptr<Value>> &holder) {
        auto weak_holder = to_weak(std::dynamic_pointer_cast<typename multimap::holder<Key, std::shared_ptr<Value>>>(
            holder.shared_from_this()));
        return [weak_holder](Key const &key, std::shared_ptr<Value> &value,
                             typename multimap::holder<Key, std::shared_ptr<Value>>::impl::wrapper_ptr &wrapper) {
            typename multimap::holder<Key, std::shared_ptr<Value>>::impl::wrapper_wptr weak_wrapper = wrapper;
            auto weak_value = to_weak(value);
            wrapper->observer =
                value->sendable()
                    ->chain_unsync()
                    .perform([weak_holder, weak_wrapper, key, weak_value](auto const &relayed) {
                        auto holder = weak_holder.lock();
                        auto value = weak_value.lock();
                        typename multimap::holder<Key, std::shared_ptr<Value>>::impl::wrapper_ptr wrapper =
                            weak_wrapper.lock();
                        if (holder && wrapper && value) {
                            holder->broadcast(make_relayed_event(key, value, relayed));
                        }
                    })
                    .end();
        };
    }

    template <typename Key, typename Value>
    void _replace(holder<Key, Value> &holder, std::multimap<Key, Value> &&map,
                  typename multimap::holder<Key, Value>::impl::chaining_f chaining) {
        auto &impl_ptr = holder._impl;

        for (auto &wrapper_pair : impl_ptr->_observers) {
            if (any_observer_ptr &observer = wrapper_pair.second->observer) {
                observer->invalidate();
            }
        }

        impl_ptr->_observers.clear();
        impl_ptr->_raw.clear();

        if (chaining) {
            for (auto &pair : map) {
                auto inserted = impl_ptr->_raw.emplace(pair.first, pair.second);
                typename multimap::holder<Key, Value>::impl::wrapper_ptr wrapper =
                    std::make_shared<typename multimap::holder<Key, Value>::impl::observer_wrapper>();
                wrapper->value = &inserted->second;
                chaining(pair.first, pair.second, wrapper);
                impl_ptr->_observers.emplace(pair.first, std::move(wrapper));
            }
        } else {
            impl_ptr->_raw = std::move(map);
        }

        holder.broadcast(make_any_event(impl_ptr->_raw));
    }

    template <typename Key, typename Value>
    void _insert(holder<Key, Value> &holder, std::multimap<Key, Value> &&map,
                 typename multimap::holder<Key, Value>::impl::chaining_f chaining) {
        auto &impl_ptr = holder._impl;

        if (chaining) {
            for (auto &pair : map) {
                auto inserted = impl_ptr->_raw.emplace(pair.first, pair.second);
                typename multimap::holder<Key, Value>::impl::wrapper_ptr wrapper =
                    std::make_shared<typename multimap::holder<Key, Value>::impl::observer_wrapper>();
                wrapper->value = &inserted->second;
                chaining(pair.first, pair.second, wrapper);
                impl_ptr->_observers.emplace(pair.first, std::move(wrapper));
            }
        } else {
            impl_ptr->_raw.insert(map.begin(), map.end());
        }

        holder.broadcast(make_inserted_event(map));
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void replace(holder<Key, std::shared_ptr<Value>> &holder, std::multimap<Key, std::shared_ptr<Value>> &&map) {
        utils::_replace(holder, std::move(map), utils::_element_chaining(holder));
    }

    template <typename Key, typename Value>
    void replace(holder<Key, Value> &holder, std::multimap<Key, Value> &&map) {
        utils::_replace(holder, std::move(map), nullptr);
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert(holder<Key, std::shared_ptr<Value>> &holder, std::multimap<Key, std::shared_ptr<Value>> &&map) {
        utils::_insert(holder, std::move(map), utils::_element_chaining(holder));
    }

    template <typename Key, typename Value>
    void insert(holder<Key, Value> &holder, std::multimap<Key, Value> &&map) {
        utils::_insert(holder, std::move(map), nullptr);
    }
}  // namespace utils

#pragma mark - multimap::holder

template <typename Key, typename Value>
holder<Key, Value>::holder() : _impl(std::make_shared<impl>()) {
}

template <typename Key, typename Value>
holder<Key, Value>::holder(std::shared_ptr<impl> &&impl) : sender<event>(std::move(impl)) {
}

template <typename Key, typename Value>
holder<Key, Value>::~holder() = default;

template <typename Key, typename Value>
std::multimap<Key, Value> const &holder<Key, Value>::raw() const {
    return this->_impl->_raw;
}

template <typename Key, typename Value>
std::multimap<Key, Value> &holder<Key, Value>::raw() {
    return this->_impl->_raw;
}

template <typename Key, typename Value>
std::size_t holder<Key, Value>::size() const {
    return this->raw().size();
}

template <typename Key, typename Value>
void holder<Key, Value>::replace(std::multimap<Key, Value> map) {
    utils::replace(*this, std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(std::multimap<Key, Value> map) {
    utils::insert(*this, std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(Key key, Value value) {
    this->insert(std::multimap<Key, Value>{{std::move(key), std::move(value)}});
}

template <typename Key, typename Value>
std::multimap<Key, Value> holder<Key, Value>::erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
    std::multimap<Key, Value> erased;

    if (this->_impl->_observers.size() > 0) {
        yas::erase_if(this->_impl->_observers, [&handler](std::pair<Key, typename impl::wrapper_ptr> const &pair) {
            return handler(pair.first, *pair.second->value);
        });
    }

    yas::erase_if(this->_impl->_raw, [&handler, &erased](std::pair<Key, Value> const &pair) {
        if (handler(pair.first, pair.second)) {
            erased.insert(pair);
            return true;
        } else {
            return false;
        }
    });

    this->broadcast(make_erased_event(erased));

    return erased;
}

template <typename Key, typename Value>
std::multimap<Key, Value> holder<Key, Value>::erase_for_value(Value const &value) {
    return this->erase_if([&value](Key const &, Value const &map_value) { return map_value == value; });
}

template <typename Key, typename Value>
std::multimap<Key, Value> holder<Key, Value>::erase_for_key(Key const &key) {
    return this->erase_if([&key](Key const &map_key, Value const &) { return map_key == key; });
}

template <typename Key, typename Value>
void holder<Key, Value>::clear() {
    for (auto &pair : this->_impl->_observers) {
        if (auto &wrapper = pair.second) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
        }
    }

    this->_impl->_observers.clear();
    this->_impl->_raw.clear();

    this->broadcast(make_any_event(this->_impl->_raw));
}

template <typename Key, typename Value>
typename holder<Key, Value>::chain_t holder<Key, Value>::holder<Key, Value>::chain() {
    return this->chain_sync();
}

template <typename Key, typename Value>
bool holder<Key, Value>::is_equal(sender<event> const &rhs) const {
    auto sendable_ptr = rhs.shared_from_this();
    auto rhs_ptr = std::dynamic_pointer_cast<typename multimap::holder<Key, Value> const>(sendable_ptr);
    if (rhs_ptr) {
        return this->_impl->_raw == rhs_ptr->_impl->_raw;
    } else {
        return false;
    }
}

template <typename Key, typename Value>
void holder<Key, Value>::fetch_for(any_joint const &joint) {
    this->send_value_to_target(make_fetched_event(this->raw()), joint.identifier());
}

template <typename Key, typename Value>
void holder<Key, Value>::_prepare(std::multimap<Key, Value> &&map) {
    utils::replace(*this, std::move(map));
}

template <typename Key, typename Value>
std::shared_ptr<holder<Key, Value>> holder<Key, Value>::make_shared() {
    return make_shared(std::multimap<Key, Value>{});
}

template <typename Key, typename Value>
std::shared_ptr<holder<Key, Value>> holder<Key, Value>::make_shared(std::multimap<Key, Value> map) {
    auto shared = std::shared_ptr<holder<Key, Value>>(new holder<Key, Value>{});
    shared->_prepare(std::move(map));
    return shared;
}
}  // namespace yas::chaining::multimap
