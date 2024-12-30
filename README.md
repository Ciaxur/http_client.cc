# HTTP Client
Http client for c++ that uses libcurl under the hood.
This library is a thin-wrapper around libcurl.

# Usage
This library requires `libcurl` and `fmt` to be installed.

## Examples
See [examples](./examples) for how to use this library.

Compile and run like so:
```sh
$ g++ -lcurl -lfmt -o main -I ./include ./examples/basic_scoped_curl.cc $(find ./src -name "*.cc")
$ ./main
```

## Build
Recommend using the [build.sh](./scripts/build.sh) script to build and run
a given source file like so:

```sh
# Building
$ ./scripts/build.sh ./examples/basic_curl.cc
Compiled ./examples/basic_curl.cc -> main

# Invoke built binary
$ ./main
```

