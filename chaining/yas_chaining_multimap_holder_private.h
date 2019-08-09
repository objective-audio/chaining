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

    void fetch_for(any_joint const &joint) override {
        this->send_value_to_target(make_fetched_event(this->_raw), joint.identifier());
    }
};

namespace utils {
    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    typename multimap::holder<Key, Value>::impl::chaining_f _element_chaining(holder<Key, Value> &holder) {
        auto impl_ptr = holder.template impl_ptr<typename multimap::holder<Key, Value>::impl>();
        auto weak_holder_impl = to_weak(impl_ptr);
        return [weak_holder_impl](Key const &key, Value &value,
                                  typename multimap::holder<Key, Value>::impl::wrapper_ptr &wrapper) {
            typename multimap::holder<Key, Value>::impl::wrapper_wptr weak_wrapper = wrapper;
            auto weak_value = to_weak(value);
            wrapper->observer = value.sendable()
                                    ->chain_unsync()
                                    .perform([weak_holder_impl, weak_wrapper, key, weak_value](auto const &relayed) {
                                        auto holder_impl = weak_holder_impl.lock();
                                        auto value = weak_value.lock();
                                        typename multimap::holder<Key, Value>::impl::wrapper_ptr wrapper =
                                            weak_wrapper.lock();
                                        if (holder_impl && wrapper && value) {
                                            holder_impl->broadcast(make_relayed_event(key, *value, relayed));
                                        }
                                    })
                                    .end();
        };
    }

    template <typename Key, typename Value>
    void _replace(holder<Key, Value> &holder, std::multimap<Key, Value> &&map,
                  typename multimap::holder<Key, Value>::impl::chaining_f chaining) {
        auto impl_ptr = holder.template impl_ptr<typename multimap::holder<Key, Value>::impl>();

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

        impl_ptr->broadcast(make_any_event(impl_ptr->_raw));
    }

    template <typename Key, typename Value>
    void _insert(holder<Key, Value> &holder, std::multimap<Key, Value> &&map,
                 typename multimap::holder<Key, Value>::impl::chaining_f chaining) {
        auto impl_ptr = holder.template impl_ptr<typename multimap::holder<Key, Value>::impl>();

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

        impl_ptr->broadcast(make_inserted_event(map));
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void replace(holder<Key, Value> &holder, std::multimap<Key, Value> &&map) {
        utils::_replace(holder, std::move(map), utils::_element_chaining(holder));
    }

    template <typename Key, typename Value, disable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void replace(holder<Key, Value> &holder, std::multimap<Key, Value> &&map) {
        utils::_replace(holder, std::move(map), nullptr);
    }

    template <typename Key, typename Value, enable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert(holder<Key, Value> &holder, std::multimap<Key, Value> &&map) {
        utils::_insert(holder, std::move(map), utils::_element_chaining(holder));
    }

    template <typename Key, typename Value, disable_if_base_of_sender_t<Value, std::nullptr_t> = nullptr>
    void insert(holder<Key, Value> &holder, std::multimap<Key, Value> &&map) {
        utils::_insert(holder, std::move(map), nullptr);
    }
}  // namespace utils

#pragma mark - multimap::holder

template <typename Key, typename Value>
holder<Key, Value>::holder(std::multimap<Key, Value> map) : sender<event>(std::make_shared<impl>()) {
    this->_prepare(std::move(map));
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
    auto impl_ptr = this->template impl_ptr<impl>();

    std::multimap<Key, Value> erased;

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
std::multimap<Key, Value> holder<Key, Value>::erase_for_value(Value const &value) {
    return this->erase_if([&value](Key const &, Value const &map_value) { return map_value == value; });
}

template <typename Key, typename Value>
std::multimap<Key, Value> holder<Key, Value>::erase_for_key(Key const &key) {
    return this->erase_if([&key](Key const &map_key, Value const &) { return map_key == key; });
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
void holder<Key, Value>::_prepare(std::multimap<Key, Value> &&map) {
    auto impl_ptr = this->template impl_ptr<impl>();
    utils::replace(*this, std::move(map));
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
