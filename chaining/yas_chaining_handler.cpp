//
//  yas_chaining_handler.cpp
//

#include "yas_chaining_handler.h"

using namespace yas;

chaining::any_handler::any_handler(handler_impl_base_ptr const &impl) : _impl_base(impl) {
}
