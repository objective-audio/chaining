//
//  yas_chaining_fetcher_protocol.h
//

#pragma once

#include "yas_protocol.h"

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
struct chain;

template <typename T>
struct fetchable : protocol {
    struct impl {
        virtual std::optional<T> fetched_value() = 0;
    };

    fetchable(std::shared_ptr<impl>);
    fetchable(std::nullptr_t);

    std::optional<T> fetched_value();
};
}  // namespace yas::chaining
