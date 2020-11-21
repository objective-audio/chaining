//
//  yas_chaining_types.h
//

#pragma once

#include <optional>

namespace yas::chaining {
template <typename Out, typename Begin, bool Syncable>
class chain;

template <typename T>
using chain_sync_t = chain<T, T, true>;
template <typename T, typename U>
using chain_relayed_sync_t = chain<T, U, true>;
template <typename T>
using chain_unsync_t = chain<T, T, false>;
template <typename T, typename U>
using chain_relayed_unsync_t = chain<T, U, false>;

template <typename T, typename U>
using opt_pair_t = std::pair<std::optional<T>, std::optional<U>>;

class any_joint;
template <typename T>
using joint_handler_f = std::function<void(T const &, any_joint &)>;
}  // namespace yas::chaining
