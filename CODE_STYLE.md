# Code Style

## C++ Code style

- Indent everything: namespaces, class member accessor modifiers, switch statements.

  ```c++
  namespace example {
      class Example {
          public:
              void example();
      };
      
      void example_fn() {
          int example_n = 0;
          switch (example_n) {
              case 1:
                  break;
              case 2:
                  break;
          }
      }
  }
  ```

- When using a `class` prefer private members at top of class without explicit private.
- Prefix members with `m_`.

  ```c++
  class Example {
      int m_example;
      public:
          int example() {
              return m_example;
          }
  };
  ```

- IWYU: Include what you use.

  Only select libraries should follow this.

  - GLM
  - c++ stl - reduce transitive includes

- Include ordering

  1. Primary header for `.cpp` file
  2. Project headers
  3. Library headers. Grouped by library.
  4. C++ STL headers.

# CMake

- When adding dependencies use `feature_find_package` instead of `find_package` as this allows `feature_summary` to
  know about all packages before all required have been found. If a dependency provides cmake functions or macros place 
  `defer_on_missing_required_packages` after `feature_find_package` and after any `add_subdirectory` this will 
  short circuit cmake configuration and return up the subdirectory tree until `feature_sumary`

  This functionality is provided by `include(FeatureFindPackage)`

  `feature_find_package` extends find_package with `REQUIRED;OPTIONAL;REQUIRED_COMPONENTS;OPTIONAL_COMPONENTS`