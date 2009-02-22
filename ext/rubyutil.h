#include <ruby.h>

// Tests whether a VALUE is a primitive number type.
#define PRIM_NUM_P(num) (T_FIXNUM == TYPE(num) || \
                          T_FLOAT == TYPE(num) || \
                         T_BIGNUM == TYPE(num))

#define CSTR2SYM(str) (ID2SYM(rb_intern(str)))
