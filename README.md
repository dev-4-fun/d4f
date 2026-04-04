# d4f

A collection of small C micro-libraries, free to use.

**Status:** This library is under active development. APIs and module set may change.

**Author:** Dzmitry Fiodarau (fedor.dmitry@gmail.com) (aka Gribadze)  
**License:** MIT — see [LICENSE](LICENSE).

---

## Array

A dynamic generic array with owned storage, automatic growth, and the same
logical surface as the view layer (push, pop, insert/remove, slice, foreach).
Public headers live under `include/d4f/`; implementations are in `src/`. Full
build instructions, linking notes, API details, and examples are in
[`docs/array.md`](docs/array.md).

---

## Array View

A lightweight view over existing contiguous memory. Build and usage details are in
[`docs/array_view.md`](docs/array_view.md).

---

## Future

More micro-libraries will be documented under `docs/` with short pointers here.
An **all-in-one** build (single library or header combining all modules) is
planned.
