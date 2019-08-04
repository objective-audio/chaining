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

template <typename Key, typename Value>
event make_relayed_event(Key const &key, Value const &value, typename Value::SendType const &relayed) {
    return event{relayed_event<Key, Value>{.key = key, .value = value, .relayed = relayed}};
}

#pragma mark - map::holder::impl

template <typename Key, typename Value>
struct holder<Key, Value>::impl : sender<event>::impl, chaining::receivable<event>, weakable_impl {
    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
        Value *value = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(Key const &, Value &, wrapper_ptr &)>;

    void prepare(std::map<Key, Value> &&map) {
        this->replace_all(std::move(map));
    }

    template <typename Element = Value, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace_all(std::map<Key, Element> map) {
        this->_replace(std::move(map), this->_element_chaining());
    }

    template <typename Element = Value, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void replace_all(std::map<Key, Element> map) {
        this->_replace(std::move(map), nullptr);
    }

    template <typename Element = Value, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert_or_replace(Key key, Value value) {
        this->_insert_or_replace(std::move(key), std::move(value), this->_element_chaining());
    }

    template <typename Element = Value, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert_or_replace(Key key, Value value) {
        this->_insert_or_replace(std::move(key), std::move(value), nullptr);
    }

    template <typename Element = Value, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert(std::map<Key, Value> map) {
        this->_insert(std::move(map), this->_element_chaining());
    }

    template <typename Element = Value, disable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    void insert(std::map<Key, Value> map) {
        this->_insert(std::move(map), nullptr);
    }

    std::map<Key, Value> erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
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

    std::map<Key, Value> erase_for_key(Key const &key) {
        this->_erase_observer_for_key(key);

        std::map<Key, Value> erased;

        if (this->_raw.count(key)) {
            erased.emplace(key, std::move(this->_raw.at(key)));
            this->_raw.erase(key);
        }

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

    std::map<Key, Value> &raw() {
        return this->_raw;
    }

    virtual bool is_equal(std::shared_ptr<sender<event>::impl> const &rhs) const override {
        if (auto rhs_impl = std::dynamic_pointer_cast<typename holder<Key, Value>::impl>(rhs)) {
            return this->_raw == rhs_impl->_raw;
        } else {
            return false;
        }
    }

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(make_fetched_event(this->_raw), joint.identifier());
    }

    void receive_value(event const &event) override {
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

   private:
    std::map<Key, Value> _raw;
    std::map<Key, wrapper_ptr> _observers;

    template <typename Element = Value, enable_if_base_of_sender_t<Element, std::nullptr_t> = nullptr>
    chaining_f _element_chaining() {
        auto weak_holder_impl = to_weak(std::dynamic_pointer_cast<holder<Key, Value>::impl>(shared_from_this()));
        return [weak_holder_impl](Key const &key, Value &value, wrapper_ptr &wrapper) {
            auto weak_value = to_weak(value);
            wrapper_wptr weak_wrapper = wrapper;
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

    void _replace(std::map<Key, Value> &&map, chaining_f chaining) {
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

    void _insert_or_replace(Key &&key, Value &&value, chaining_f chaining) {
        this->_erase_observer_for_key(key);

        bool isErased = false;
        if (this->_raw.count(key) > 0) {
            this->_raw.erase(key);
            isErased = true;
        }

        auto inserted = this->_raw.emplace(key, value);

        if (chaining && inserted.second) {
            wrapper_ptr wrapper = std::make_shared<observer_wrapper>();
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

    void _insert(std::map<Key, Value> &&map, chaining_f chaining) {
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

    void _erase_observer_for_key(Key const &key) {
        if (this->_observers.count(key) > 0) {
            auto &wrapper = this->_observers.at(key);
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
            this->_observers.erase(key);
        }
    }
};

#pragma mark - map::holder

template <typename Key, typename Value>
holder<Key, Value>::holder(std::map<Key, Value> map) : sender<event>(std::make_shared<impl>()) {
    this->template impl_ptr<impl>()->prepare(std::move(map));
}

template <typename Key, typename Value>
holder<Key, Value>::holder(std::shared_ptr<impl> &&ptr) : sender<event>(std::move(ptr)) {
}

template <typename Key, typename Value>
holder<Key, Value>::~holder() = default;

template <typename Key, typename Value>
std::map<Key, Value> const &holder<Key, Value>::raw() const {
    return this->template impl_ptr<impl>()->raw();
}

template <typename Key, typename Value>
std::map<Key, Value> &holder<Key, Value>::raw() {
    return this->template impl_ptr<impl>()->raw();
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
    return this->template impl_ptr<impl>()->raw().at(key);
}

template <typename Key, typename Value>
std::size_t holder<Key, Value>::size() const {
    return this->raw().size();
}

template <typename Key, typename Value>
void holder<Key, Value>::replace_all(std::map<Key, Value> map) {
    this->template impl_ptr<impl>()->replace_all(std::move(map));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert_or_replace(Key key, Value value) {
    this->template impl_ptr<impl>()->insert_or_replace(std::move(key), std::move(value));
}

template <typename Key, typename Value>
void holder<Key, Value>::insert(std::map<Key, Value> map) {
    this->template impl_ptr<impl>()->insert(std::move(map));
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_if(std::function<bool(Key const &, Value const &)> const &handler) {
    return this->template impl_ptr<impl>()->erase_if(handler);
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_for_value(Value const &value) {
    return this->erase_if([&value](Key const &, Value const &map_value) { return map_value == value; });
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_for_key(Key const &key) {
    return this->template impl_ptr<impl>()->erase_for_key(key);
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
chaining::receivable_ptr<event> holder<Key, Value>::receivable() {
    return this->template impl_ptr<typename chaining::receivable<event>>();
}

template <typename Key, typename Value>
std::shared_ptr<weakable_impl> holder<Key, Value>::weakable_impl_ptr() const {
    return this->template impl_ptr<impl>();
}

template <typename Key, typename Value>
std::shared_ptr<holder<Key, Value>> holder<Key, Value>::make_shared() {
    return make_shared(std::map<Key, Value>{});
}

template <typename Key, typename Value>
std::shared_ptr<holder<Key, Value>> holder<Key, Value>::make_shared(std::map<Key, Value> map) {
    return std::shared_ptr<holder<Key, Value>>(new holder<Key, Value>{std::move(map)});
}
}  // namespace yas::chaining::map
