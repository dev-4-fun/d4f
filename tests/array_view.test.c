#include <stdio.h>
#include <stdlib.h>

#define D4F_ARRAY_VIEW_IMPLEMENTATION
#include "d4f/array_view_ho.h"

int print_number_item(void *item, size_t i, void *ctx) {
  (void)ctx;
  printf("[%zu] = %d\n", i, *(int *)item);
  return 0;
}

int filter_odd(void *item, size_t i, void *ctx) {
  int *num = item;
  array_view_t *filtered = ctx;
  (void)i;
  if (*num % 2 == 0) {
    array_view_push(filtered, num);
  }
  return 0;
}

int static_array_test(void) {
  int numbers[] = {0, 2, 5, 1, 7};
  int filtered[1];
  array_view_t array_numbers = {0};
  array_view_t array_filtered = {0};
  array_view_t array_slice = {0};

  array_view_wrap(&array_numbers, numbers);
  array_view_foreach(&array_numbers, print_number_item, NULL);
  printf("\n");

  array_view_create(&array_filtered, filtered);
  array_view_foreach(&array_numbers, filter_odd, &array_filtered);
  array_view_foreach(&array_filtered, print_number_item, NULL);
  printf("\n");

  array_view_slice(&array_numbers, -2, 0, &array_slice);
  *(int *)array_view_at(&array_slice, 0) = 99;
  array_view_foreach(&array_slice, print_number_item, NULL);
  printf("\n");

  array_view_foreach(&array_numbers, print_number_item, NULL);
  printf("\n");

  return 0;
}

int fill_by_index(void *item, size_t i, void *ctx) {
  *(int *)item = i;
  (void)ctx;
  return 0;
}

int dynamic_array_test(void) {
  int *numbers = malloc(5 * sizeof(int));
  int *filtered = malloc(10 * sizeof(int));
  array_view_t array_numbers = {0};
  array_view_t array_filtered = {0};
  array_view_t array_slice = {0};

  array_view_init(&array_numbers, numbers, sizeof(*numbers), 5, 5);
  array_view_foreach(&array_numbers, fill_by_index, NULL);
  array_view_foreach(&array_numbers, print_number_item, NULL);
  printf("\n");

  array_view_init(&array_filtered, filtered, sizeof(*filtered), 0, 10);
  array_view_foreach(&array_numbers, filter_odd, &array_filtered);
  array_view_foreach(&array_filtered, print_number_item, NULL);
  printf("\n");

  array_view_slice(&array_numbers, -2, 0, &array_slice);
  *(int *)array_view_at(&array_slice, 0) = 99;
  array_view_foreach(&array_slice, print_number_item, NULL);
  printf("\n");

  array_view_foreach(&array_numbers, print_number_item, NULL);
  printf("\n");

  free(numbers);
  free(filtered);
  return 0;
}

int main(void) {
  static_array_test();
  dynamic_array_test();
  return 0;
}
