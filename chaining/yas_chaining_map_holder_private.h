//
//  yas_chaining_map_holder_private.h
//

#pragma once

#include <chaining/yas_chaining_chain.h>
#include <cpp_utils/yas_stl_utils.h>

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
event make_relayed_event(Key const &key, Value const &value, typename Value::element_type::send_type const &relayed) {
    return event{relayed_event<Key, Value>{.key = key, .value = value, .relayed = relayed}};
}

#pragma mark - map::holder

template <typename Key, typename Value>
holder<Key, Value>::holder() {
}

template <typename Key, typename Value>
holder<Key, Value>::~holder() = default;

template <typename Key, typename Value>
std::map<Key, Value> const &holder<Key, Value>::value() const {
    return this->_raw;
}

template <typename Key, typename Value>
std::map<Key, Value> &holder<Key, Value>::value() {
    return this->_raw;
}

template <typename Key, typename Value>
bool holder<Key, Value>::has_value(Key const &key) const {
    return this->value().count(key) > 0;
}

template <typename Key, typename Value>
Value const &holder<Key, Value>::at(Key const &key) const {
    return this->value().at(key);
}

template <typename Key, typename Value>
Value &holder<Key, Value>::at(Key const &key) {
    return this->value().at(key);
}

template <typename Key, typename Value>
std::size_t holder<Key, Value>::size() const {
    return this->value().size();
}

template <typename Key, typename Value>
void holder<Key, Value>::replace_all(std::map<Key, Value> map) {
    this->_replace_all(std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert_or_replace(Key key, Value value) {
    this->_insert_or_replace(std::move(key), std::move(value));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(std::map<Key, Value> map) {
    this->_insert(std::move(map));
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
    std::map<Key, Value> erased;

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
std::map<Key, Value> holder<Key, Value>::erase_for_value(Value const &value) {
    return this->erase_if([&value](Key const &, Value const &map_value) { return map_value == value; });
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_for_key(Key const &key) {
    this->_erase_observer_for_key(key);

    std::map<Key, Value> erased;

    if (this->_raw.count(key)) {
        erased.emplace(key, std::move(this->_raw.at(key)));
        this->_raw.erase(key);
    }

    this->broadcast(make_erased_event(erased));

    return erased;
}

template <typename Key, typename Value>
void holder<Key, Value>::clear() {
    for (auto &pair : this->_observers) {
        if (auto &wrapper = pair.second) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
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
void holder<Key, Value>::fetch_for(any_joint const &joint) const {
    this->send_value_to_target(make_fetched_event(this->value()), joint.identifier());
}

template <typename Key, typename Value>
void holder<Key, Value>::_prepare(std::map<Key, Value> &&map) {
    this->_replace_all(std::move(map));
}

template <typename Key, typename Value>
typename holder<Key, Value>::chaining_f holder<Key, Value>::_element_chaining() {
    auto weak_holder = to_weak(std::dynamic_pointer_cast<typename map::holder<Key, Value>>(this->shared_from_this()));
    return [weak_holder](Key const &key, Value &value, wrapper_ptr &wrapper) {
        auto weak_value = to_weak(value);
        wrapper_wptr weak_wrapper = wrapper;
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
void holder<Key, Value>::_replace(std::map<Key, Value> &&map, chaining_f chaining) {
    for (auto &wrapper_pair : this->_observers) {
        if (any_observer_ptr &observer = wrapper_pair.second->observer) {
            observer->invalidate();
        }
    }

    this->_observers.clear();
    this->_raw.clear();

    if (chaining) {
        for (auto &pair : map) {
            auto inserted = this->_raw.emplace(pair.first, pair.second);
            if (inserted.second) {
                wrapper_ptr wrapper = std::make_shared<observer_wrapper>();
                wrapper->value = &inserted.first->second;
                chaining(pair.first, pair.second, wrapper);
                this->_observers.emplace(pair.first, std::move(wrapper));
            }
        }
    } else {
        this->_raw = std::move(map);
    }

    this->broadcast(make_any_event(this->_raw));
}

template <typename Key, typename Value>
void holder<Key, Value>::_erase_observer_for_key(Key const &key) {
    if (this->_observers.count(key) > 0) {
        auto &wrapper = this->_observers.at(key);
        if (any_observer_ptr &observer = wrapper->observer) {
            observer->invalidate();
        }
        this->_observers.erase(key);
    }
}

template <typename Key, typename Value>
void holder<Key, Value>::_insert_or_replace(Key &&key, Value &&value, chaining_f chaining) {
    this->_erase_observer_for_key(key);

    bool isErased = false;
    if (this->_raw.count(key) > 0) {
        this->_raw.erase(key);
        isErased = true;
    }

    auto inserted = this->_raw.emplace(key, value);

    if (chaining && inserted.second) {
        typename map::holder<Key, Value>::wrapper_ptr wrapper =
            std::make_shared<typename map::holder<Key, Value>::observer_wrapper>();
        wrapper->value = &inserted.first->second;
        chaining(key, value, wrapper);
        this->_observers.emplace(key, std::move(wrapper));
    }

    if (isErased) {
        this->broadcast(make_replaced_event(key, value));
    } else {
        std::map<Key, Value> map{{std::move(key), std::move(value)}};
        this->broadcast(make_inserted_event(map));
    }
}

template <typename Key, typename Value>
void holder<Key, Value>::_insert(std::map<Key, Value> &&map, chaining_f chaining) {
    if (chaining) {
        for (auto &pair : map) {
            auto inserted = this->_raw.emplace(pair.first, pair.second);
            if (inserted.second) {
                wrapper_ptr wrapper = std::make_shared<observer_wrapper>();
                wrapper->value = &inserted.first->second;
                chaining(pair.first, pair.second, wrapper);
                this->_observers.emplace(pair.first, std::move(wrapper));
            }
        }
    } else {
        this->_raw.insert(map.begin(), map.end());
    }

    this->broadcast(make_inserted_event(map));
}

template <typename Key, typename Value>
holder_ptr<Key, Value> holder<Key, Value>::make_shared() {
    return make_shared(std::map<Key, Value>{});
}

template <typename Key, typename Value>
holder_ptr<Key, Value> holder<Key, Value>::make_shared(std::map<Key, Value> map) {
    auto shared = std::shared_ptr<holder<Key, Value>>(new holder<Key, Value>{});
    shared->_prepare(std::move(map));
    return shared;
}
}  // namespace yas::chaining::map
