#include <assert.h>
#include <string.h>

#define D4F_ARRAY_VIEW_IMPLEMENTATION
#include "d4f/array_view_ho.h"

static int cb_count(void *item, size_t i, void *ctx) {
  int *c = (int *)ctx;
  (void)item;
  (void)i;
  (*c)++;
  return 0;
}

static void test_array_view_create(void) {
  array_view_t v = {0};
  int buf[4] = {0};
  assert(array_view_create(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(v.data_ptr == (void *)buf);
  assert(v.length == 0);
  assert(v.capacity == 4);
  assert(v.item_size == sizeof(int));
}

static void test_array_view_wrap(void) {
  array_view_t v = {0};
  int buf[3] = {1, 2, 3};
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(v.length == 3);
  assert(v.capacity == 3);
  assert(*(int *)array_view_at(&v, 2) == 3);
}

static void test_array_view_init(void) {
  array_view_t v = {0};
  int items[4] = {0};
  assert(array_view_init(NULL, items, sizeof(*items), 1, 1) ==
         D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_init(&v, NULL, sizeof(*items), 1, 1) ==
         D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_init(&v, items, 0, 1, 1) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_init(&v, items, sizeof(*items), 1, 0) ==
         D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_init(&v, items, sizeof(*items), 2, 1) ==
         D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_init(&v, items, sizeof(*items), 0, 4) ==
         D4F_ARRAY_VIEW_OK);
  assert(v.data_ptr == items);
  assert(v.length == 0);
  assert(v.item_size == sizeof(*items));
  assert(v.capacity == 4);
}

static void test_array_view_length(void) {
  array_view_t v = {0};
  array_view_t *null_view = NULL;
  int buf[2] = {0};
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_length(&v) == 2);
  assert(array_view_length(null_view) == 0);
}

static void test_array_view_capacity(void) {
  array_view_t v = {0};
  array_view_t *null_view = NULL;
  int buf[5] = {0};
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_capacity(&v) == 5);
  assert(array_view_capacity(null_view) == 0);
}

static void test_array_view_item_size(void) {
  array_view_t v = {0};
  array_view_t *null_view = NULL;
  short buf[1] = {0};
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_item_size(&v) == sizeof(short));
  assert(array_view_item_size(null_view) == 0);
}

static void test_array_view_validate(void) {
  array_view_t v = {0};
  int buf[2] = {0};
  assert(array_view_validate(NULL) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_validate(&v) == D4F_ARRAY_VIEW_OK);
}

