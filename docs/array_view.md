# Array View

A lightweight "view" over existing contiguous memory. It does not own storage
and does not allocate. The view tracks `data_ptr`, `length`, `item_size`,
and `capacity` and provides bounds-checked access, slicing, push/pop, and
foreach callbacks.

Sources:
- `src/array_view.h`
- `src/array_view.c`

## Build

From the project root:

- **Local development (default):** outputs in `build/`, `include/`, `lib/`:

  ```bash
  make array_view
  ```

- **Release build:** `-O3`, no debug info, outputs in `dist/`:

  ```bash
  make BUILD=release array_view
  ```

- **Build individually (default = debug):**

  | Variant         | Command               | Output (debug)                       | Output (release)                          |
  | --------------- | --------------------- | ------------------------------------ | ----------------------------------------- |
  | Static library  | `make array_view.a`   | `lib/libd4farray_view.a`             | `dist/lib/libd4farray_view.a`             |
  | Dynamic library | `make array_view.so`  | `lib/libd4farray_view.so`            | `dist/lib/libd4farray_view.so`            |
  | Header-only     | `make array_view_ho.h`| `include/d4f/array_view_ho.h`        | `dist/include/d4f/array_view_ho.h`        |

Debug flags: `-std=c89 -Wall -Wextra -pedantic -glldb`. Release adds `-O3`
and omits debug symbols. `make clean` removes dev outputs; `make clean-release`
removes `dist/`.

## Usage

### Header-only

In **exactly one** `.c` file define `D4F_ARRAY_VIEW_IMPLEMENTATION` before
including the header, then include `d4f/array_view_ho.h`:

```c
#define D4F_ARRAY_VIEW_IMPLEMENTATION
#include "d4f/array_view_ho.h"
```

Other translation units should include the same header without the macro.
Add the directory that contains `d4f/` to the include path: `-I include`
for debug or `-I dist/include` for release.

### Static or dynamic library

Include `d4f/array_view.h` and link to the produced library:

- Static: `lib/libd4farray_view.a`
- Dynamic: `lib/libd4farray_view.so` (or the `dist/` equivalents)

Example (debug):

```bash
clang -std=c89 -Wall -Wextra -pedantic -I include \
  -L lib -ld4farray_view -o app main.c
```

## API Overview

### Types

- `array_view_t`: `{ void *data_ptr; size_t length; size_t item_size; size_t capacity; }`
- `D4F_ARRAY_VIEW_STATUS`: `D4F_ARRAY_VIEW_OK`, `D4F_ARRAY_VIEW_BAD_ARGS`,
  `D4F_ARRAY_VIEW_OUT_OF_BOUNDS`, `D4F_ARRAY_VIEW_TYPE_MISMATCH`
- `array_foreach_callback_t`: `int (*)(void *item, size_t i, void *ctx)`

### Convenience macros

- `array_view_create(view, buffer)`  
  Initialize an empty view over a fixed-size C array (`length = 0`).
- `array_view_wrap(view, items)`  
  Wrap a fixed-size C array as a full view (`length = capacity`).
- `array_view_push(view, item)`  
  Push a single item (by pointer) into the view.
- `array_view_length(view)`, `array_view_capacity(view)`,
  `array_view_item_size(view)`

### Functions

- `array_view_init(view, items, item_size, length, capacity)`  
  Create a view over `items`. Fails if `length > capacity` or arguments
  are invalid.
- `array_view_copy(src, dest)`  
  Copies up to `dest->capacity` items. Requires matching `item_size`.
- `array_view_slice(view, start, end, slice)`  
  Creates a view slice with Python-like indexing. `start` and `end` may
  be negative. `end` is **exclusive** (i.e., slice is `[start, end)`).
- `array_view_validate(view)`  
  Checks basic invariants.
- `array_view_at(view, i)` / `array_view_at_const(view, i)`  
  Bounds-checked access within `length`.
- `array_view_push_n(view, items, n)`  
  Appends `n` items from `items`. Fails if exceeding `capacity`.
- `array_view_pop(view, &item)`  
  Removes the last item and returns its address via `item`.
- `array_view_foreach(view, cb, ctx)`  
  Iterates items until callback returns non-zero.
- `array_view_status_message(status)`  
  Returns a human-readable status string.

## Notes and Pitfalls

- The view does **not** allocate or free memory. The caller owns the storage.
- `array_view_pop` returns a pointer to the item in the original buffer;
  the data remains valid as long as the underlying storage does.
- `array_view_slice` enforces bounds. Out-of-range indices return
  `D4F_ARRAY_VIEW_OUT_OF_BOUNDS`.
- `array_view_push_n` and `array_view_pop` operate only within `capacity`
  and `length` respectively; both validate the view first.

## Minimal Example

```c
#define D4F_ARRAY_VIEW_IMPLEMENTATION
#include "d4f/array_view_ho.h"

int main(void) {
  int data[8] = {0};
  array_view_t view = {0};

  array_view_create(&view, data);
  int value = 42;
  array_view_push(&view, &value);

  array_view_t slice = {0};
  array_view_slice(&view, 0, 1, &slice);

  return 0;
}
```
