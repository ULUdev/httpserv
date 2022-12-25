# `httpserv`
## An http server building library written in C (this project is inspired by [`rsweb`](http://gitlab.sokoll.com/moritz/rsweb))
[![pipeline status](https://gitlab.sokoll.com/moritz/httpserv/badges/main/pipeline.svg)](https://gitlab.sokoll.com/moritz/httpserv/-/commits/main) 

## Introduction
This project aims to provide a simple web server with `cweb` as well as a
powerful abstraction library with `libhttpserv`. Everything in this server is
meant to conform to `HTTP/1.1`

## Compilation
### Dependencies
Currently there is [`libtree`](http://gitlab.sokoll.com/moritz/libtree) as a
dependency, which is managed through `git` submodules. However for testing you
need to have `cmocka` installed.
### Building
1. `git submodule init`
2. `git submodule update`
3. `./configure`
4. `make`

This will build `libhttpserv.so` and `cweb`.
### Configure script
The script `configure.sh` can be used to control the compilation process. If
you wish to combine settings just pass them all to `./configure.sh`
#### Prefix
The prefix controls the directory prefix to which `make install` installs the
generated binaries and libraries. Usually this will be something like `/usr` or
`/usr/local`. To change it use `./configure.sh -p <prefix>`
#### Release
The release mode compiles everything with optimization enabled. In the future
it may also control certain features (for example debug logging that will be
disabled in release mode). Enable it with `./configure.sh -r`
#### Debug
The debug mode compiles everything with debug flags for `gdb`. Enable it with
`./configure.sh -d`
#### Environment
The environment is only important to compilation for tests at the moment. As in
docker containers you can't do IPv6. Possible values are:
- `TEST_ENV_DOCKER`: The docker environment (no IPv6)
- `TEST_ENV_NORMAL`: The normal environment

### Testing
For testing you need to have `cmocka` installed. If you have it installed you
can run `make tests` and it will run through the tests.
