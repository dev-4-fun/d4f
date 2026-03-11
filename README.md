# d4f

A collection of small C micro-libraries, free to use.

**Status:** This library is under active development. APIs and module set may change.

**Author:** Dzmitry Fiodarau (fedor.dmitry@gmail.com) (aka Gribadze)  
**License:** MIT — see [LICENSE](LICENSE).

---

## Array

A dynamic generic array type: contiguous storage, iteration, push/pop, slice, copy, and callbacks.

- **Type:** `array_t` with `data_ptr`, `length`, and internal capacity.
- **API** (from `src/array.h`): `array_init`, `array_from`, `array_copy`, `array_slice`, `array_reset`, `array_at`, `array_push` / `array_push_n`, `array_pop`, `array_foreach`.
- **Sources:** `src/array.c`, `src/array.h`.

### Build

From the project root:

- **Build all array artifacts (static, shared, header-only):**

  ```bash
  make array
  ```

  This builds the static library, the shared library, and the header-only file. Directories `build/`, `include/d4f/`, and `lib/` are created automatically.

- **Build individually:**

  | Variant         | Command           | Output                   |
  | --------------- | ----------------- | ------------------------ |
  | Static library  | `make array.a`    | `lib/libd4farray.a`      |
  | Dynamic library | `make array.so`   | `lib/libd4farray.so`     |
  | Header-only     | `make array_ho.h` | `include/d4f/array_ho.h` |

  Compiler and flags (from the Makefile): `CC=clang`, `CFLAGS=-std=c89 -Wall -Wextra -pedantic -glldb`.

### Usage

- **Header-only:** In exactly one `.c` file define `D4F_ARRAY_IMPLEMENTATION` before including the header, then include `include/d4f/array_ho.h` (or your path to it). Other translation units include the same header without the macro.
- **Static or dynamic:** Include `include/d4f/array.h` and link with `lib/libd4farray.a` or `-Llib -ld4farray` as appropriate.

### Example

`examples/array_demo.c` uses the header-only variant. It:

1. Creates an array from existing data with `array_from` and an empty array with capacity via `array_init`.
2. Builds a “string” from a char array and uses `array_slice` (e.g. negative indices) to get a substring.
3. Uses `array_foreach` with callbacks: one to print each element, another to “map” (e.g. multiply by 2) into a second array.

Snippet:

```c
#define D4F_ARRAY_IMPLEMENTATION
#include "include/d4f/array_ho.h"

array_t arr = {0};
array_from(&arr, data, sizeof(*data), count);
array_foreach(&arr, print_item, NULL);
array_slice(&arr, -10, -2, &slice);
```

Build and run the demo (after `make array`):

```bash
clang -std=c89 -Wall -Wextra -pedantic -I. -o array_demo examples/array_demo.c && ./array_demo
```

---

## Future

More micro-libraries will be documented in this README in separate sections. An **all-in-one** build (single library or header combining all modules) is planned.
