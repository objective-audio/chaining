//
//  yas_chaining_map_holder.h
//

#pragma once

#include <map>
#include "yas_chaining_sender.h"

namespace yas::chaining::map {
enum event_type {
    fetched,
    any,
    inserted,
    erased,
    replaced,
    relayed,
};

template <typename Key, typename Value>
struct event {
    event_type const type;
    std::map<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct immutable_holder : sender<event<Key, Value>> {
    class impl;

    immutable_holder(std::nullptr_t);

    using event_t = map::event<Key, Value>;
    using chain_t = chain<event_t, event_t, event_t, true>;

    std::map<Key, Value> const &raw() const;
    bool has_value(Key const &) const;
    Value const &at(Key const &) const;
    std::size_t size() const;

    chain_t chain();

   protected:
    immutable_holder(std::shared_ptr<impl> &&);
};

template <typename Key, typename Value>
struct holder : immutable_holder<Key, Value> {
    holder();
    explicit holder(std::map<Key, Value>);
    holder(std::nullptr_t);

    ~holder() final;

    std::map<Key, Value> &raw();

    void replace_all(std::map<Key, Value>);
    void insert_or_replace(Key, Value);
    void insert(std::map<Key, Value>);
    std::map<Key, Value> erase_if(std::function<bool(Key const &, Value const &)> const &);
    std::map<Key, Value> erase_for_value(Value const &);
    std::map<Key, Value> erase_for_key(Key const &);
    void clear();

   private:
    using immutable_impl = typename immutable_holder<Key, Value>::impl;
};
}  // namespace yas::chaining::map

#include "yas_chaining_map_holder_private.h"
