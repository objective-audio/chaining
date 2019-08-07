//
//  yas_chaining_map_holder.h
//

#pragma once

#include <cpp_utils/yas_weakable.h>
#include <map>
#include "yas_chaining_event.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::multimap {
struct event : chaining::event {
    template <typename Event>
    event(Event &&event) : chaining::event(std::move(event)) {
    }
};

template <typename Key, typename Value>
struct fetched_event {
    static event_type const type = event_type::fetched;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct any_event {
    static event_type const type = event_type::any;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct inserted_event {
    static event_type const type = event_type::inserted;
    std::multimap<Key, Value> const &elements;
};

template <typename Key, typename Value>
struct erased_event {
    static event_type const type = event_type::erased;
    std::multimap<Key, Value> const &elements;
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
struct holder final : sender<event>, weakable<holder<Key, Value>> {
    class impl;

    using chain_t = chain<event, event, true>;

    explicit holder(std::shared_ptr<impl> &&);

    ~holder();

    [[nodiscard]] std::multimap<Key, Value> const &raw() const;
    [[nodiscard]] std::multimap<Key, Value> &raw();

    [[nodiscard]] std::size_t size() const;
    void replace(std::multimap<Key, Value>);
    void insert(std::multimap<Key, Value>);
    void insert(Key, Value);
    std::multimap<Key, Value> erase_if(std::function<bool(Key const &, Value const &)> const &);
    std::multimap<Key, Value> erase_for_value(Value const &);
    std::multimap<Key, Value> erase_for_key(Key const &);
    void clear();

    [[nodiscard]] chain_t chain() const;

    std::shared_ptr<weakable_impl> weakable_impl_ptr() const override;

   private:
    explicit holder(std::multimap<Key, Value>);

    bool is_equal(sender<event> const &rhs) const override;

   public:
    static std::shared_ptr<holder> make_shared();
    static std::shared_ptr<holder> make_shared(std::multimap<Key, Value>);
};
}  // namespace yas::chaining::multimap

#include "yas_chaining_multimap_holder_private.h"
