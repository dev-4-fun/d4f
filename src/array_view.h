#ifndef D4F_ARRAY_VIEW_H
#define D4F_ARRAY_VIEW_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#ifndef D4F_ARRAY_VIEW_IMPLEMENTATION
#define D4FDEF extern
#else
#define D4FDEF
#endif

#define D4F_ARRAY_VIEW_STATUS_LIST(X)                                          \
  X(D4F_ARRAY_VIEW_OK, "OK")                                                   \
  X(D4F_ARRAY_VIEW_BAD_ARGS, "Bad arguments")                                  \
  X(D4F_ARRAY_VIEW_OUT_OF_BOUNDS, "Out of bounds")                             \
  X(D4F_ARRAY_VIEW_TYPE_MISMATCH, "Type mismatch")

typedef enum {
#define X(id, msg) id,
  D4F_ARRAY_VIEW_STATUS_LIST(X)
#undef X
      _D4F_ARRAY_VIEW_ERROR_COUNT
} D4F_ARRAY_VIEW_STATUS;

typedef int (*array_foreach_callback_t)(void *item, size_t i, void *ctx);

typedef struct {
  void *data_ptr;
  size_t length;
  size_t item_size;
  size_t capacity;
} array_view_t;

#define array_view_create(array_view, buffer)                                  \
  array_view_init((array_view), (buffer), sizeof(*(buffer)), 0,                \
                  sizeof((buffer)) / sizeof(*(buffer)))
#define array_view_wrap(array_view, items)                                     \
  array_view_init((array_view), (items), sizeof(*(items)),                     \
                  sizeof((items)) / sizeof(*(items)),                          \
                  sizeof((items)) / sizeof(*(items)))
#define array_view_push(array_view, item)                                      \
  array_view_push_n((array_view), (item), 1)
#define array_view_length(array_view) (array_view) ? (array_view)->length : 0
#define array_view_capacity(array_view)                                        \
  (array_view) ? (array_view)->capacity : 0
#define array_view_item_size(array_view)                                       \
  (array_view) ? (array_view)->item_size : 0

D4FDEF D4F_ARRAY_VIEW_STATUS array_view_init(array_view_t *array_view,
                                             void *items, size_t item_size,
                                             size_t length, size_t capacity);
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_copy(const array_view_t *src,
                                             array_view_t *dest);
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_slice(const array_view_t *array_view,
                                              int start, int end,
                                              array_view_t *slice);
D4FDEF D4F_ARRAY_VIEW_STATUS
array_view_validate(const array_view_t *array_view);

D4FDEF void *array_view_at(const array_view_t *array_view, size_t i);
D4FDEF const void *array_view_at_const(const array_view_t *array_view,
                                       size_t i);

D4FDEF D4F_ARRAY_VIEW_STATUS array_view_push_n(array_view_t *array_view,
                                               const void *items, size_t n);
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_pop(array_view_t *array_view,
                                            void **item);

D4FDEF D4F_ARRAY_VIEW_STATUS array_view_foreach(const array_view_t *array_view,
                                                array_foreach_callback_t cb,
                                                void *ctx);

D4FDEF const char *
array_view_status_message(const D4F_ARRAY_VIEW_STATUS status);

#ifdef __cplusplus
}
#endif

#endif /* D4F_ARRAY_VIEW_H */
