#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "d4f/array_view.h"
#define D4F_ARRAY_IMPLEMENTATION
#include "d4f/array.h"

static const char *const _array_status_messages[] = {
#define X(id, msg) msg,
    D4F_ARRAY_VIEW_STATUS_LIST(X) D4F_ARRAY_STATUS_LIST(X)
#undef X
};

static D4F_ARRAY_STATUS _array_maybe_grow_data(array_t *array, size_t item_size,
                                               size_t new_length,
                                               size_t new_capacity) {
  D4F_ARRAY_STATUS status = D4F_ARRAY_OK;
  array_view_t *view = (array_view_t *)array;
  size_t capacity = array_capacity(array);
  if (new_capacity == 0 || item_size == 0) {
    return D4F_ARRAY_BAD_ARGS;
  }
  while (new_capacity > capacity) {
    capacity = capacity == 0 ? D4F_ARRAY_INITIAL_CAP : capacity * 2;
  }
  if (capacity != array_capacity(array)) {
    void *data_ptr = array_dataptr(array);
    data_ptr = realloc(data_ptr, item_size * capacity);
    if (data_ptr == NULL) {
      return D4F_ARRAY_ALLOC_NULL;
    }
    status = (D4F_ARRAY_STATUS)array_view_init(view, data_ptr, item_size,
                                               new_length, capacity);
  }
  return status;
}

D4F_ARRAY_STATUS array_init(array_t *array, size_t item_size, size_t capacity) {
  D4F_ARRAY_STATUS status = D4F_ARRAY_OK;
  if (array == NULL) {
    return D4F_ARRAY_BAD_ARGS;
  }
  status = _array_maybe_grow_data(array, item_size, 0, capacity);
  if (status != D4F_ARRAY_OK) {
    return status;
  }
  return D4F_ARRAY_OK;
}

D4F_ARRAY_STATUS array_from(array_t *array, void *data, size_t item_size,
                            size_t length) {
  D4F_ARRAY_STATUS status = D4F_ARRAY_OK;
  if (array == NULL) {
    return D4F_ARRAY_BAD_ARGS;
  }
  status = _array_maybe_grow_data(array, item_size, length, length);
  if (status != D4F_ARRAY_OK) {
    return status;
  }
  memcpy(array_dataptr(array), data, item_size * length);
  return D4F_ARRAY_OK;
}

D4F_ARRAY_STATUS array_copy(array_t *src, array_t *dest) {
  array_view_t *src_view = (array_view_t *)src;
  return array_from(dest, src_view->data_ptr, src_view->item_size,
                    src_view->length);
}

D4F_ARRAY_STATUS array_slice(array_t *array, int start, int end,
                             array_t *slice) {
  D4F_ARRAY_STATUS status = D4F_ARRAY_OK;
  array_view_t *view = (array_view_t *)array;
  array_view_t tmp = {0};
  status = (D4F_ARRAY_STATUS)array_view_slice(view, start, end, &tmp);
  if (status != D4F_ARRAY_OK) {
    return status;
  }
  status = (D4F_ARRAY_STATUS)array_from(slice, tmp.data_ptr, tmp.item_size,
                                        tmp.length);
  return status;
}

void array_free(array_t *array) {
  array_view_t *view = (array_view_t *)array;
  if (array == NULL) {
    return;
  }
  view->item_size = 0;
  view->capacity = 0;
  if (view->data_ptr) {
    free(view->data_ptr);
    view->data_ptr = NULL;
  }
  view->length = 0;
}

D4F_ARRAY_STATUS array_push_n(array_t *array, void *item, size_t n) {
  D4F_ARRAY_STATUS status = D4F_ARRAY_OK;
  array_view_t *view = (array_view_t *)array;
  size_t new_capacity = 0;
  status = (D4F_ARRAY_STATUS)array_view_validate((array_view_t *)array);
  if (status != D4F_ARRAY_OK) {
    return status;
  }
  if (n == 0) {
    return D4F_ARRAY_OK;
  }
  new_capacity =
      (view->length + n > view->capacity) ? view->length + n : view->capacity;
  status = _array_maybe_grow_data(array, view->item_size, view->length,
                                  new_capacity);
  if (status != D4F_ARRAY_OK) {
    return status;
  }
  return (D4F_ARRAY_STATUS)array_view_push_n(view, item, n);
}

D4F_ARRAY_STATUS array_insert_n_at(array_t *array, size_t index, void *items,
                                   size_t n) {
  D4F_ARRAY_STATUS status = D4F_ARRAY_OK;
  array_view_t *view = (array_view_t *)array;
  size_t new_capacity = 0;
  status = (D4F_ARRAY_STATUS)array_view_validate((array_view_t *)array);
  if (status != D4F_ARRAY_OK) {
    return status;
  }
  if (n == 0) {
    return D4F_ARRAY_OK;
  }
  new_capacity =
      (view->length + n > view->capacity) ? view->length + n : view->capacity;
  status = _array_maybe_grow_data(array, view->item_size, view->length,
                                  new_capacity);
  if (status != D4F_ARRAY_OK) {
    return status;
  }
  return (D4F_ARRAY_STATUS)array_view_insert_n_at(view, index, items, n);
}

const char *array_status_message(const D4F_ARRAY_STATUS status) {
  if (status >= _D4F_ARRAY_ERROR_COUNT) {
    return "UNKNOWN";
  }
  return _array_status_messages[status];
}
