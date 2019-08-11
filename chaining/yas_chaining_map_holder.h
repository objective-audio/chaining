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
    class impl;

    using chain_t = chain<event, event, true>;

    explicit holder(std::shared_ptr<impl> &&);

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

    std::shared_ptr<impl> _impl;

   private:
    holder();

    holder(holder const &) = delete;
    holder(holder &&) = delete;
    holder &operator=(holder const &) = delete;
    holder &operator=(holder &&) = delete;

    bool is_equal(sender<event> const &rhs) const override;
    void fetch_for(any_joint const &joint) override;

    void _prepare(std::map<Key, Value> &&);

   public:
    static std::shared_ptr<holder> make_shared();
    static std::shared_ptr<holder> make_shared(std::map<Key, Value>);
};
}  // namespace yas::chaining::map

#include "yas_chaining_map_holder_private.h"
