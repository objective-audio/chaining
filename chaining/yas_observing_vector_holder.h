//
//  yas_observing_vector_holder.h
//

#pragma once

#include <chaining/yas_observing_caller.h>

#include <vector>

namespace yas::observing::vector {
template <typename T>
class holder;

template <typename T>
using holder_ptr = std::shared_ptr<holder<T>>;

template <typename T>
struct holder final {
    using vector_t = std::vector<T>;

    enum class event_type {
        any,
        replaced,
        inserted,
        erased,
    };

    struct event {
        event_type type;
        std::vector<T> const &elements;
        T const *element = nullptr;
        std::optional<std::size_t> index = std::nullopt;
    };

    [[nodiscard]] vector_t const &value() const;
    [[nodiscard]] T const &at(std::size_t const) const;
    [[nodiscard]] std::size_t size() const;

    void replace(vector_t const &);
    void replace(vector_t &&);
    void replace(T const &, std::size_t const);
    void replace(T &&, std::size_t const);
    void push_back(T const &);
    void push_back(T &&);
    void insert(T const &, std::size_t const);
    void insert(T &&, std::size_t const);
    T erase(std::size_t const);
    void clear();

    [[nodiscard]] canceller_ptr observe(typename caller<event>::handler_f &&, bool const sync);

    [[nodiscard]] static holder_ptr<T> make_shared();
    [[nodiscard]] static holder_ptr<T> make_shared(vector_t &&);
    [[nodiscard]] static holder_ptr<T> make_shared(vector_t const &);

   private:
    std::vector<T> _raw;
    caller<event> _caller;

    holder(vector_t const &);
    holder(vector_t &&);

    void _call_any();
    void _call_replaced(std::size_t const idx);
    void _call_inserted(std::size_t const idx);
    void _call_erased(T const *, std::size_t const idx);
};
}  // namespace yas::observing::vector

#include <chaining/yas_observing_vector_holder_private.h>
