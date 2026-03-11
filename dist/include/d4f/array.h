#ifndef D4F_ARRAY_H
#define D4F_ARRAY_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef D4F_ARRAY_IMPLEMENTATION
#define D4FDEF extern
#else
#define D4FDEF
#endif

typedef struct {
  void *data_ptr;
  size_t length;
  size_t __item_size;
  size_t __capacity;
} array_t;

typedef int (*array_foreach_callback_t)(void *item, size_t i, void *ctx);

D4FDEF int array_init(array_t *array, size_t item_size, size_t capacity);
D4FDEF int array_from(array_t *array, void *items, size_t item_size,
                      size_t length);
D4FDEF int array_copy(array_t *src, array_t *dest);
D4FDEF int array_slice(array_t *array, int start, int end, array_t *slice);
D4FDEF void array_reset(array_t *array);
D4FDEF void *array_at(array_t *array, size_t i);
D4FDEF int array_push_n(array_t *array, void *items, size_t n);
D4FDEF int array_push(array_t *array, void *item);
D4FDEF int array_pop(array_t *array, void **out_item);
D4FDEF int array_foreach(array_t *array, array_foreach_callback_t cb,
                         void *ctx);

#ifdef __cplusplus
}
#endif

#endif /* D4F_ARRAY_H */
