//
//  yas_chaining_map_holder.h
//

#pragma once

#include <map>
#include "yas_chaining_event.h"
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::map {
struct event : chaining::event {
    template <typename Event>
    event(Event &&event) : chaining::event(std::move(event)) {
    }

    event(std::nullptr_t) : chaining::event(nullptr) {
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
    typename Value::SendType const &relayed;
};

template <typename Key, typename Value>
struct holder : sender<event>, receiver<event> {
    class impl;

    using chain_t = chain<event, event, true>;

    holder();
    explicit holder(std::map<Key, Value>);
    holder(std::nullptr_t);

    ~holder() final;

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

    [[nodiscard]] chain_t chain() const;

    [[nodiscard]] chaining::receivable<event> receivable() override;

   protected:
    explicit holder(std::shared_ptr<impl> &&);
};
}  // namespace yas::chaining::map

#include "yas_chaining_map_holder_private.h"
