//
//  yas_observing_map_holder.h
//

#pragma once

#include <chaining/yas_observing_caller.h>

#include <map>

namespace yas::observing::map {
template <typename Key, typename Element>
class holder;

template <typename Key, typename Element>
using holder_ptr = std::shared_ptr<holder<Key, Element>>;

enum class event_type {
    any,
    replaced,
    inserted,
    erased,
};

template <typename Key, typename Element>
struct holder final {
    struct event {
        event_type type;
        std::map<Key, Element> const &elements;
        Element const *element = nullptr;
        Key const *key = nullptr;
    };

    [[nodiscard]] std::map<Key, Element> const &elements() const;
    [[nodiscard]] Element const &at(Key const &) const;
    [[nodiscard]] bool contains(Key const &) const;
    [[nodiscard]] std::size_t size() const;

    void replace(std::map<Key, Element> const &);
    void replace(std::map<Key, Element> &&);
    void insert_or_replace(Key const &, Element const &);
    std::map<Key, Element> erase(Key const &);
    void clear();

    [[nodiscard]] canceller_ptr observe(typename caller<event>::handler_f &&, bool const sync);

    [[nodiscard]] static holder_ptr<Key, Element> make_shared();
    [[nodiscard]] static holder_ptr<Key, Element> make_shared(std::map<Key, Element> &&);
    [[nodiscard]] static holder_ptr<Key, Element> make_shared(std::map<Key, Element> const &);

   private:
    std::map<Key, Element> _raw;
    caller<event> _caller;

    holder(std::map<Key, Element> const &);
    holder(std::map<Key, Element> &&);

    void _call_any();
    void _call_replaced(Key const *);
    void _call_inserted(Key const *);
    void _call_erased(Element const *, Key const *);
};
}  // namespace yas::observing::map

#include <chaining/yas_observing_map_holder_private.h>
