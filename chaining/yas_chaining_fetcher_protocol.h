//
//  yas_chaining_fetcher_protocol.h
//

#pragma once

#include "yas_protocol.h"
#include "yas_types.h"

namespace yas::chaining {
template <typename T>
struct fetchable : protocol {
    struct impl {
        virtual opt_t<T> fetched_value() = 0;
    };

    fetchable(std::shared_ptr<impl>);
    fetchable(std::nullptr_t);

    opt_t<T> fetched_value();
};
}  // namespace yas::chaining
