//
//  yas_chaining_map_holder_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>
#include "yas_chaining_chain.h"

namespace yas::chaining::map {
template <typename Key, typename Value>
event make_fetched_event(std::map<Key, Value> const &elements) {
    return event{fetched_event<Key, Value>{.elements = elements}};
}

template <typename Key, typename Value>
event make_any_event(std::map<Key, Value> const &elements) {
    return event{any_event<Key, Value>{.elements = elements}};
}

template <typename Key, typename Value>
event make_inserted_event(std::map<Key, Value> const &elements) {
    return event{inserted_event<Key, Value>{.elements = elements}};
}

template <typename Key, typename Value>
event make_erased_event(std::map<Key, Value> const &elements) {
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

#pragma mark - map::holder::impl

template <typename Key, typename Value>
struct holder<Key, Value>::impl {
    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
        Value *value = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(Key const &, Value &, wrapper_ptr &)>;

    std::map<Key, Value> _raw;
    std::map<Key, wrapper_ptr> _observers;
};

namespace utils {
    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    typename holder<Key, std::shared_ptr<Value>>::impl::chaining_f element_chaining(
        holder<Key, std::shared_ptr<Value>> &holder) {
        auto weak_holder = to_weak(
            std::dynamic_pointer_cast<typename map::holder<Key, std::shared_ptr<Value>>>(holder.shared_from_this()));
        return [weak_holder](Key const &key, std::shared_ptr<Value> &value,
                             typename map::holder<Key, std::shared_ptr<Value>>::impl::wrapper_ptr &wrapper) {
            auto weak_value = to_weak(value);
            typename map::holder<Key, std::shared_ptr<Value>>::impl::wrapper_wptr weak_wrapper = wrapper;
            wrapper->observer = value->chain_unsync()
                                    .perform([weak_holder, weak_wrapper, key, weak_value](auto const &relayed) {
                                        auto holder = weak_holder.lock();
                                        auto value = weak_value.lock();
                                        typename map::holder<Key, std::shared_ptr<Value>>::impl::wrapper_ptr wrapper =
                                            weak_wrapper.lock();
                                        if (holder && wrapper && value) {
                                            holder->broadcast(make_relayed_event(key, value, relayed));
                                        }
                                    })
                                    .end();
        };
    }

    template <typename Key, typename Value>
    void _replace(holder<Key, Value> &holder, std::map<Key, Value> &&map,
                  typename map::holder<Key, Value>::impl::chaining_f chaining) {
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
                if (inserted.second) {
                    typename map::holder<Key, Value>::impl::wrapper_ptr wrapper =
                        std::make_shared<typename map::holder<Key, Value>::impl::observer_wrapper>();
                    wrapper->value = &inserted.first->second;
                    chaining(pair.first, pair.second, wrapper);
                    impl_ptr->_observers.emplace(pair.first, std::move(wrapper));
                }
            }
        } else {
            impl_ptr->_raw = std::move(map);
        }

        holder.broadcast(make_any_event(impl_ptr->_raw));
    }

    template <typename Key, typename Value>
    void _erase_observer_for_key(holder<Key, Value> &holder, Key const &key) {
        auto &impl_ptr = holder._impl;

        if (impl_ptr->_observers.count(key) > 0) {
            auto &wrapper = impl_ptr->_observers.at(key);
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
            impl_ptr->_observers.erase(key);
        }
    }

    template <typename Key, typename Value>
    void _insert_or_replace(holder<Key, Value> &holder, Key &&key, Value &&value,
                            typename map::holder<Key, Value>::impl::chaining_f chaining) {
        auto &impl_ptr = holder._impl;

        utils::_erase_observer_for_key(holder, key);

        bool isErased = false;
        if (impl_ptr->_raw.count(key) > 0) {
            impl_ptr->_raw.erase(key);
            isErased = true;
        }

        auto inserted = impl_ptr->_raw.emplace(key, value);

        if (chaining && inserted.second) {
            typename map::holder<Key, Value>::impl::wrapper_ptr wrapper =
                std::make_shared<typename map::holder<Key, Value>::impl::observer_wrapper>();
            wrapper->value = &inserted.first->second;
            chaining(key, value, wrapper);
            impl_ptr->_observers.emplace(key, std::move(wrapper));
        }

        if (isErased) {
            holder.broadcast(make_replaced_event(key, value));
        } else {
            std::map<Key, Value> map{{std::move(key), std::move(value)}};
            holder.broadcast(make_inserted_event(map));
        }
    }

