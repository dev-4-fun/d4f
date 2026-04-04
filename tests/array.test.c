#include "d4f/array.h"
#include <assert.h>
#include <string.h>

#define D4F_ARRAY_IMPLEMENTATION
#include "d4f/array_ho.h"

static int cb_sum(void *item, size_t i, void *ctx) {
  int *acc = (int *)ctx;
  (void)i;
  *acc += *(int *)item;
  return 0;
}

static void test_array_init(void) {
  array_t a = {0};
  assert(array_init(NULL, sizeof(int), 4) == D4F_ARRAY_BAD_ARGS);
  assert(array_init(&a, 0, 4) == D4F_ARRAY_BAD_ARGS);
  assert(array_init(&a, sizeof(int), 0) == D4F_ARRAY_BAD_ARGS);
  assert(array_init(&a, sizeof(int), 4) == D4F_ARRAY_OK);
  assert(a.length == 0);
  assert(a.capacity >= 4);
  assert(a.item_size == sizeof(int));
  array_free(&a);
}

static void test_array_from(void) {
  array_t a = {0};
  int buf[3] = {1, 2, 3};
  assert(array_from(NULL, buf, sizeof(int), 3) == D4F_ARRAY_BAD_ARGS);
  assert(array_from(&a, buf, 0, 3) == D4F_ARRAY_BAD_ARGS);
  assert(array_from(&a, buf, sizeof(int), 3) == D4F_ARRAY_OK);
  assert(array_dataptr(&a) != (void *)buf);
  assert(array_length(&a) == 3);
  assert(*(int *)array_at(&a, 1) == 2);
  array_free(&a);
  memset(&a, 0, sizeof(a));
}

static void test_array_free(void) {
  array_t a = {0};
  array_free(NULL);
  assert(array_init(&a, sizeof(int), 2) == D4F_ARRAY_OK);
  array_free(&a);
  assert(a.data_ptr == NULL);
  assert(a.length == 0);
  assert(a.capacity == 0);
}

static void test_array_push_n(void) {
  array_t a = {0};
  int v = 7;
  assert(array_init(&a, sizeof(int), 2) == D4F_ARRAY_OK);
  assert(array_push_n(NULL, &v, 1) == D4F_ARRAY_BAD_ARGS);
  assert(array_push_n(&a, &v, 0) == D4F_ARRAY_OK);
  assert(array_push_n(&a, &v, 1) == D4F_ARRAY_OK);
  assert(array_length(&a) == 1);
  assert(*(int *)array_at(&a, 0) == 7);
  assert(array_push_n(&a, &v, 1) == D4F_ARRAY_OK);
  assert(array_length(&a) == 2);
  array_free(&a);
}

static void test_array_push(void) {
  array_t a = {0};
  int v = 42;
  assert(array_init(&a, sizeof(int), 2) == D4F_ARRAY_OK);
  assert(array_push(&a, &v) == D4F_ARRAY_OK);
  assert(array_length(&a) == 1);
  assert(*(int *)array_at(&a, 0) == 42);
  array_free(&a);
}

static void test_array_insert_n_at(void) {
  array_t a = {0};
  int x[2] = {10, 20};
  assert(array_init(&a, sizeof(int), 8) == D4F_ARRAY_OK);
  assert(array_insert_n_at(NULL, 0, x, 1) == D4F_ARRAY_BAD_ARGS);
  assert(array_insert_n_at(&a, 0, x, 2) == D4F_ARRAY_OK);
  assert(array_length(&a) == 2);
  assert(array_insert_n_at(&a, 1, x, 2) == D4F_ARRAY_OK);
  assert(array_length(&a) == 4);
  assert(*(int *)array_at(&a, 1) == 10);
  array_free(&a);
}

static void test_array_insert_at(void) {
  array_t a = {0};
  int v = 5;
  assert(array_init(&a, sizeof(int), 4) == D4F_ARRAY_OK);
  assert(array_insert_at(&a, 0, &v) == D4F_ARRAY_OK);
  assert(array_length(&a) == 1);
  assert(*(int *)array_at(&a, 0) == 5);
  array_free(&a);
}

static void test_array_copy(void) {
  array_t src = {0};
  array_t dest = {0};
  int buf_src[2] = {3, 4};
  int buf_dest[2] = {0, 0};
  assert(array_from(&src, buf_src, sizeof(int), 2) == D4F_ARRAY_OK);
  assert(array_from(&dest, buf_dest, sizeof(int), 2) == D4F_ARRAY_OK);
  assert(array_copy(&src, &dest) == D4F_ARRAY_OK);
  assert(array_dataptr(&dest) != (void *)buf_src);
  assert(array_dataptr(&dest) != (void *)buf_dest);
  assert(array_length(&dest) == 2);
  assert(*(int *)array_at(&dest, 0) == 3);
  assert(*(int *)array_at(&dest, 1) == 4);
  array_free(&src);
  array_free(&dest);
}

static void test_array_slice(void) {
  array_t a = {0};
  array_t s = {0};
  int buf[5] = {0, 1, 2, 3, 4};
  assert(array_from(&a, buf, sizeof(int), 5) == D4F_ARRAY_OK);
  /* inclusive range [1,3]: API uses end such that end-1 is last index */
  assert(array_slice(&a, 1, 4, &s) == D4F_ARRAY_OK);
  assert(array_length(&s) == 3);
  assert(*(int *)array_at(&s, 0) == 1);
  assert(array_slice(&a, 10, 11, &s) == D4F_ARRAY_OUT_OF_BOUNDS);
  array_free(&a);
  array_free(&s);
}

