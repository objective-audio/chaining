//
//  yas_chaining_sender_protocol.h
//

#pragma once

#include <cpp_utils/yas_protocol.h>
#include "yas_chaining_types.h"

namespace yas::chaining {
template <typename T>
struct sendable {
    virtual ~sendable() = default;

    virtual void broadcast(T const &) = 0;
    virtual void send_value_to_target(T const &, std::uintptr_t const) = 0;
    virtual void erase_joint(std::uintptr_t const) = 0;
    virtual void fetch_for(any_joint const &) = 0;
    virtual chain_unsync_t<T> chain_unsync() = 0;
    virtual chain_sync_t<T> chain_sync() = 0;
};
}  // namespace yas::chaining
