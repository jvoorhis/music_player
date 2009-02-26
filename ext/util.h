#include <ruby.h>
#include <CoreFoundation/CoreFoundation.h>

// Tests whether a VALUE is a primitive number type.
#define PRIM_NUM_P(num) (T_FIXNUM == TYPE(num) || \
                          T_FLOAT == TYPE(num) || \
                         T_BIGNUM == TYPE(num))

#define CSTR2SYM(str) (ID2SYM(rb_intern(str)))

#define PATH2CFURL(path) (rb_path_to_cfurl(path))

static CFURLRef
rb_path_to_cfurl (VALUE rb_path)
{
    VALUE rb_abs_path = rb_file_expand_path(rb_path, Qnil);
    char *path = StringValueCStr(rb_abs_path);
    CFURLRef url = CFURLCreateFromFileSystemRepresentation(NULL, (const UInt8 *) path, strlen(path), false);
    return url;
}