static void test_array_at(void) {
  array_t a = {0};
  int buf[2] = {9, 8};
  assert(array_from(&a, buf, sizeof(int), 2) == D4F_ARRAY_OK);
  assert(array_at(&a, 0) != NULL);
  assert(*(int *)array_at(&a, 1) == 8);
  assert(array_at(&a, 2) == NULL);
  array_free(&a);
}

static void test_array_at_const(void) {
  array_t a = {0};
  int buf[2] = {9, 8};
  const void *p = NULL;
  assert(array_from(&a, buf, sizeof(int), 2) == D4F_ARRAY_OK);
  p = array_at_const(&a, 0);
  assert(p != NULL);
  assert(*(const int *)p == 9);
  p = array_at_const(&a, 1);
  assert(p != NULL);
  assert(*(const int *)p == 8);
  assert(array_at_const(&a, 2) == NULL);
  array_free(&a);
}

static void test_array_foreach(void) {
  array_t a = {0};
  int buf[3] = {1, 2, 3};
  int sum = 0;
  assert(array_from(&a, buf, sizeof(int), 3) == D4F_ARRAY_OK);
  assert(array_foreach(&a, cb_sum, &sum) == D4F_ARRAY_OK);
  assert(sum == 6);
  array_free(&a);
}

static void test_array_pop(void) {
  array_t a = {0};
  void *p = NULL;
  int v = 1;
  assert(array_init(&a, sizeof(int), 2) == D4F_ARRAY_OK);
  assert(array_push(&a, &v) == D4F_ARRAY_OK);
  assert(array_pop(&a, &p) == D4F_ARRAY_OK);
  assert(p != NULL);
  assert(*(int *)p == 1);
  assert(array_length(&a) == 0);
  array_free(&a);
}

static void test_array_remove_n_at(void) {
  array_t a = {0};
  int buf[4] = {1, 2, 3, 4};
  assert(array_from(&a, buf, sizeof(int), 4) == D4F_ARRAY_OK);
  assert(array_remove_n_at(&a, 1, 2) == D4F_ARRAY_OK);
  assert(array_length(&a) == 2);
  assert(*(int *)array_at(&a, 0) == 1);
  assert(*(int *)array_at(&a, 1) == 4);
  array_free(&a);
}

static void test_array_remove_at(void) {
  array_t a = {0};
  int buf[3] = {1, 2, 3};
  assert(array_from(&a, buf, sizeof(int), 3) == D4F_ARRAY_OK);
  assert(array_remove_at(&a, 1) == D4F_ARRAY_OK);
  assert(array_length(&a) == 2);
  assert(*(int *)array_at(&a, 0) == 1);
  assert(*(int *)array_at(&a, 1) == 3);
  array_free(&a);
}

static void test_array_reset(void) {
  array_t a = {0};
  int v = 1;
  assert(array_init(&a, sizeof(int), 2) == D4F_ARRAY_OK);
  assert(array_push(&a, &v) == D4F_ARRAY_OK);
  array_reset(&a);
  assert(array_length(&a) == 0);
  array_free(&a);
  array_reset(NULL);
}

static void test_array_capacity(void) {
  array_t a = {0};
  assert(array_init(&a, sizeof(int), 5) == D4F_ARRAY_OK);
  assert(array_capacity(&a) >= 5);
  array_free(&a);
}

static void test_array_dataptr(void) {
  array_t a = {0};
  int buf[1] = {42};
  assert(array_from(&a, buf, sizeof(int), 1) == D4F_ARRAY_OK);
  assert(array_dataptr(&a) != (void *)buf);
  assert(*(int *)array_dataptr(&a) == 42);
  array_free(&a);
}

static void test_array_length(void) {
  array_t a = {0};
  int buf[2] = {0, 0};
  assert(array_from(&a, buf, sizeof(int), 2) == D4F_ARRAY_OK);
  assert(array_length(&a) == 2);
  array_free(&a);
}

static void test_array_item_size(void) {
  array_t a = {0};
  assert(array_init(&a, sizeof(short), 2) == D4F_ARRAY_OK);
  assert(array_item_size(&a) == sizeof(short));
  array_free(&a);
}

static void test_array_status_message(void) {
  assert(strcmp(array_status_message(D4F_ARRAY_OK), "OK") == 0);
  assert(strcmp(array_status_message(D4F_ARRAY_BAD_ARGS), "Bad arguments") ==
         0);
  assert(strcmp(array_status_message(D4F_ARRAY_ALLOC_NULL),
                "Allocation failure") == 0);
  assert(strcmp(array_status_message((D4F_ARRAY_STATUS)9999), "UNKNOWN") == 0);
}

int main(void) {
  test_array_init();
  test_array_from();
  test_array_free();
  test_array_push_n();
  test_array_push();
  test_array_insert_n_at();
  test_array_insert_at();
  test_array_copy();
  test_array_slice();
  test_array_at();
  test_array_at_const();
  test_array_foreach();
  test_array_pop();
  test_array_remove_n_at();
  test_array_remove_at();
  test_array_reset();
  test_array_capacity();
  test_array_dataptr();
  test_array_length();
  test_array_item_size();
  test_array_status_message();
  return 0;
}
