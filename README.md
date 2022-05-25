*This project is being developed as part of a 4th semester project at AAU.*

# Build toolchain requirements

- CMake >= 3.16.0
- Clang
- ANTLR4
- python3 (optional - required for included build script)


# Build Dependencies

Most dependencies are either managed as vendored dependencies or fetched during the automated build process.
Exception are marked with \*.

- [catch2](https://github.com/catchorg/Catch2)
- [ANTLR4](https://github.com/antlr/antlr4)
- [cxxopts](https://github.com/jarro2783/cxxopts)
- pkg-config\*
- uuid-dev\*
- {fmt} v8.1.1\*

# Build instructions
To build the project the above toolchains and build dependencies must be installed.
The project includes a python build script for building the project. Example usage:

```shell
./build --release
```

For the build to succeed cmake must be able to find the antlr binary(.jar file), the easiest way
to ensure that it is found is to move the .jar file to the project root and rename it to `antlr.jar`.

# Runtime Dependencies

- avr-gcc