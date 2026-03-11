#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define D4F_ARRAY_IMPLEMENTATION
#include "array.h"

#define D4F_ARRAY_INITIAL_CAP 64
#define D4F_ARRAY_SUCCESS 1
#define D4F_ARRAY_FAIL 0

#define D4F_ARRAY_PTR_VALIDATE(ptr)                                            \
  do {                                                                         \
    if (ptr == NULL) {                                                         \
      D4F_ARRAY_ERROR("" #ptr " is NULL");                                     \
      return D4F_ARRAY_FAIL;                                                   \
    }                                                                          \
  } while (0);

#define D4F_ARRAY_ERROR(message) __error((message), __FILE__, __LINE__)

static void __error(const char *message, const char *file, int line) {
  fprintf(stderr, "[ERROR]%s:%d: %s\n", (file), (line), (message));
}

static int __array_maybe_grow_data(array_t *array, size_t item_size,
                                   size_t capacity) {
  size_t array_capacity = array->__capacity;
  if (capacity == 0 || item_size == 0) {
    return D4F_ARRAY_SUCCESS;
  }
  while (capacity > array_capacity) {
    array_capacity =
        array_capacity == 0 ? D4F_ARRAY_INITIAL_CAP : array_capacity * 2;
  }
  if (array_capacity != array->__capacity) {
    void *data_ptr = NULL;
    data_ptr = realloc(array->data_ptr, item_size * array_capacity);
    D4F_ARRAY_PTR_VALIDATE(data_ptr);
    array->data_ptr = data_ptr;
    array->__item_size = item_size;
    array->__capacity = array_capacity;
  }
  return D4F_ARRAY_SUCCESS;
}

int array_init(array_t *array, size_t item_size, size_t capacity) {
  D4F_ARRAY_PTR_VALIDATE(array);
  array_reset(array);
  if (!__array_maybe_grow_data(array, item_size, capacity)) {
    return D4F_ARRAY_FAIL;
  }
  array->length = 0;
  return D4F_ARRAY_SUCCESS;
}

int array_from(array_t *array, void *data, size_t item_size, size_t length) {
  D4F_ARRAY_PTR_VALIDATE(array);
  if (data == NULL || length == 0 || item_size == 0) {
    return array_init(array, item_size, length);
  }
  array_reset(array);
  if (!__array_maybe_grow_data(array, item_size, length)) {
    return D4F_ARRAY_FAIL;
  }
  memcpy(array->data_ptr, data, array->__item_size * length);
  array->length = length;
  return D4F_ARRAY_SUCCESS;
}

int array_copy(array_t *src, array_t *dest) {
  D4F_ARRAY_PTR_VALIDATE(src);
  D4F_ARRAY_PTR_VALIDATE(dest);
  array_reset(dest);
  if (!__array_maybe_grow_data(dest, src->__item_size, src->length)) {
    return D4F_ARRAY_FAIL;
  }
  memcpy(dest->data_ptr, src->data_ptr, src->__item_size * src->length);
  dest->length = src->length;
  return D4F_ARRAY_SUCCESS;
}

int array_slice(array_t *array, int start, int end, array_t *slice) {
  size_t start_idx = 0;
  size_t end_idx = 0;
  size_t slice_length = 0;
  D4F_ARRAY_PTR_VALIDATE(array);
  D4F_ARRAY_PTR_VALIDATE(slice);
  start_idx = (start < 0) ? array->length + start : start;
  end_idx = ((end < 0) ? array->length + end : end) - 1;
  if (start_idx > end_idx || start_idx >= array->length ||
      end_idx >= array->length) {
    D4F_ARRAY_ERROR("Slice bad boundary");
    return D4F_ARRAY_FAIL;
  }
  slice_length = end_idx - start_idx + 1;
  array_reset(slice);
  if (!__array_maybe_grow_data(slice, array->__item_size, slice_length)) {
    return D4F_ARRAY_FAIL;
  }
  memcpy(slice->data_ptr,
         (char *)array->data_ptr + (start_idx * array->__item_size),
         array->__item_size * slice_length);
  slice->length = slice_length;
  return D4F_ARRAY_SUCCESS;
}

void array_reset(array_t *array) {
  if (array == NULL) {
    return;
  }
  array->__item_size = 0;
  array->__capacity = 0;
  if (array->data_ptr) {
    free(array->data_ptr);
    array->data_ptr = NULL;
  }
  array->length = 0;
}

void *array_at(array_t *array, size_t i) {
  D4F_ARRAY_PTR_VALIDATE(array);
  if (i >= array->length) {
    D4F_ARRAY_ERROR("Index is out of range");
    return NULL;
  }
  return (char *)array->data_ptr + array->__item_size * i;
}

int array_push_n(array_t *array, void *items, size_t n) {
  D4F_ARRAY_PTR_VALIDATE(array);
  D4F_ARRAY_PTR_VALIDATE(items);
  if (n == 0) {
    return D4F_ARRAY_SUCCESS;
  }
  if (!__array_maybe_grow_data(array, array->__item_size, array->length + n)) {
    return D4F_ARRAY_FAIL;
  }
  memcpy((char *)array->data_ptr + (array->__item_size * array->length), items,
         (array->__item_size * n));
  array->length += n;
  return D4F_ARRAY_SUCCESS;
}

int array_push(array_t *array, void *item) {
  return array_push_n(array, item, 1);
}

int array_pop(array_t *array, void **out_item) {
  D4F_ARRAY_PTR_VALIDATE(array);
  D4F_ARRAY_PTR_VALIDATE(out_item);
  if (array->length == 0) {
    return D4F_ARRAY_FAIL;
  }
  memcpy(*out_item, array_at(array, array->length - 1), array->__item_size);
  array->length--;
  return D4F_ARRAY_SUCCESS;
}

int array_foreach(array_t *array, array_foreach_callback_t cb, void *ctx) {
  size_t i;
  D4F_ARRAY_PTR_VALIDATE(array);
  for (i = 0; i < array->length; i++) {
    if (cb(array_at(array, i), i, ctx) != 0)
      break;
  }
  return D4F_ARRAY_SUCCESS;
}
