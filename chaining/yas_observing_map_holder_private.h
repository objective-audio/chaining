//
//  yas_observing_map_holder_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>

namespace yas::observing::map {
template <typename Key, typename Element>
holder<Key, Element>::holder(std::map<Key, Element> const &value) : _raw(value) {
}

template <typename Key, typename Element>
holder<Key, Element>::holder(std::map<Key, Element> &&value) : _raw(std::move(value)) {
}

template <typename Key, typename Element>
std::map<Key, Element> const &holder<Key, Element>::elements() const {
    return this->_raw;
}

template <typename Key, typename Element>
Element const &holder<Key, Element>::at(Key const &key) const {
    return this->_raw.at(key);
}

template <typename Key, typename Element>
bool holder<Key, Element>::contains(Key const &key) const {
    return this->_raw.count(key) > 0;
}

template <typename Key, typename Element>
std::size_t holder<Key, Element>::size() const {
    return this->_raw.size();
}

template <typename Key, typename Element>
void holder<Key, Element>::replace(std::map<Key, Element> const &map) {
    this->_raw = map;
    this->_call_any();
}

template <typename Key, typename Element>
void holder<Key, Element>::replace(std::map<Key, Element> &&map) {
    this->_raw = std::move(map);
    this->_call_any();
}

template <typename Key, typename Element>
void holder<Key, Element>::insert_or_replace(Key const &key, Element const &element) {
    bool replaced = false;

    if (this->_raw.count(key) > 0) {
        this->_raw.erase(key);
        replaced = true;
    }

    this->_raw.emplace(key, element);

    if (replaced) {
        this->_call_replaced(&key);
    } else {
        this->_call_inserted(&key);
    }
}

template <typename Key, typename Element>
std::map<Key, Element> holder<Key, Element>::erase(Key const &key) {
    std::map<Key, Element> erased;

    if (this->_raw.count(key) > 0) {
        erased.emplace(key, std::move(this->_raw.at(key)));
        this->_raw.erase(key);
        this->_call_erased(&erased.at(key), &key);
    }

    return erased;
}

template <typename Key, typename Element>
void holder<Key, Element>::clear() {
    if (this->_raw.size() > 0) {
        this->_raw.clear();
        this->_call_any();
    }
}

template <typename Key, typename Element>
canceller_ptr holder<Key, Element>::observe(typename caller<event>::handler_f &&handler, bool const sync) {
    if (sync) {
        handler(event{.type = event_type::any, .elements = this->_raw});
    }
    return this->_caller.add(std::move(handler));
}

template <typename Key, typename Element>
holder_ptr<Key, Element> holder<Key, Element>::make_shared() {
    return make_shared({});
}

template <typename Key, typename Element>
holder_ptr<Key, Element> holder<Key, Element>::make_shared(std::map<Key, Element> &&map) {
    return holder_ptr<Key, Element>(new holder<Key, Element>{std::move(map)});
}

template <typename Key, typename Element>
holder_ptr<Key, Element> holder<Key, Element>::make_shared(std::map<Key, Element> const &map) {
    return holder_ptr<Key, Element>(new holder<Key, Element>{map});
}

template <typename Key, typename Element>
void holder<Key, Element>::_call_any() {
    this->_caller.call(event{.type = event_type::any, .elements = this->_raw});
}

template <typename Key, typename Element>
void holder<Key, Element>::_call_replaced(Key const *key) {
    this->_caller.call(
        event{.type = event_type::replaced, .elements = this->_raw, .element = &this->_raw.at(*key), .key = key});
}

template <typename Key, typename Element>
void holder<Key, Element>::_call_inserted(Key const *key) {
    this->_caller.call(
        event{.type = event_type::inserted, .elements = this->_raw, .element = &this->_raw.at(*key), .key = key});
}

template <typename Key, typename Element>
void holder<Key, Element>::_call_erased(Element const *element, Key const *key) {
    this->_caller.call(event{.type = event_type::erased, .elements = this->_raw, .element = element, .key = key});
}
}  // namespace yas::observing::map
