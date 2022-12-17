# tables++

So, you need a hashtable? Well, good news, I've got some for you.

This repository is a collection of different hashtable-based containers - several flavors of API-compatible alternatives
for `std::unordered_map` and `std::unordered_set`, as well as accompanying `ordered_*` versions (insertion-order
preserving tables), `multiset`s and `multimap`s (associative tables with multiple keys per entry).

Here is a brief list of the features of this library:

* Open addressing (SwissHash) containers
    * Packed (elements are stored in a buffer, references are invalidated on resize)
        - [ ] `tpp::sparse_set`
        - [ ] `tpp::sparse_map`
        - [ ] `tpp::ordered_sparse_set`
        - [ ] `tpp::ordered_sparse_map`
        - [ ] `tpp::sparse_multiset`
        - [ ] `tpp::sparse_multimap`
    * Stable (elements are stored on the heap individually, references are stable)
        - [ ] `tpp::stable_set`
        - [ ] `tpp::stable_map`
        - [ ] `tpp::ordered_stable_set`
        - [ ] `tpp::ordered_stable_map`
        - [ ] `tpp::stable_multiset`
        - [ ] `tpp::stable_multimap`
* Closed addressing (sparse & dense array) containers
    - [x] `tpp::dense_set`
    - [x] `tpp::dense_map`
    - [x] `tpp::ordered_dense_set`
    - [x] `tpp::ordered_dense_map`
    - [x] `tpp::dense_multiset`
    - [x] `tpp::dense_multimap`
* Utilities & hash functions
    - [x] `tpp::hash_combine`
    - [x] `tpp::seahash_builder`
    - [x] `tpp::seahash`
    - [x] `tpp::fnv1a`
    - [x] `tpp::sdbm`
    - [x] `tpp::crc32`
    - [x] `tpp::md5`

## Build

Since **tables++** is a header-only library, the only thing you need is a C++ compiler with support for C++17.
Optionally, a `CMakeLists.txt` is also provided if you want to use the library as an `INTERFACE` CMake link target.

## Library& CMake options

<table>
  <tr><th>#define macro</th><th>CMake option</th><th>Default value</th><th>Description</th></tr>
  <tr>
    <td>TPP_USE_MODULES</td>
    <td>-DTPP_USE_MODULES</td>
    <td>ON</td>
    <td>Toggles support for C++20 modules</td>
  </tr>
  <tr>
    <td>TPP_NO_SIMD</td>
    <td>-DTPP_NO_SIMD</td>
    <td>OFF</td>
    <td>Toggles availability of SIMD optimizations for swiss tables</td>
  </tr>
  <tr>
    <td>TPP_NO_HASH</td>
    <td>-DTPP_NO_HASH</td>
    <td>OFF</td>
    <td>Toggles availability of hash function utilities</td>
  </tr>
  <tr>
    <td>TPP_OPTIONAL_HASH</td>
    <td>-DTPP_OPTIONAL_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/optional">&lt;optional&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_VARIANT_HASH</td>
    <td>-DTPP_VARIANT_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/variant">&lt;variant&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_STRING_HASH</td>
    <td>-DTPP_STRING_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/string">&lt;string&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_STRING_VIEW_HASH</td>
    <td>-DTPP_STRING_VIEW_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/string_view">&lt;string_view&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_BITSET_HASH</td>
    <td>-DTPP_BITSET_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/bitset">&lt;bitset&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_MEMORY_HASH</td>
    <td>-DTPP_MEMORY_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/memory">&lt;memory&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_TYPEINDEX_HASH</td>
    <td>-DTPP_TYPEINDEX_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/typeindex">&lt;typeindex&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_FILESYSTEM_HASH</td>
    <td>-DTPP_FILESYSTEM_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/filesystem">&lt;filesystem&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_THREAD_HASH</td>
    <td>-DTPP_THREAD_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/thread">&lt;thread&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_SYSTEM_ERROR_HASH</td>
    <td>-DTPP_SYSTEM_ERROR_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/system_error">&lt;system_error&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_COROUTINE_HASH</td>
    <td>-DTPP_COROUTINE_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/coroutine">&lt;coroutine&gt;</a> header</td>
  </tr>
  <tr>
    <td>TPP_STACKTRACE_HASH</td>
    <td>-DTPP_STACKTRACE_HASH</td>
    <td>OFF</td>
    <td>Enables hash specializations for the <a href="https://en.cppreference.com/w/cpp/header/stacktrace">&lt;stacktrace&gt;</a> header (requires C++23)</td>
  </tr>
  <tr>
    <td>TPP_STL_HASH_ALL</td>
    <td>-DTPP_STL_HASH_ALL</td>
    <td>OFF</td>
    <td>Enables all STL header specific options described above</td>
  </tr>
  <tr>
    <td>N/A</td>
    <td>-DTPP_TESTS</td>
    <td>OFF</td>
    <td>Enables unit test target</td>
  </tr>
</table>

If hash utility functions are enabled, all tables will use the `tpp::seahash_hash` hasher by default,
otherwise `std::hash` is used.

## Modules

At present, `tpp` has basic support for C++20 modules. If `TPP_USE_MODULES` option is enabled, and modules are supported
by the compiler, the library will prefer using `import` as opposed to `#include` for standard library headers.

In the future, a module interface may be provided for the library as an alternative to the current header API.

## API compatibility

While the provided map & set containers are largely API-compatible with `std::unordered_map` and `std::unordered_set`,
they are not guaranteed to be drop-in replacements for STL types, and neither provide the same pointer stability
guarantees.