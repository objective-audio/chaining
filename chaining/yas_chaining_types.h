//
//  yas_chaining_types.h
//

#pragma once

namespace yas::chaining {
template <typename Out, typename In, typename Begin, bool Syncable>
class chain;

template <typename T>
using chain_sync_t = chain<T, T, T, true>;
template <typename T, typename U>
using chain_relayed_sync_t = chain<T, U, U, true>;
template <typename T, typename U>
using chain_normalized_sync_t = chain<T, T, U, true>;
template <typename T>
using chain_unsync_t = chain<T, T, T, false>;
template <typename T, typename U>
using chain_relayed_unsync_t = chain<T, U, U, false>;
template <typename T, typename U>
using chain_normalized_unsync_t = chain<T, T, U, false>;
}  // namespace yas::chaining
