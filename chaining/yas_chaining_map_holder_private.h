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
struct holder<Key, Value>::impl : sender<event>::impl, weakable_impl {
    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
        Value *value = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(Key const &, Value &, wrapper_ptr &)>;

    std::map<Key, Value> _raw;
    std::map<Key, wrapper_ptr> _observers;

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(make_fetched_event(this->_raw), joint.identifier());
    }
};

namespace utils {
    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    typename holder<Key, Value>::impl::chaining_f element_chaining(holder<Key, Value> &holder) {
        auto impl_ptr = holder.template impl_ptr<typename map::holder<Key, Value>::impl>();
        auto weak_holder_impl = to_weak(impl_ptr);
        return [weak_holder_impl](Key const &key, Value &value,
                                  typename map::holder<Key, Value>::impl::wrapper_ptr &wrapper) {
            auto weak_value = to_weak(value);
            typename map::holder<Key, Value>::impl::wrapper_wptr weak_wrapper = wrapper;
            wrapper->observer = value.sendable()
                                    ->chain_unsync()
                                    .perform([weak_holder_impl, weak_wrapper, key, weak_value](auto const &relayed) {
                                        auto holder_impl = weak_holder_impl.lock();
                                        auto value = weak_value.lock();
                                        typename map::holder<Key, Value>::impl::wrapper_ptr wrapper =
                                            weak_wrapper.lock();
                                        if (holder_impl && wrapper && value) {
                                            holder_impl->broadcast(make_relayed_event(key, *value, relayed));
                                        }
                                    })
                                    .end();
        };
    }

    template <typename Key, typename Value>
    void _replace(holder<Key, Value> &holder, std::map<Key, Value> &&map,
                  typename map::holder<Key, Value>::impl::chaining_f chaining) {
        auto impl_ptr = holder.template impl_ptr<typename map::holder<Key, Value>::impl>();

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

        impl_ptr->broadcast(make_any_event(impl_ptr->_raw));
    }

    template <typename Key, typename Value>
    void _erase_observer_for_key(holder<Key, Value> &holder, Key const &key) {
        auto impl_ptr = holder.template impl_ptr<typename map::holder<Key, Value>::impl>();

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
        auto impl_ptr = holder.template impl_ptr<typename map::holder<Key, Value>::impl>();

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
            impl_ptr->broadcast(make_replaced_event(key, value));
        } else {
            std::map<Key, Value> map{{std::move(key), std::move(value)}};
            impl_ptr->broadcast(make_inserted_event(map));
        }
    }

    template <typename Key, typename Value>
    void _insert(holder<Key, Value> &holder, std::map<Key, Value> &&map,
                 typename map::holder<Key, Value>::impl::chaining_f chaining) {
        auto impl_ptr = holder.template impl_ptr<typename map::holder<Key, Value>::impl>();

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

        impl_ptr->broadcast(make_inserted_event(map));
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void replace_all(holder<Key, Value> &holder, std::map<Key, Value> map) {
        utils::_replace(holder, std::move(map), utils::element_chaining(holder));
    }

    template <typename Key, typename Value, disable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void replace_all(holder<Key, Value> &holder, std::map<Key, Value> map) {
        utils::_replace(holder, std::move(map), nullptr);
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert_or_replace(holder<Key, Value> &holder, Key key, Value value) {
        auto impl_ptr = holder.template impl_ptr<typename map::holder<Key, Value>::impl>();
        utils::_insert_or_replace(holder, std::move(key), std::move(value), utils::element_chaining(holder));
    }

    template <typename Key, typename Value, disable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert_or_replace(holder<Key, Value> &holder, Key key, Value value) {
        utils::_insert_or_replace(holder, std::move(key), std::move(value), nullptr);
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert(holder<Key, Value> &holder, std::map<Key, Value> map) {
        auto impl_ptr = holder.template impl_ptr<typename map::holder<Key, Value>::impl>();
        utils::_insert(holder, std::move(map), utils::element_chaining(holder));
    }

    template <typename Key, typename Value, disable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert(holder<Key, Value> &holder, std::map<Key, Value> map) {
        utils::_insert(holder, std::move(map), nullptr);
    }
}  // namespace utils

#pragma mark - map::holder

template <typename Key, typename Value>
holder<Key, Value>::holder(std::map<Key, Value> map) : sender<event>(std::make_shared<impl>()) {
    this->_prepare(std::move(map));
}

template <typename Key, typename Value>
holder<Key, Value>::holder(std::shared_ptr<impl> &&ptr) : sender<event>(std::move(ptr)) {
}

template <typename Key, typename Value>
holder<Key, Value>::~holder() = default;

template <typename Key, typename Value>
std::map<Key, Value> const &holder<Key, Value>::raw() const {
    return this->template impl_ptr<impl>()->_raw;
}

template <typename Key, typename Value>
std::map<Key, Value> &holder<Key, Value>::raw() {
    return this->template impl_ptr<impl>()->_raw;
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
    auto impl_ptr = this->template impl_ptr<impl>();

    std::map<Key, Value> erased;

    if (impl_ptr->_observers.size() > 0) {
        yas::erase_if(impl_ptr->_observers, [&handler](std::pair<Key, typename impl::wrapper_ptr> const &pair) {
            return handler(pair.first, *pair.second->value);
        });
    }

    yas::erase_if(impl_ptr->_raw, [&handler, &erased](std::pair<Key, Value> const &pair) {
        if (handler(pair.first, pair.second)) {
            erased.insert(pair);
            return true;
        } else {
            return false;
        }
    });

    impl_ptr->broadcast(make_erased_event(erased));

    return erased;
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_for_value(Value const &value) {
    return this->erase_if([&value](Key const &, Value const &map_value) { return map_value == value; });
}

template <typename Key, typename Value>
std::map<Key, Value> holder<Key, Value>::erase_for_key(Key const &key) {
    auto impl_ptr = this->template impl_ptr<impl>();

    utils::_erase_observer_for_key(*this, key);

    std::map<Key, Value> erased;

    if (impl_ptr->_raw.count(key)) {
        erased.emplace(key, std::move(impl_ptr->_raw.at(key)));
        impl_ptr->_raw.erase(key);
    }

    impl_ptr->broadcast(make_erased_event(erased));

    return erased;
}

template <typename Key, typename Value>
void holder<Key, Value>::clear() {
    auto impl_ptr = this->template impl_ptr<impl>();

    for (auto &pair : impl_ptr->_observers) {
        if (auto &wrapper = pair.second) {
            if (any_observer_ptr &observer = wrapper->observer) {
                observer->invalidate();
            }
        }
    }

    impl_ptr->_observers.clear();
    impl_ptr->_raw.clear();

    impl_ptr->broadcast(make_any_event(impl_ptr->_raw));
}

template <typename Key, typename Value>
typename holder<Key, Value>::chain_t holder<Key, Value>::holder<Key, Value>::chain() const {
    return this->template impl_ptr<impl>()->chain_sync();
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
void holder<Key, Value>::_prepare(std::map<Key, Value> &&map) {
    utils::replace_all(*this, std::move(map));
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
