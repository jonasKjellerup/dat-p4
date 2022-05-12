This document provides a guide for the formatting of c++ code for this project.

# Naming conventions

- Function/method names should use `snake_case`.
- Variable/field names should use `snake_case`.
- Namespaces should use `snake_case`.
- Types should use `PascalCase`. An exception to this is allowed when a type is used solely as an internal type in which 
  case an arbitrary suffix `_someSuffixHere` may be added. (See example at the end of this section for example of the exception)
- Enum variants should use `PascalCase`.

An example of acceptable use suffixes on struct names can be seen below,
where we alias `SomeType` to be a shared pointer to a `SomeType_` in this
case `SomeType` is not intended to be used directly, hence the exception
to the naming rules.
```c++
struct SomeType_ {};
using SomeType = std::shared_ptr<SomeType>
```

# Struct vs class declarations
Generally `struct` is preferred over `class` for the sake of consistency.
The only difference between the two is the default access modifier (struct is public by default).

# Enum declarations

To avoid unnecessary naming conflicts all enums should be declared with `enum struct` instead of just `enum`.

Discouraged enum use:
```c++
// This will cause enum variants to be declared
// in the global scope.
enum Weekday {
    Monday,
    Tuesday,
    ...
};

struct Monday {};
// This causes an error since Monday refers to Weekday::Monday
Monday instance;
// In order to refer to the struct you would have to write
struct Monday instance;
```

Encouraged enum use
```c++
// This will restrict the variants the namespace of the enum type.
enum struct Weekday {
    Monday,
    Tuesday,
    ...
};

struct Monday {};
// This no longer errors as Monday no longer refers to Weekday::Monday.
Monday instance;
```

# Pointer and reference marker alignment
When marking something as a pointer or reference it preferred to align the `*` or `&` 
symbols with the type instead of the identifier. E.g. `int* x;` is preferred to `int *x`.

# Namespaces

Generally speaking, any code that is contained within a header files should be put in an appropriate namespace.
This is not required for non-header files since their contents won't leak into other files.

Similarly, the use of `using x` and `using namespace x` directives should be limited in cases where it can affect
other files, e.g. globally in a header is a no-go.


# Header guards
For header files the `#pragma once` directive should be used instead of traditional header guards.

