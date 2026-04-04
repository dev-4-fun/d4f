#ifndef D4F_ARRAY_VIEW_H
#define D4F_ARRAY_VIEW_H

#include <stddef.h>

/*
 * Non-owning view of contiguous memory: no allocation or free. length is the
 * active count; capacity is the size of the backing buffer in elements.
 * Most routines return D4F_ARRAY_VIEW_*; array_view_at returns NULL on error.
 */

#ifdef __cplusplus
extern "C" {
#endif

#ifndef D4F_ARRAY_VIEW_IMPLEMENTATION
#define D4FDEF extern
#else
#define D4FDEF
#endif

/* Expand X(id, msg) for each status; ids become enum D4F_ARRAY_VIEW##id. */
#define D4F_ARRAY_VIEW_STATUS_LIST(X)                                          \
  X(_OK, "OK")                                                                 \
  X(_BAD_ARGS, "Bad arguments")                                                \
  X(_OUT_OF_BOUNDS, "Out of bounds")                                           \
  X(_TYPE_MISMATCH, "Type mismatch")

typedef enum {
#define X(id, msg) D4F_ARRAY_VIEW##id,
  D4F_ARRAY_VIEW_STATUS_LIST(X)
#undef X
      _D4F_ARRAY_VIEW_ERROR_COUNT
} D4F_ARRAY_VIEW_STATUS;

/* Return 0 to continue foreach; any other value stops iteration. */
typedef int (*array_foreach_callback_t)(void *item, size_t i, void *ctx);

typedef struct {
  void *data_ptr;
  size_t length;
  size_t item_size;
  size_t capacity;
} array_view_t;

/* Fixed-size C array helpers and accessors (null-safe where noted). */
#define array_view_create(array_view, buffer)                                  \
  array_view_init((array_view), (buffer), sizeof(*(buffer)), 0,                \
                  sizeof((buffer)) / sizeof(*(buffer)))
#define array_view_wrap(array_view, items)                                     \
  array_view_init((array_view), (items), sizeof(*(items)),                     \
                  sizeof((items)) / sizeof(*(items)),                          \
                  sizeof((items)) / sizeof(*(items)))
#define array_view_push(array_view, item)                                      \
  array_view_push_n((array_view), (item), 1)
#define array_view_insert_at(array_view, index, item)                          \
  array_view_insert_n_at((array_view), (index), (item), 1)
#define array_view_remove_at(array_view, index)                                \
  array_view_remove_n_at((array_view), (index), 1)
#define array_view_length(array_view) (array_view) ? (array_view)->length : 0
#define array_view_capacity(array_view)                                        \
  (array_view) ? (array_view)->capacity : 0
#define array_view_item_size(array_view)                                       \
  (array_view) ? (array_view)->item_size : 0

/* Point view at items; length must not exceed capacity. */
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_init(array_view_t *array_view,
                                             void *items, size_t item_size,
                                             size_t length, size_t capacity);
/* Copy min(src->length, dest->capacity) elements; dest item_size must match. */
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_copy(const array_view_t *src,
                                             array_view_t *dest);
/* Non-owning subview; Python-style indices, end exclusive. */
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_slice(const array_view_t *array_view,
                                              int start, int end,
                                              array_view_t *slice);
/* NULL or invalid fields -> D4F_ARRAY_VIEW_BAD_ARGS. */
D4FDEF D4F_ARRAY_VIEW_STATUS
array_view_validate(const array_view_t *array_view);

/* Item i within length, else NULL. */
D4FDEF void *array_view_at(const array_view_t *array_view, size_t i);
D4FDEF const void *array_view_at_const(const array_view_t *array_view,
                                       size_t i);

/* Append n elements; fails if length + n > capacity. */
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_push_n(array_view_t *array_view,
                                               const void *items, size_t n);
/* Decrement length; *item is last element address or NULL if empty. */
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_pop(array_view_t *array_view,
                                            void **item);

/* Insert n elements at index (index == length acts as push). */
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_insert_n_at(array_view_t *array_view,
                                                    size_t index,
                                                    const void *items,
                                                    size_t n);
/* Remove n elements at index; shifts tail down. */
D4FDEF D4F_ARRAY_VIEW_STATUS array_view_remove_n_at(array_view_t *array_view,
                                                    size_t index, size_t n);

D4FDEF D4F_ARRAY_VIEW_STATUS array_view_foreach(const array_view_t *array_view,
                                                array_foreach_callback_t cb,
                                                void *ctx);

D4FDEF const char *
array_view_status_message(const D4F_ARRAY_VIEW_STATUS status);

#ifdef __cplusplus
}
#endif

#endif /* D4F_ARRAY_VIEW_H */
