#ifndef D4F_ARRAY_H
#define D4F_ARRAY_H

#include <stddef.h>

#include "d4f/array_view.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifndef D4FDEF
#ifndef D4F_ARRAY_IMPLEMENTATION
#define D4FDEF extern
#else
#define D4FDEF
#endif
#endif

#ifndef D4F_ARRAY_INITIAL_CAP
#define D4F_ARRAY_INITIAL_CAP 64
#endif

#define D4F_ARRAY_STATUS_LIST(X) X(_ALLOC_NULL, "Allocation failure")

typedef enum {
#define X(id, msg) D4F_ARRAY##id,
  D4F_ARRAY_VIEW_STATUS_LIST(X) D4F_ARRAY_STATUS_LIST(X)
#undef X
      _D4F_ARRAY_ERROR_COUNT
} D4F_ARRAY_STATUS;

typedef array_view_t array_t;

#define array_capacity(array) (array_view_capacity((array_view_t *)(array)))
#define array_dataptr(array) (((array_view_t *)(array))->data_ptr)
#define array_length(array) (array_view_length((array_view_t *)(array)))
#define array_item_size(array) (array_view_item_size((array_view_t *)(array)))
#define array_push(array, item) array_push_n((array), (item), 1)
#define array_insert_at(array, index, item)                                    \
  array_insert_n_at((array), (index), (item), 1)
#define array_remove_n_at(array, index, n)                                     \
  ((D4F_ARRAY_STATUS)array_view_remove_n_at((array_view_t *)(array), (index),  \
                                            (n)))
#define array_remove_at(array, index)                                          \
  ((D4F_ARRAY_STATUS)array_view_remove_at((array_view_t *)(array), (index)))
#define array_foreach(array, cb, ctx)                                          \
  ((D4F_ARRAY_STATUS)array_view_foreach((array_view_t *)(array), (cb), (ctx)))
#define array_at(array, i) array_view_at((array_view_t *)(array), (i))
#define array_at_const(array, i)                                               \
  array_view_at_const((array_view_t *)(array), (i))
#define array_pop(array, out_item)                                             \
  ((D4F_ARRAY_STATUS)array_view_pop((array_view_t *)(array), (out_item)))
#define array_reset(array)                                                     \
  ((array) ? (void)(((array_view_t *)(array))->length = 0) : (void)0)

D4FDEF D4F_ARRAY_STATUS array_init(array_t *array, size_t item_size,
                                   size_t capacity);
D4FDEF D4F_ARRAY_STATUS array_from(array_t *array, void *data, size_t item_size,
                                   size_t length);
D4FDEF D4F_ARRAY_STATUS array_copy(array_t *src, array_t *dest);
D4FDEF D4F_ARRAY_STATUS array_slice(array_t *array, int start, int end,
                                    array_t *slice);
D4FDEF void array_free(array_t *array);
D4FDEF D4F_ARRAY_STATUS array_push_n(array_t *array, void *item, size_t n);
D4FDEF D4F_ARRAY_STATUS array_insert_n_at(array_t *array, size_t index,
                                          void *items, size_t n);
D4FDEF const char *array_status_message(D4F_ARRAY_STATUS status);

#ifdef __cplusplus
}
#endif

#endif /* D4F_ARRAY_H */
