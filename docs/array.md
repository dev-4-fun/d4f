# Array

A dynamic generic array: contiguous storage owned by the array, with growth via
`realloc`, iteration, push/pop, insert/remove, slice, copy, and foreach callbacks.

`array_t` is a typedef of [`array_view_t`](../include/d4f/array_view.h) (same
`data_ptr`, `length`, `item_size`, `capacity`). Unlike a plain view, an array
allocates and frees its buffer; use [`array_free`](../include/d4f/array.h) when
done. The implementation builds on the array view module.

Sources:

- `include/d4f/array.h`
- `include/d4f/array_view.h`
- `src/array.c`
- `src/array_view.c`

## Build

From the project root:

- **Local development (default):** outputs in `build/`, `include/`, `lib/`:

  ```bash
  make array
  ```

  This also builds `array_view` objects where needed: `libd4farray.a` archives
  both `array.o` and `array_view.o`, and `libd4farray.so` links the same pair.

- **Release build** (same output directories, stricter optimizations, no debug
  symbols in `CFLAGS`):

  ```bash
  make BUILD=release array
  ```

  Or build everything tagged for release:

  ```bash
  make release
  ```

- **Build individually (default = debug):**

  | Variant         | Command            | Output                         |
  | --------------- | ------------------ | ------------------------------ |
  | Static library  | `make array.a`     | `lib/libd4farray.a`            |
  | Dynamic library | `make array.so`    | `lib/libd4farray.so`           |
  | Header-only     | `make array_ho.h`  | `include/d4f/array_ho.h`       |

Debug flags: `-std=c89 -Wall -Wextra -Werror -pedantic -glldb`. Release adds
`-O3` and omits `-g`. `make clean` removes `build/`, generated `*_ho.h` under
`include/d4f/`, and `lib/`.

## Usage

### Header-only

In **exactly one** `.c` file define `D4F_ARRAY_IMPLEMENTATION` before including
the header, then include `d4f/array_ho.h` (this pulls in array view and array
implementation):

```c
#define D4F_ARRAY_IMPLEMENTATION
#include "d4f/array_ho.h"
```

Other translation units should include the same header without the macro. Add
the directory that contains `d4f/` to the include path: `-I include` (or your
install prefix).

### Static or dynamic library

Include `d4f/array.h` and link the produced library:

- Static: `lib/libd4farray.a` already contains `array_view` objects, so
  `-ld4farray` alone resolves `array_view_*` symbols. Use
  `lib/libd4farray_view.a` separately only when you need the view module without
  pulling in `array`.
- Dynamic: `lib/libd4farray.so` (links `array` and `array_view`).

Example (debug, static):

```bash
clang -std=c89 -Wall -Wextra -pedantic -I include \
  -L lib -ld4farray -o app main.c
```

For the shared library you may need `-Wl,-rpath,'$ORIGIN/../lib'` or an
equivalent so the loader finds `libd4farray.so`.

## API Overview

### Types

- `array_t`: same layout as `array_view_t`.
- `D4F_ARRAY_STATUS`: includes all
  [`D4F_ARRAY_VIEW_STATUS`](../include/d4f/array_view.h) values
  (`D4F_ARRAY_OK`, `D4F_ARRAY_BAD_ARGS`, `D4F_ARRAY_OUT_OF_BOUNDS`,
  `D4F_ARRAY_TYPE_MISMATCH`) plus `D4F_ARRAY_ALLOC_NULL` for allocation
  failure.
- `array_foreach_callback_t`: same as in array view,
  `int (*)(void *item, size_t i, void *ctx)`.

### Convenience macros

These forward to the underlying view helpers (cast internally):

- `array_capacity`, `array_dataptr`, `array_length`, `array_item_size`
- `array_push(array, item)` → `array_push_n` with `n == 1`
- `array_insert_at(array, index, item)` → `array_insert_n_at` with `n == 1`
- `array_remove_at` / `array_remove_n_at` → view remove
- `array_foreach`, `array_at`, `array_at_const`, `array_pop`
- `array_reset(array)` — sets `length` to `0`; does **not** free storage

### Functions

- `array_init(array, item_size, capacity)` — allocate at least `capacity`
  slots; `length` starts at `0`.
- `array_from(array, data, item_size, length)` — copy `length` elements from
  `data` into newly owned storage.
- `array_copy(src, dest)` — copy all elements of `src` into `dest` (via
  `array_from`).
- `array_slice(array, start, end, slice)` — take a Python-style slice of
  `array` (same index rules as [`array_view_slice`](../include/d4f/array_view.h)),
  then **copy** that range into `slice` with `array_from`. The result in
  `slice` owns its own buffer.
- `array_free(array)` — `free` the buffer and clear fields.
- `array_push_n(array, item, n)` — append `n` items from `item`; grows buffer
  as needed.
- `array_insert_n_at(array, index, items, n)` — insert `n` elements at `index`;
  grows as needed. Delegates to view insert after ensuring capacity.
- `array_status_message(status)` — human-readable string for a status code.

## Notes and Pitfalls

- Call `array_free` when the array owns data (after `array_init`, `array_from`,
  `array_copy`, `array_slice` into a fresh `slice`, etc.). Omitting `array_free`
  leaks the buffer.
- `array_reset` only clears `length`; capacity and `data_ptr` stay valid for
  reuse.
- `array_slice` **allocates** for `slice`; it is not a non-owning view over
  `array`.
- Insert/remove semantics and bounds match
  [`array_view_insert_n_at`](../include/d4f/array_view.h) /
  [`array_view_remove_n_at`](../include/d4f/array_view.h) after growth.

## Minimal example

```c
#define D4F_ARRAY_IMPLEMENTATION
#include "d4f/array_ho.h"

int main(void) {
  array_t a = {0};
  int v = 42;

  if (array_init(&a, sizeof(int), 4) != D4F_ARRAY_OK) {
    return 1;
  }
  if (array_push(&a, &v) != D4F_ARRAY_OK) {
    array_free(&a);
    return 1;
  }
  array_free(&a);
  return 0;
}
```

Run the unit tests from the project root:

```bash
make test
```

See `tests/array.test.c` for more usage patterns.
