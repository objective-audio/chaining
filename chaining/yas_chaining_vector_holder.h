//
//  yas_chaining_array_holder.h
//

#pragma once

#include <vector>
#include "yas_chaining_any_observer.h"
#include "yas_chaining_event.h"
#include "yas_chaining_receiver.h"
#include "yas_chaining_sender.h"

namespace yas::chaining::vector {
struct event : chaining::event {
    template <typename Event>
    event(Event &&event) : chaining::event(std::move(event)) {
    }
};

template <typename T>
struct fetched_event {
    static event_type const type = event_type::fetched;
    std::vector<T> const &elements;
};

template <typename T>
struct any_event {
    static event_type const type = event_type::any;
    std::vector<T> const &elements;
};

template <typename T>
struct inserted_event {
    static event_type const type = event_type::inserted;
    T const &element;
    std::size_t const index;
};

template <typename T>
struct erased_event {
    static event_type const type = event_type::erased;
    std::size_t const index;
};

template <typename T>
struct replaced_event {
    static event_type const type = event_type::replaced;
    T const &element;
    std::size_t const index;
};

template <typename T>
struct relayed_event {
    static event_type const type = event_type::relayed;
    T const &element;
    std::size_t const index;
    typename T::element_type::SendType const &relayed;
};

template <typename T>
struct holder final : sender<event>, receiver<event> {
    using vector_t = std::vector<T>;
    using chain_t = chain<event, event, true>;

    [[nodiscard]] vector_t const &raw() const;
    [[nodiscard]] vector_t &raw();
    [[nodiscard]] T const &at(std::size_t const) const;
    [[nodiscard]] std::size_t size() const;

    void replace(vector_t);
    void replace(T, std::size_t const);
    void push_back(T);
    void insert(T, std::size_t const);
    T erase_at(std::size_t const);
    void clear();

    [[nodiscard]] chain_t chain();

    void receive_value(event const &) override;

   private:
    struct observer_wrapper {
        any_observer_ptr observer = nullptr;
    };

    using wrapper_ptr = std::shared_ptr<observer_wrapper>;
    using wrapper_wptr = std::weak_ptr<observer_wrapper>;
    using chaining_f = std::function<void(T &, wrapper_ptr &)>;

    std::vector<T> _raw;
    std::vector<wrapper_ptr> _observers;

    holder();

    holder(holder const &) = delete;
    holder(holder &&) = delete;
    holder &operator=(holder const &) = delete;
    holder &operator=(holder &&) = delete;

    void fetch_for(any_joint const &joint) override;

    void _prepare(std::vector<T> &&);

    chaining_f _element_chaining();
    void _replace(std::vector<T> &&vec, chaining_f chaining);
    void _replace(T &&element, std::size_t const idx, chaining_f chaining);
    void _insert(T &&element, std::size_t const idx, chaining_f chaining);

    template <typename U, enable_if_base_of_sender_t<U, std::nullptr_t> = nullptr>
    void replace_all(std::vector<std::shared_ptr<U>> vec) {
        this->_replace(std::move(vec), this->_element_chaining());
    }

    template <typename U>
    void replace_all(std::vector<U> vec) {
        this->_replace(std::move(vec), nullptr);
    }

    template <typename U, enable_if_base_of_sender_t<U, std::nullptr_t> = nullptr>
    void replace(vector::holder<std::shared_ptr<U>> &holder, std::shared_ptr<U> element, std::size_t const idx) {
        this->_replace(std::move(element), idx, this->_element_chaining());
    }

    template <typename U>
    void replace(vector::holder<U> &holder, U element, std::size_t const idx) {
        this->_replace(std::move(element), idx, nullptr);
    }

    template <typename U, enable_if_base_of_sender_t<U, std::nullptr_t> = nullptr>
    void insert(vector::holder<std::shared_ptr<U>> &holder, std::shared_ptr<U> element, std::size_t const idx) {
        this->_insert(std::move(element), idx, this->_element_chaining());
    }

    template <typename U>
    void insert(vector::holder<U> &holder, U element, std::size_t const idx) {
        this->_insert(std::move(element), idx, nullptr);
    }

   public:
    static std::shared_ptr<holder> make_shared();
    static std::shared_ptr<holder> make_shared(vector_t);
};
}  // namespace yas::chaining::vector

#include "yas_chaining_vector_holder_private.h"
