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

template <typename Key, typename Value>
event make_relayed_event(Key const &key, Value const &value, typename Value::SendType const &relayed) {
    return event{relayed_event<Key, Value>{.key = key, .value = value, .relayed = relayed}};
}

#pragma mark - multimap::holder::impl

template <typename Key, typename Value>
struct holder<Key, Value>::impl : sender<event>::impl, weakable_impl {
    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
        Value *value = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(Key const &, Value &, wrapper_ptr &)>;

    std::multimap<Key, Value> _raw;
    std::multimap<Key, wrapper_ptr> _observers;

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

        this->broadcast(make_erased_event(erased));

        return erased;
    }

    void clear() {
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

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(make_fetched_event(this->_raw), joint.identifier());
    }

    template <typename Element = Value, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    chaining_f _element_chaining() {
        auto weak_holder_impl = to_weak(std::dynamic_pointer_cast<holder<Key, Value>::impl>(shared_from_this()));
        return [weak_holder_impl](Key const &key, Value &value, wrapper_ptr &wrapper) {
            wrapper_wptr weak_wrapper = wrapper;
            auto weak_value = to_weak(value);
            wrapper->observer = value.sendable()
                                    ->chain_unsync()
                                    .perform([weak_holder_impl, weak_wrapper, key, weak_value](auto const &relayed) {
                                        auto holder_impl = weak_holder_impl.lock();
                                        auto value = weak_value.lock();
                                        wrapper_ptr wrapper = weak_wrapper.lock();
                                        if (holder_impl && wrapper && value) {
                                            holder_impl->broadcast(make_relayed_event(key, *value, relayed));
                                        }
                                    })
                                    .end();
        };
    }

    void _replace(std::multimap<Key, Value> &&map, chaining_f chaining) {
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

        this->broadcast(make_inserted_event(map));
    }
};

#pragma mark - multimap::holder

template <typename Key, typename Value>
holder<Key, Value>::holder(std::multimap<Key, Value> map) : sender<event>(std::make_shared<impl>()) {
    this->template impl_ptr<impl>()->prepare(std::move(map));
}

template <typename Key, typename Value>
holder<Key, Value>::holder(std::shared_ptr<impl> &&impl) : sender<event>(std::move(impl)) {
}

template <typename Key, typename Value>
holder<Key, Value>::~holder() = default;

template <typename Key, typename Value>
std::multimap<Key, Value> const &holder<Key, Value>::raw() const {
    return this->template impl_ptr<impl>()->_raw;
}

template <typename Key, typename Value>
std::multimap<Key, Value> &holder<Key, Value>::raw() {
    return this->template impl_ptr<impl>()->_raw;
}

template <typename Key, typename Value>
std::size_t holder<Key, Value>::size() const {
    return this->raw().size();
}

template <typename Key, typename Value>
void holder<Key, Value>::replace(std::multimap<Key, Value> map) {
    this->template impl_ptr<impl>()->replace(std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(std::multimap<Key, Value> map) {
    this->template impl_ptr<impl>()->insert(std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(Key key, Value value) {
    this->insert(std::multimap<Key, Value>{{std::move(key), std::move(value)}});
}

template <typename Key, typename Value>
std::multimap<Key, Value> holder<Key, Value>::erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
    return this->template impl_ptr<impl>()->erase_if(handler);
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
    this->template impl_ptr<impl>()->clear();
}

template <typename Key, typename Value>
typename holder<Key, Value>::chain_t holder<Key, Value>::holder<Key, Value>::chain() const {
    return this->template impl_ptr<impl>()->chain_sync();
}

template <typename Key, typename Value>
std::shared_ptr<weakable_impl> holder<Key, Value>::weakable_impl_ptr() const {
    return this->template impl_ptr<impl>();
}

template <typename Key, typename Value>
bool holder<Key, Value>::is_equal(sender<event> const &rhs) const {
    auto lhs_impl = this->template impl_ptr<impl>();
    auto rhs_impl = rhs.template impl_ptr<impl>();
    if (lhs_impl && rhs_impl) {
        return lhs_impl->_raw == rhs_impl->_raw;
    } else {
        return false;
    }
}

template <typename Key, typename Value>
std::shared_ptr<holder<Key, Value>> holder<Key, Value>::make_shared() {
    return make_shared(std::multimap<Key, Value>{});
}

template <typename Key, typename Value>
std::shared_ptr<holder<Key, Value>> holder<Key, Value>::make_shared(std::multimap<Key, Value> map) {
    return std::shared_ptr<holder<Key, Value>>(new holder<Key, Value>{std::move(map)});
}
}  // namespace yas::chaining::multimap
