//
//  yas_chaining_map_holder_private.h
//

#pragma once

#include <chaining/yas_chaining_chain.h>
#include <cpp_utils/yas_stl_utils.h>

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
event make_relayed_event(Key const &key, Value const &value, typename Value::element_type::send_type const &relayed) {
    return event{relayed_event<Key, Value>{.key = key, .value = value, .relayed = relayed}};
}

#pragma mark - multimap::holder

template <typename Key, typename Value>
holder<Key, Value>::holder() {
}

template <typename Key, typename Value>
holder<Key, Value>::~holder() = default;

template <typename Key, typename Value>
std::multimap<Key, Value> const &holder<Key, Value>::raw() const {
    return this->_raw;
}

template <typename Key, typename Value>
std::multimap<Key, Value> &holder<Key, Value>::raw() {
    return this->_raw;
}

template <typename Key, typename Value>
std::size_t holder<Key, Value>::size() const {
    return this->raw().size();
}

template <typename Key, typename Value>
void holder<Key, Value>::replace(std::multimap<Key, Value> map) {
    this->_replace(std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(std::multimap<Key, Value> map) {
    this->_insert(std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(Key key, Value value) {
    this->insert(std::multimap<Key, Value>{{std::move(key), std::move(value)}});
}

template <typename Key, typename Value>
std::multimap<Key, Value> holder<Key, Value>::erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
    std::multimap<Key, Value> erased;

    if (this->_observers.size() > 0) {
        yas::erase_if(this->_observers, [&handler](std::pair<Key, wrapper_ptr> const &pair) {
            return handler(pair.first, *pair.second->value);
        });
    }

    yas::erase_if(this->_raw, [&handler, &erased](std::pair<Key, Value> const &pair) {
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
    for (auto &pair : this->_observers) {
        if (auto const &observer = pair.second->observer) {
            observer.value()->invalidate();
        }
    }

    this->_observers.clear();
    this->_raw.clear();

    this->broadcast(make_any_event(this->_raw));
}

template <typename Key, typename Value>
typename holder<Key, Value>::chain_t holder<Key, Value>::holder<Key, Value>::chain() {
    return this->chain_sync();
}

template <typename Key, typename Value>
void holder<Key, Value>::fetch_for(any_joint const &joint) const {
    this->send_value_to_target(make_fetched_event(this->raw()), joint.identifier());
}

template <typename Key, typename Value>
void holder<Key, Value>::_prepare(std::multimap<Key, Value> &&map) {
    this->_replace(std::move(map));
}

template <typename Key, typename Value>
typename multimap::holder<Key, Value>::chaining_f holder<Key, Value>::_element_chaining() {
    auto weak_holder =
        to_weak(std::dynamic_pointer_cast<typename multimap::holder<Key, Value>>(this->shared_from_this()));
    return [weak_holder](Key const &key, Value &value, wrapper_ptr &wrapper) {
        wrapper_wptr weak_wrapper = wrapper;
        auto weak_value = to_weak(value);
        wrapper->observer = value->chain_unsync()
                                .perform([weak_holder, weak_wrapper, key, weak_value](auto const &relayed) {
                                    auto holder = weak_holder.lock();
                                    auto value = weak_value.lock();
                                    wrapper_ptr wrapper = weak_wrapper.lock();
                                    if (holder && wrapper && value) {
                                        holder->broadcast(make_relayed_event(key, value, relayed));
                                    }
                                })
                                .end();
    };
}

template <typename Key, typename Value>
void holder<Key, Value>::_replace(std::multimap<Key, Value> &&map, chaining_f chaining) {
    for (auto &wrapper_pair : this->_observers) {
        if (auto const &observer = wrapper_pair.second->observer) {
            observer.value()->invalidate();
        }
    }

    this->_observers.clear();
    this->_raw.clear();

    if (chaining) {
        for (auto &pair : map) {
            auto inserted = this->_raw.emplace(pair.first, pair.second);
            wrapper_ptr wrapper = std::make_shared<observer_wrapper>();
            wrapper->value = &inserted->second;
            chaining(pair.first, pair.second, wrapper);
            this->_observers.emplace(pair.first, std::move(wrapper));
        }
    } else {
        this->_raw = std::move(map);
    }

    this->broadcast(make_any_event(this->_raw));
}

template <typename Key, typename Value>
void holder<Key, Value>::_insert(std::multimap<Key, Value> &&map, chaining_f chaining) {
    if (chaining) {
        for (auto &pair : map) {
            auto inserted = this->_raw.emplace(pair.first, pair.second);
            wrapper_ptr wrapper = std::make_shared<observer_wrapper>();
            wrapper->value = &inserted->second;
            chaining(pair.first, pair.second, wrapper);
            this->_observers.emplace(pair.first, std::move(wrapper));
        }
    } else {
        this->_raw.insert(map.begin(), map.end());
    }

    this->broadcast(make_inserted_event(map));
}

template <typename Key, typename Value>
holder_ptr<Key, Value> holder<Key, Value>::make_shared() {
    return make_shared(std::multimap<Key, Value>{});
}

template <typename Key, typename Value>
holder_ptr<Key, Value> holder<Key, Value>::make_shared(std::multimap<Key, Value> map) {
    auto shared = std::shared_ptr<holder<Key, Value>>(new holder<Key, Value>{});
    shared->_prepare(std::move(map));
    return shared;
}
}  // namespace yas::chaining::multimap