    template <typename Key, typename Value>
    void _insert(holder<Key, Value> &holder, std::map<Key, Value> &&map,
                 typename map::holder<Key, Value>::impl::chaining_f chaining) {
        auto &impl_ptr = holder._impl;

        if (chaining) {
            for (auto &pair : map) {
                auto inserted = impl_ptr->_raw.emplace(pair.first, pair.second);
                if (inserted.second) {
                    typename map::holder<Key, Value>::impl::wrapper_ptr wrapper =
                        std::make_shared<typename map::holder<Key, Value>::impl::observer_wrapper>();
                    wrapper->value = &inserted.first->second;
                    chaining(pair.first, pair.second, wrapper);
                    impl_ptr->_observers.emplace(pair.first, std::move(wrapper));
                }
            }
        } else {
            impl_ptr->_raw.insert(map.begin(), map.end());
        }

        holder.broadcast(make_inserted_event(map));
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void replace_all(holder<Key, std::shared_ptr<Value>> &holder, std::map<Key, std::shared_ptr<Value>> map) {
        utils::_replace(holder, std::move(map), utils::element_chaining(holder));
    }

    template <typename Key, typename Value>
    void replace_all(holder<Key, Value> &holder, std::map<Key, Value> map) {
        utils::_replace(holder, std::move(map), nullptr);
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert_or_replace(holder<Key, std::shared_ptr<Value>> &holder, Key key, std::shared_ptr<Value> value) {
        utils::_insert_or_replace(holder, std::move(key), std::move(value), utils::element_chaining(holder));
    }

    template <typename Key, typename Value>
    void insert_or_replace(holder<Key, Value> &holder, Key key, Value value) {
        utils::_insert_or_replace(holder, std::move(key), std::move(value), nullptr);
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert(holder<Key, std::shared_ptr<Value>> &holder, std::map<Key, std::shared_ptr<Value>> map) {
        utils::_insert(holder, std::move(map), utils::element_chaining(holder));
    }

    template <typename Key, typename Value>
    void insert(holder<Key, Value> &holder, std::map<Key, Value> map) {
        utils::_insert(holder, std::move(map), nullptr);
    }
}  // namespace utils

#pragma mark - map::holder

template <typename Key, typename Value>
holder<Key, Value>::holder() : _impl(std::make_unique<impl>()) {
}

template <typename Key, typename Value>
holder<Key, Value>::holder(std::shared_ptr<impl> &&ptr) : sender<event>(std::move(ptr)) {
}

template <typename Key, typename Value>
holder<Key, Value>::~holder() = default;

template <typename Key, typename Value>
std::map<Key, Value> const &holder<Key, Value>::raw() const {
    return this->_impl->_raw;
}

template <typename Key, typename Value>
std::map<Key, Value> &holder<Key, Value>::raw() {
    return this->_impl->_raw;
}

template <typename Key, typename Value>
bool holder<Key, Value>::has_value(Key const &key) const {
    return this->raw().count(key) > 0;
}

template <typename Key, typename Value>
Value const &holder<Key, Value>::at(Key const &key) const {
    return this->raw().at(key);
}

template <typename Key, typename Value>
Value &holder<Key, Value>::at(Key const &key) {
    return this->raw().at(key);
}

template <typename Key, typename Value>
std::size_t holder<Key, Value>::size() const {
    return this->raw().size();
}

template <typename Key, typename Value>
void holder<Key, Value>::replace_all(std::map<Key, Value> map) {
    utils::replace_all(*this, std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert_or_replace(Key key, Value value) {
    utils::insert_or_replace(*this, std::move(key), std::move(value));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(std::map<Key, Value> map) {
    utils::insert(*this, std::move(map));
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
    std::map<Key, Value> erased;

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
std::map<Key, Value> holder<Key, Value>::erase_for_value(Value const &value) {
    return this->erase_if([&value](Key const &, Value const &map_value) { return map_value == value; });
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_for_key(Key const &key) {
    utils::_erase_observer_for_key(*this, key);

    std::map<Key, Value> erased;

    if (this->_impl->_raw.count(key)) {
        erased.emplace(key, std::move(this->_impl->_raw.at(key)));
        this->_impl->_raw.erase(key);
    }

    this->broadcast(make_erased_event(erased));

    return erased;
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
void holder<Key, Value>::receive_value(map::event const &event) {
    switch (event.type()) {
        case event_type::fetched: {
            auto const &fetched = event.get<map::fetched_event<Key, Value>>();
            this->replace_all(fetched.elements);
        } break;
        case event_type::any: {
            auto const &any = event.get<map::any_event<Key, Value>>();
            this->replace_all(any.elements);
        } break;
        case event_type::inserted: {
            auto const &inserted = event.get<map::inserted_event<Key, Value>>();
            this->insert(inserted.elements);
        } break;
        case event_type::erased: {
            auto const &erased = event.get<map::erased_event<Key, Value>>();
            this->erase_if([&erased](Key const &key, Value const &) { return erased.elements.count(key) > 0; });
        } break;
        case event_type::replaced: {
            auto const &replaced = event.get<map::replaced_event<Key, Value>>();
            this->insert_or_replace(replaced.key, replaced.value);
        } break;
        case event_type::relayed:
            break;
    }
}

template <typename Key, typename Value>
void holder<Key, Value>::fetch_for(any_joint const &joint) {
    this->send_value_to_target(make_fetched_event(this->raw()), joint.identifier());
}

template <typename Key, typename Value>
void holder<Key, Value>::_prepare(std::map<Key, Value> &&map) {
    utils::replace_all(*this, std::move(map));
}

template <typename Key, typename Value>
std::shared_ptr<holder<Key, Value>> holder<Key, Value>::make_shared() {
    return make_shared(std::map<Key, Value>{});
}

template <typename Key, typename Value>
std::shared_ptr<holder<Key, Value>> holder<Key, Value>::make_shared(std::map<Key, Value> map) {
    auto shared = std::shared_ptr<holder<Key, Value>>(new holder<Key, Value>{});
    shared->_prepare(std::move(map));
    return shared;
}
}  // namespace yas::chaining::map
