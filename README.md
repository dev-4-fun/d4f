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

- **Local development (default):** debug symbols, output in `build/`, `include/`, `lib/` (these dirs are gitignored):

  ```bash
  make array
  ```

- **Release build for distribution:** `-O3`, no debug info, output in `dist/` (suitable for committing):

  ```bash
  make release
  ```

  This produces `dist/include/d4f/`, `dist/lib/` with the same artifacts. Use when you want to add prebuilt libraries to the repo.

- **Build individually (default = debug):**

  | Variant         | Command           | Output (debug)              | Output (release)                 |
  | --------------- | ----------------- | --------------------------- | -------------------------------- |
  | Static library  | `make array.a`    | `lib/libd4farray.a`         | `dist/lib/libd4farray.a`         |
  | Dynamic library | `make array.so`   | `lib/libd4farray.so`        | `dist/lib/libd4farray.so`        |
  | Header-only     | `make array_ho.h` | `include/d4f/array_ho.h`    | `dist/include/d4f/array_ho.h`    |

  Debug: `CFLAGS=-std=c89 -Wall -Wextra -pedantic -glldb`. Release: same plus `-O3`, no `-g`. `make clean` cleans dev dirs; `make clean-release` removes `dist/`.

### Usage

- **Header-only:** In exactly one `.c` file define `D4F_ARRAY_IMPLEMENTATION` before including the header, then `#include "d4f/array_ho.h"`. Add the directory that *contains* `d4f/` to the include path (e.g. `-I include` for dev or `-I dist/include` for release). Other translation units include the same header without the macro.
- **Static or dynamic:** Include `d4f/array.h` (with `-I include` or `-I dist/include`) and link with `lib/libd4farray.a` or `-Llib -ld4farray` as appropriate.

### Example

`examples/array_demo.c` uses the header-only variant. It:

1. Creates an array from existing data with `array_from` and an empty array with capacity via `array_init`.
2. Builds a “string” from a char array and uses `array_slice` (e.g. negative indices) to get a substring, then prints both.
3. Uses `array_foreach` with callbacks: one to print each element, another to “map” (e.g. multiply by 2) into a second array.
4. Calls `array_reset` on all arrays before exit.

Snippet:

```c
#define D4F_ARRAY_IMPLEMENTATION
#include "d4f/array_ho.h"

array_t arr = {0};
array_from(&arr, data, sizeof(*data), count);
array_foreach(&arr, print_item, NULL);
array_slice(&arr, -10, -2, &slice);
```

Build and run the demo:

```bash
make demo
./build/demo/array_demo
```

For a release build, run `make BUILD=release demo`, then `./dist/demo/array_demo`. To build the demo manually (with headers in `include/`): `clang -std=c89 -Wall -Wextra -pedantic -I include -o array_demo examples/array_demo.c`.

---

## Array View

A lightweight view over existing contiguous memory. Build and usage details are in
`docs/array_view.md`.

---

## Future

More micro-libraries will be documented in this README in separate sections. An **all-in-one** build (single library or header combining all modules) is planned.
