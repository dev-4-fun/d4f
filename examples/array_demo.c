#include <stdio.h>
#include <string.h>

#define D4F_ARRAY_IMPLEMENTATION
#include "d4f/array_ho.h"

int print_item(void *item, size_t i, void *ctx) {
  const int val = *(int *)item;
  (void)ctx;
  printf("a[%zu] = %d\n", i, val);
  return 0;
}

int mult_by_2(void *item, size_t i, void *ctx) {
  array_t *dest = ctx;
  int val = *(int *)item * 2;
  (void)i;
  if (!array_push(dest, &val)) {
    fprintf(stderr, "Could not push item\n");
    return 1;
  }
  return 0;
}

int main(void) {
  int arr[] = {1, 2, 3, 4, 5};
  size_t arr_length = sizeof(arr) / sizeof(*arr);
  char *hello = "Hello, world!";
  array_t arrays[] = {{0}, {0}};
  array_t string = {0};
  array_t string_slice = {0};
  array_from(&arrays[0], arr, sizeof(*arr), arr_length);
  array_init(&arrays[1], sizeof(*arr), arr_length);
  array_from(&string, hello, sizeof(*hello), strlen(hello));
  array_slice(&string, -10, -2, &string_slice);

  printf("array\n");
  array_foreach(&arrays[0], print_item, NULL);

  printf("mapped\n");
  array_foreach(&arrays[0], mult_by_2, &arrays[1]);
  array_foreach(&arrays[1], print_item, NULL);

  printf("string\n");
  printf("%s\n", (char *)string.data_ptr);

  printf("string slice\n");
  printf("%s\n", (char *)string_slice.data_ptr);

  array_reset(&arrays[0]);
  array_reset(&arrays[1]);
  array_reset(&string);
  array_reset(&string_slice);
}
