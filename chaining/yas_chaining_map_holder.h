//
//  yas_chaining_map_holder.h
//

#pragma once

#include <map>
#include "yas_chaining_any_observer.h"
#include "yas_chaining_event.h"
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::map {
struct event : chaining::event {
    template <typename Event>
    event(Event &&event) : chaining::event(std::move(event)) {
    }
};

template <typename Key, typename Value>
struct fetched_event {
    static event_type const type = event_type::fetched;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct any_event {
    static event_type const type = event_type::any;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct inserted_event {
    static event_type const type = event_type::inserted;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct erased_event {
    static event_type const type = event_type::erased;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct replaced_event {
    static event_type const type = event_type::replaced;
    Key const &key;
    Value const &value;
};

template <typename Key, typename Value>
struct relayed_event {
    static event_type const type = event_type::relayed;
    Key const &key;
    Value const &value;
    typename Value::element_type::SendType const &relayed;
};

template <typename Key, typename Value>
struct holder final : sender<event>, receiver<event> {
    using chain_t = chain<event, event, true>;

    ~holder();

    [[nodiscard]] std::map<Key, Value> const &raw() const;
    [[nodiscard]] std::map<Key, Value> &raw();

    [[nodiscard]] bool has_value(Key const &) const;
    [[nodiscard]] Value const &at(Key const &) const;
    [[nodiscard]] Value &at(Key const &);
    [[nodiscard]] std::size_t size() const;

    void replace_all(std::map<Key, Value>);
    void insert_or_replace(Key, Value);
    void insert(std::map<Key, Value>);
    std::map<Key, Value> erase_if(std::function<bool(Key const &, Value const &)> const &);
    std::map<Key, Value> erase_for_value(Value const &);
    std::map<Key, Value> erase_for_key(Key const &);
    void clear();

    [[nodiscard]] chain_t chain();

    void receive_value(event const &) override;

   private:
    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
        Value *value = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(Key const &, Value &, wrapper_ptr &)>;

    std::map<Key, Value> _raw;
    std::map<Key, wrapper_ptr> _observers;

    holder();

    holder(holder const &) = delete;
    holder(holder &&) = delete;
    holder &operator=(holder const &) = delete;
    holder &operator=(holder &&) = delete;

    void fetch_for(any_joint const &joint) const override;

    void _prepare(std::map<Key, Value> &&);

    chaining_f _element_chaining();
    void _replace(std::map<Key, Value> &&map, chaining_f chaining);
    void _erase_observer_for_key(Key const &key);
    void _insert_or_replace(Key &&key, Value &&value, chaining_f chaining);
    void _insert(std::map<Key, Value> &&map, chaining_f chaining);

    template <typename SenderValue, enable_if_base_of_sender_t<SenderValue, std::nullptr_t> = nullptr>
    void _replace_all(std::map<Key, std::shared_ptr<SenderValue>> map) {
        this->_replace(std::move(map), this->_element_chaining());
    }

    template <typename NonSenderValue>
    void _replace_all(std::map<Key, NonSenderValue> map) {
        this->_replace(std::move(map), nullptr);
    }

    template <typename SenderValue, enable_if_base_of_sender_t<SenderValue, std::nullptr_t> = nullptr>
    void _insert_or_replace(Key key, std::shared_ptr<SenderValue> value) {
        this->_insert_or_replace(std::move(key), std::move(value), this->_element_chaining());
    }

    template <typename NonSenderValue>
    void _insert_or_replace(Key key, NonSenderValue value) {
        this->_insert_or_replace(std::move(key), std::move(value), nullptr);
    }

    template <typename SenderValue, enable_if_base_of_sender_t<SenderValue, std::nullptr_t> = nullptr>
    void _insert(std::map<Key, std::shared_ptr<SenderValue>> map) {
        this->_insert(std::move(map), this->_element_chaining());
    }

    template <typename NonSenderValue>
    void _insert(std::map<Key, NonSenderValue> map) {
        this->_insert(std::move(map), nullptr);
    }

   public:
    static std::shared_ptr<holder> make_shared();
    static std::shared_ptr<holder> make_shared(std::map<Key, Value>);
};
}  // namespace yas::chaining::map

#include "yas_chaining_map_holder_private.h"
