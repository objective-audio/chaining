//
//  yas_chaining_map_holder_private.h
//

#pragma once

#include "yas_chaining_chain.h"
#include "yas_stl_utils.h"

namespace yas::chaining::multimap {

#pragma mark - multimap::immutable_holder

template <typename Key, typename Value>
struct immutable_holder<Key, Value>::impl : sender<event<Key, Value>>::impl {
    struct observer_wrapper {
        any_observer observer = nullptr;
        Value *value = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(Key const &, Value &, wrapper_ptr &)>;

    void prepare(std::multimap<Key, Value> &&map) {
        this->replace(std::move(map));
    }

    template <typename Element = Value, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace(std::multimap<Key, Value> &&map) {
        this->_replace(std::move(map), this->_element_chaining());
    }

    template <typename Element = Value, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace(std::multimap<Key, Value> &&map) {
        this->_replace(std::move(map), nullptr);
    }

    template <typename Element = Value, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert(std::multimap<Key, Value> &&map) {
        this->_insert(std::move(map), this->_element_chaining());
    }

    template <typename Element = Value, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert(std::multimap<Key, Value> &&map) {
        this->_insert(std::move(map), nullptr);
    }

    std::multimap<Key, Value> erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
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

        this->broadcast(event<Key, Value>{event_type::erased, erased});

        return erased;
    }

    void clear() {
        for (auto &pair : this->_observers) {
            if (auto &wrapper = pair.second) {
                if (any_observer &observer = wrapper->observer) {
                    observer.invalidate();
                }
            }
        }

        this->_observers.clear();
        this->_raw.clear();

        this->broadcast(event<Key, Value>{event_type::any, this->_raw});
    }

    std::multimap<Key, Value> &raw() {
        return this->_raw;
    }

    virtual bool is_equal(std::shared_ptr<base::impl> const &rhs) const override {
        if (auto rhs_impl = std::dynamic_pointer_cast<typename immutable_holder<Key, Value>::impl>(rhs)) {
            return this->_raw == rhs_impl->_raw;
        } else {
            return false;
        }
    }

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(event<Key, Value>{event_type::fetched, this->_raw}, joint.identifier());
    }

   private:
    std::multimap<Key, Value> _raw;
    std::multimap<Key, wrapper_ptr> _observers;

    template <typename Element = Value, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    chaining_f _element_chaining() {
        auto weak_holder = to_weak(this->template cast<immutable_holder<Key, Value>>());
        return [weak_holder](Key const &key, Value &value, wrapper_ptr &wrapper) {
            wrapper_wptr weak_wrapper = wrapper;
            wrapper->observer = value.sendable()
                                    .chain_unsync()
                                    .perform([weak_holder, weak_wrapper, key](Value const &value) {
                                        auto holder = weak_holder.lock();
                                        wrapper_ptr wrapper = weak_wrapper.lock();
                                        if (holder && wrapper) {
                                            std::multimap<Key, Value> elements{{key, value}};
                                            holder.template impl_ptr<impl>()->broadcast(
                                                event<Key, Value>{event_type::relayed, elements});
                                        }
                                    })
                                    .end();
        };
    }

    void _replace(std::multimap<Key, Value> &&map, chaining_f chaining) {
        for (auto &wrapper_pair : this->_observers) {
            if (any_observer &observer = wrapper_pair.second->observer) {
                observer.invalidate();
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

        this->broadcast(event<Key, Value>{event_type::any, this->_raw});
    }

    void _insert(std::multimap<Key, Value> &&map, chaining_f chaining) {
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

        this->broadcast(event<Key, Value>{event_type::inserted, map});
    }
};

template <typename Key, typename Value>
immutable_holder<Key, Value>::immutable_holder(std::shared_ptr<impl> &&imp)
    : sender<event<Key, Value>>(std::move(imp)) {
}

template <typename Key, typename Value>
immutable_holder<Key, Value>::immutable_holder(std::nullptr_t) : sender<event<Key, Value>>(nullptr) {
}

template <typename Key, typename Value>
std::multimap<Key, Value> const &immutable_holder<Key, Value>::raw() const {
    return this->template impl_ptr<impl>()->raw();
}

template <typename Key, typename Value>
std::size_t immutable_holder<Key, Value>::size() const {
    return this->raw().size();
}

template <typename Key, typename Value>
typename immutable_holder<Key, Value>::chain_t immutable_holder<Key, Value>::immutable_holder<Key, Value>::chain() {
    return this->template impl_ptr<impl>()->chain_sync();
}

#pragma mark - multimap::holder

template <typename Key, typename Value>
holder<Key, Value>::holder() : immutable_holder<Key, Value>(std::make_shared<immutable_impl>()) {
}

template <typename Key, typename Value>
holder<Key, Value>::holder(std::multimap<Key, Value> map)
    : immutable_holder<Key, Value>(std::make_shared<immutable_impl>()) {
    this->template impl_ptr<immutable_impl>()->prepare(std::move(map));
}

template <typename Key, typename Value>
holder<Key, Value>::holder(std::nullptr_t) : immutable_holder<Key, Value>(nullptr) {
}

template <typename Key, typename Value>
holder<Key, Value>::~holder() = default;

template <typename Key, typename Value>
std::multimap<Key, Value> &holder<Key, Value>::raw() {
    return this->template impl_ptr<immutable_impl>()->raw();
}

template <typename Key, typename Value>
void holder<Key, Value>::replace(std::multimap<Key, Value> map) {
    this->template impl_ptr<immutable_impl>()->replace(std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(std::multimap<Key, Value> map) {
    this->template impl_ptr<immutable_impl>()->insert(std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(Key key, Value value) {
    this->insert(std::multimap<Key, Value>{{std::move(key), std::move(value)}});
}

template <typename Key, typename Value>
std::multimap<Key, Value> holder<Key, Value>::erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
    return this->template impl_ptr<immutable_impl>()->erase_if(handler);
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
    this->template impl_ptr<immutable_impl>()->clear();
}
}  // namespace yas::chaining::multimap
