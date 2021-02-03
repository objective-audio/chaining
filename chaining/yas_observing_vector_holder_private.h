//
//  yas_observing_vector_holder_private.h
//

#pragma once

#include <cpp_utils/yas_stl_utils.h>

namespace yas::observing::vector {
template <typename T>
holder<T>::holder(std::vector<T> const &value) : _raw(value) {
}

template <typename T>
holder<T>::holder(std::vector<T> &&value) : _raw(std::move(value)) {
}

template <typename T>
std::vector<T> const &holder<T>::value() const {
    return this->_raw;
}

template <typename T>
T const &holder<T>::at(std::size_t const idx) const {
    return this->_raw.at(idx);
}

template <typename T>
std::size_t holder<T>::size() const {
    return this->_raw.size();
}

template <typename T>
void holder<T>::replace(std::vector<T> const &value) {
    this->_raw = value;
    this->_call_any();
}

template <typename T>
void holder<T>::replace(std::vector<T> &&value) {
    this->_raw = std::move(value);
    this->_call_any();
}

template <typename T>
void holder<T>::replace(T const &element, std::size_t const idx) {
    this->_raw.at(idx) = element;
    this->_call_replaced(idx);
}

template <typename T>
void holder<T>::replace(T &&element, std::size_t const idx) {
    this->_raw.at(idx) = std::move(element);
    this->_call_replaced(idx);
}

template <typename T>
void holder<T>::push_back(T const &element) {
    this->_raw.emplace_back(element);
    this->_call_inserted(this->_raw.size() - 1);
}

template <typename T>
void holder<T>::push_back(T &&element) {
    this->_raw.emplace_back(std::move(element));
    this->_call_inserted(this->_raw.size() - 1);
}

template <typename T>
void holder<T>::insert(T const &element, std::size_t const idx) {
    this->_raw.insert(this->_raw.begin() + idx, element);
    this->_call_inserted(idx);
}

template <typename T>
void holder<T>::insert(T &&element, std::size_t const idx) {
    this->_raw.insert(this->_raw.begin() + idx, std::move(element));
    this->_call_inserted(idx);
}

template <typename T>
T holder<T>::erase(std::size_t const idx) {
    T erased = this->_raw.at(idx);
    yas::erase_at(this->_raw, idx);
    this->_call_erased(&erased, idx);
    return erased;
}

template <typename T>
void holder<T>::clear() {
    if (this->_raw.size() > 0) {
        this->_raw.clear();
        this->_call_any();
    }
}

template <typename T>
canceller_ptr holder<T>::observe(typename caller<event>::handler_f &&handler, bool const sync) {
    if (sync) {
        handler(event{.type = event_type::any, .elements = this->_raw});
    }
    return this->_caller.add(std::move(handler));
}

template <typename T>
void holder<T>::_call_any() {
    this->_caller.call(event{.type = event_type::any, .elements = this->_raw});
}

template <typename T>
void holder<T>::_call_replaced(std::size_t const idx) {
    this->_caller.call(
        event{.type = event_type::replaced, .elements = this->_raw, .element = &this->_raw.at(idx), .index = idx});
}

template <typename T>
void holder<T>::_call_inserted(std::size_t const idx) {
    this->_caller.call(
        event{.type = event_type::inserted, .elements = this->_raw, .element = &this->_raw.at(idx), .index = idx});
}

template <typename T>
void holder<T>::_call_erased(T const *element, std::size_t const idx) {
    this->_caller.call(event{.type = event_type::erased, .elements = this->_raw, .element = element, .index = idx});
}

template <typename T>
holder_ptr<T> holder<T>::make_shared() {
    return holder_ptr<T>(new holder<T>{{}});
}

template <typename T>
holder_ptr<T> holder<T>::make_shared(std::vector<T> &&value) {
    return holder_ptr<T>(new holder<T>{std::move(value)});
}

template <typename T>
holder_ptr<T> holder<T>::make_shared(std::vector<T> const &value) {
    return holder_ptr<T>(new holder<T>{value});
}
}  // namespace yas::observing::vector
