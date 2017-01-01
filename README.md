## Building
To build just run `make`. You have to have the to have the fra library installed on the build system ( `<fra/core.h>` somewhere in your include path ). If you don't you can also build like this:
```bash
export C_INCLUDE_PATH=/your/path/to/FastCGI-Rest-API/include
make
```

## Running tests
To run the tests just execute `./build_and_run.sh` in the `tests` directory. You have to have the fra library installed on the build system ( `libfra.a` or `libfra.so` must be somewhere in your library path ). If you don't you can also run the test like this:
```bash
export C_INCLUDE_PATH=/your/path/to/FastCGI-Rest-API/include
export LIBRARY_PATH=/your/path/to/FastCGI-Rest-API/build
./build_and_run.sh
```

