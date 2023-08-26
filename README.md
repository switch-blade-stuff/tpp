# tables++

So, you need a hashtable? Well, good news, I've got some for you.

This repository is a collection of different hashtable-based containers - several flavors of API-compatible alternatives
for `std::unordered_map` and `std::unordered_set`, as well as accompanying `ordered_*` versions (insertion-order
preserving tables), `dense_multiset` and `dense_multimap` (associative tables with multiple keys per entry).

Here is a brief list of the features of this library:

* Open addressing (SwissHash) containers
    * Packed (elements are stored in a buffer, references are invalidated on resize)
        - `tpp::sparse_set`
        - `tpp::sparse_map`
        - `tpp::ordered_sparse_set`
        - `tpp::ordered_sparse_map`
    * Stable (elements are stored on the heap individually, references are stable)
        - `tpp::stable_set`
        - `tpp::stable_map`
        - `tpp::ordered_stable_set`
        - `tpp::ordered_stable_map`
* Closed addressing (sparse & dense array) containers
    - `tpp::dense_set`
    - `tpp::dense_map`
    - `tpp::ordered_dense_set`
    - `tpp::ordered_dense_map`
    - `tpp::dense_multiset`
    - `tpp::dense_multimap`
* Hash utilities
    - `tpp::is_hash_builder`
    - `tpp::is_hash_builder_for`
    - `tpp::fnv1a_builder`
    - `tpp::sdbm_builder`
    - `tpp::hash_combine`

## Build

Since **tables++** is a header-only library, the only thing you need is a C++ compiler with support for C++17.
Optionally, a `CMakeLists.txt` is also provided if you want to use the library as an `INTERFACE` dependency.

## Library options

<table>
  <tr><th>#define macro</th><th>CMake option</th><th>Default value</th><th>Description</th></tr>
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
    <td>N/A</td>
    <td>-DTPP_TESTS</td>
    <td>OFF</td>
    <td>Enables unit test target</td>
  </tr>
</table>

If hash utility functions are enabled, all tables will use the `tpp::seahash_hash` hasher by default,
otherwise `std::hash` is used.

## API compatibility

While the provided map & set containers are largely API-compatible with `std::unordered_map` and `std::unordered_set`,
they are not guaranteed to be drop-in replacements for STL types, and neither provide the same pointer stability
guarantees.