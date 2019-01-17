//
//  yas_chaining_event_private.h
//

#pragma once

namespace yas::chaining {
template <typename Event>
event::event(Event &&event) : base(std::make_shared<impl<Event>>(std::move(event))) {
}

template <typename Event>
Event const &event::get() const {
    if (auto event_impl = std::dynamic_pointer_cast<impl<Event>>(impl_ptr())) {
        return event_impl->event;
    }

    throw std::runtime_error("get event failed.");
}

template <typename Event>
struct event::impl : event::impl_base {
    Event const event;

    impl(Event &&event) : event(std::move(event)) {
    }

    event_type type() override {
        return Event::type;
    }
};
}  // namespace yas::chaining
