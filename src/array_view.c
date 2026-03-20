#include <assert.h>
#include <string.h>

#define D4F_ARRAY_VIEW_IMPLEMENTATION
#include "array_view.h"

static const char *const _status_messages[] = {
#define X(id, msg) msg,
    D4F_ARRAY_VIEW_STATUS_LIST(X)
#undef X
};

D4F_ARRAY_VIEW_STATUS array_view_init(array_view_t *array_view, void *items,
                                      size_t item_size, size_t length,
                                      size_t capacity) {
  if (array_view == NULL || items == NULL || capacity == 0 || item_size == 0 ||
      length > capacity) {
    return D4F_ARRAY_VIEW_BAD_ARGS;
  }
  array_view->data_ptr = items;
  array_view->length = length;
  array_view->item_size = item_size;
  array_view->capacity = capacity;
  return D4F_ARRAY_VIEW_OK;
}

D4F_ARRAY_VIEW_STATUS array_view_copy(const array_view_t *src,
                                      array_view_t *dest) {
  size_t copy_length = 0;
  D4F_ARRAY_VIEW_STATUS status = D4F_ARRAY_VIEW_OK;
  status = array_view_validate(src);
  if (status != D4F_ARRAY_VIEW_OK) {
    return status;
  }
  status = array_view_validate(dest);
  if (status != D4F_ARRAY_VIEW_OK) {
    return status;
  }
  if (src->item_size != dest->item_size) {
    return D4F_ARRAY_VIEW_TYPE_MISMATCH;
  }
  copy_length = src->length > dest->capacity ? dest->capacity : src->length;
  memcpy(dest->data_ptr, src->data_ptr, copy_length * src->item_size);
  dest->length = copy_length;
  return status;
}

D4F_ARRAY_VIEW_STATUS array_view_slice(const array_view_t *array_view,
                                       int start, int end,
                                       array_view_t *slice) {
  size_t start_idx = 0;
  size_t end_idx = 0;
  size_t slice_length = 0;
  D4F_ARRAY_VIEW_STATUS status = D4F_ARRAY_VIEW_OK;
  status = array_view_validate(array_view);
  if (status != D4F_ARRAY_VIEW_OK) {
    return status;
  }
  if (slice == NULL) {
    return D4F_ARRAY_VIEW_BAD_ARGS;
  }
  start_idx = (start < 0) ? array_view->length + start : start;
  end_idx = ((end <= 0) ? array_view->length + end : end) - 1;
  if (start_idx > end_idx || start_idx >= array_view->length ||
      end_idx >= array_view->length) {
    return D4F_ARRAY_VIEW_OUT_OF_BOUNDS;
  }
  slice_length = end_idx - start_idx + 1;
  status = array_view_init(
      slice, (char *)array_view->data_ptr + start_idx * array_view->item_size,
      array_view->item_size, slice_length, slice_length);
  return status;
}

D4F_ARRAY_VIEW_STATUS array_view_validate(const array_view_t *array_view) {
  if (array_view == NULL || array_view->data_ptr == NULL ||
      array_view->item_size == 0 || array_view->capacity == 0 ||
      array_view->length > array_view->capacity) {
    return D4F_ARRAY_VIEW_BAD_ARGS;
  }
  return D4F_ARRAY_VIEW_OK;
}

static void *array_view_at_with_boundary(const array_view_t *array_view,
                                         size_t i, size_t boundary) {
  if (i >= boundary) {
    return NULL;
  }
  return (char *)array_view->data_ptr + array_view->item_size * i;
}

void *array_view_at(const array_view_t *array_view, size_t i) {
  D4F_ARRAY_VIEW_STATUS status = D4F_ARRAY_VIEW_OK;
  status = array_view_validate(array_view);
  if (status != D4F_ARRAY_VIEW_OK) {
    return NULL;
  }
  return (char *)array_view_at_with_boundary(array_view, i, array_view->length);
}

const void *array_view_at_const(const array_view_t *array_view, size_t i) {
  return (const void *)array_view_at(array_view, i);
}

D4F_ARRAY_VIEW_STATUS array_view_push_n(array_view_t *array_view,
                                        const void *items, size_t n) {
  D4F_ARRAY_VIEW_STATUS status = D4F_ARRAY_VIEW_OK;
  status = array_view_validate(array_view);
  if (status != D4F_ARRAY_VIEW_OK) {
    return status;
  }
  if (items == NULL) {
    return D4F_ARRAY_VIEW_BAD_ARGS;
  }
  if (array_view->length + n > array_view->capacity) {
    return D4F_ARRAY_VIEW_OUT_OF_BOUNDS;
  }
  memcpy(array_view_at_with_boundary(array_view, array_view->length,
                                     array_view->capacity),
         items, n * array_view->item_size);
  array_view->length += n;
  return D4F_ARRAY_VIEW_OK;
}

D4F_ARRAY_VIEW_STATUS array_view_pop(array_view_t *array_view, void **item) {
  D4F_ARRAY_VIEW_STATUS status = D4F_ARRAY_VIEW_OK;
  status = array_view_validate(array_view);
  if (status != D4F_ARRAY_VIEW_OK) {
    return status;
  }
  if (item == NULL) {
    return D4F_ARRAY_VIEW_BAD_ARGS;
  }
  if (array_view->length == 0) {
    *item = NULL;
    return D4F_ARRAY_VIEW_OUT_OF_BOUNDS;
  }
  *item = array_view_at(array_view, --array_view->length);
  return D4F_ARRAY_VIEW_OK;
}

D4F_ARRAY_VIEW_STATUS array_view_foreach(const array_view_t *array_view,
                                         array_foreach_callback_t cb,
                                         void *ctx) {
  size_t i;
  D4F_ARRAY_VIEW_STATUS status = D4F_ARRAY_VIEW_OK;
  status = array_view_validate(array_view);
  if (status != D4F_ARRAY_VIEW_OK) {
    return status;
  }
  if (cb == NULL) {
    return D4F_ARRAY_VIEW_BAD_ARGS;
  }
  for (i = 0; i < array_view->length; i++) {
    if (cb(array_view_at(array_view, i), i, ctx) != 0)
      break;
  }
  return D4F_ARRAY_VIEW_OK;
}

const char *array_view_status_message(const D4F_ARRAY_VIEW_STATUS status) {
  if (status >= _D4F_ARRAY_VIEW_ERROR_COUNT) {
    return "UNKNOWN";
  }
  return _status_messages[status];
}