static void test_array_view_push(void) {
  array_view_t v = {0};
  int buf[2] = {0};
  int x = 9;
  assert(array_view_create(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_push(&v, &x) == D4F_ARRAY_VIEW_OK);
  assert(v.length == 1);
  assert(*(int *)array_view_at(&v, 0) == 9);
}

static void test_array_view_push_n(void) {
  array_view_t v = {0};
  int buf[4] = {0};
  int vals[2] = {3, 4};
  assert(array_view_create(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_push_n(NULL, vals, 1) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_push_n(&v, NULL, 1) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_push_n(&v, vals, 2) == D4F_ARRAY_VIEW_OK);
  assert(v.length == 2);
  assert(*(int *)array_view_at(&v, 0) == 3);
}

static void test_array_view_pop(void) {
  array_view_t v = {0};
  void *p = (void *)1;
  int buf[2] = {0};
  int x = 7;
  assert(array_view_create(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_pop(&v, NULL) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_pop(&v, &p) == D4F_ARRAY_VIEW_OUT_OF_BOUNDS);
  assert(p == NULL);
  assert(array_view_push(&v, &x) == D4F_ARRAY_VIEW_OK);
  assert(array_view_pop(&v, &p) == D4F_ARRAY_VIEW_OK);
  assert(*(int *)p == 7);
  assert(v.length == 0);
}

static void test_array_view_at(void) {
  array_view_t v = {0};
  int buf[2] = {11, 22};
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_at(NULL, 0) == NULL);
  assert(*(int *)array_view_at(&v, 0) == 11);
  assert(array_view_at(&v, 2) == NULL);
}

static void test_array_view_at_const(void) {
  array_view_t v = {0};
  int buf[1] = {5};
  const void *p = NULL;
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  p = array_view_at_const(&v, 0);
  assert(p != NULL);
  assert(*(const int *)p == 5);
}

static void test_array_view_copy(void) {
  array_view_t src = {0};
  array_view_t dest = {0};
  int a[3] = {1, 2, 3};
  int b[2] = {0, 0};
  assert(array_view_wrap(&src, a) == D4F_ARRAY_VIEW_OK);
  assert(array_view_init(&dest, b, sizeof(int), 0, 2) == D4F_ARRAY_VIEW_OK);
  assert(array_view_copy(&src, &dest) == D4F_ARRAY_VIEW_OK);
  assert(dest.length == 2);
  assert(*(int *)array_view_at(&dest, 0) == 1);
  assert(*(int *)array_view_at(&dest, 1) == 2);
}

static void test_array_view_slice(void) {
  array_view_t v = {0};
  array_view_t s = {0};
  int buf[5] = {0, 1, 2, 3, 4};
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_slice(NULL, 0, 1, &s) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_slice(&v, 0, 1, NULL) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_slice(&v, 1, 4, &s) == D4F_ARRAY_VIEW_OK);
  assert(s.length == 3);
  assert(*(int *)array_view_at(&s, 0) == 1);
  assert(array_view_slice(&v, 10, 11, &s) == D4F_ARRAY_VIEW_OUT_OF_BOUNDS);
}

static void test_array_view_insert_at(void) {
  array_view_t v = {0};
  int buf[3] = {0};
  int x = 8;
  assert(array_view_create(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_insert_at(&v, 0, &x) == D4F_ARRAY_VIEW_OK);
  assert(v.length == 1);
  assert(*(int *)array_view_at(&v, 0) == 8);
}

static void test_array_view_insert_n_at(void) {
  array_view_t v = {0};
  int buf[6] = {0};
  int pair[2] = {1, 2};
  assert(array_view_create(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_insert_n_at(&v, 0, pair, 2) == D4F_ARRAY_VIEW_OK);
  assert(v.length == 2);
  assert(array_view_insert_n_at(&v, 1, pair, 2) == D4F_ARRAY_VIEW_OK);
  assert(v.length == 4);
}

static void test_array_view_remove_at(void) {
  array_view_t v = {0};
  int buf[3] = {1, 2, 3};
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_remove_at(&v, 1) == D4F_ARRAY_VIEW_OK);
  assert(v.length == 2);
  assert(*(int *)array_view_at(&v, 0) == 1);
  assert(*(int *)array_view_at(&v, 1) == 3);
}

static void test_array_view_remove_n_at(void) {
  array_view_t v = {0};
  int buf[4] = {1, 2, 3, 4};
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_remove_n_at(&v, 1, 2) == D4F_ARRAY_VIEW_OK);
  assert(v.length == 2);
  assert(*(int *)array_view_at(&v, 0) == 1);
  assert(*(int *)array_view_at(&v, 1) == 4);
}

static void test_array_view_foreach(void) {
  array_view_t v = {0};
  int buf[3] = {0};
  int n = 0;
  assert(array_view_wrap(&v, buf) == D4F_ARRAY_VIEW_OK);
  assert(array_view_foreach(NULL, cb_count, &n) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_foreach(&v, NULL, &n) == D4F_ARRAY_VIEW_BAD_ARGS);
  assert(array_view_foreach(&v, cb_count, &n) == D4F_ARRAY_VIEW_OK);
  assert(n == 3);
}

static void test_array_view_status_message(void) {
  assert(strcmp(array_view_status_message(D4F_ARRAY_VIEW_OK), "OK") == 0);
  assert(strcmp(array_view_status_message(D4F_ARRAY_VIEW_OUT_OF_BOUNDS),
                "Out of bounds") == 0);
  assert(strcmp(array_view_status_message((D4F_ARRAY_VIEW_STATUS)9999),
                "UNKNOWN") == 0);
}

int main(void) {
  test_array_view_create();
  test_array_view_wrap();
  test_array_view_init();
  test_array_view_length();
  test_array_view_capacity();
  test_array_view_item_size();
  test_array_view_validate();
  test_array_view_push();
  test_array_view_push_n();
  test_array_view_pop();
  test_array_view_at();
  test_array_view_at_const();
  test_array_view_copy();
  test_array_view_slice();
  test_array_view_insert_at();
  test_array_view_insert_n_at();
  test_array_view_remove_at();
  test_array_view_remove_n_at();
  test_array_view_foreach();
  test_array_view_status_message();
  return 0;
}
