# `httpserv`
## An http server building library written in C (this project is inspired by
## [`rsweb`](http://gitlab.sokoll.com/moritz/rsweb))

## Compilation
### Dependencies
Currently there is [`libtree`](http://gitlab.sokoll.com/moritz/libtree) as a
dependency, which is managed through `git` submodules. However for testing you
need to have `cmocka` installed.
### Building
1. `git submodule init`
2. `git submodule update`
3. `make`
This will build the library and
### Testing
For testing you need to have `cmocka` installed. If you have it installed you
can run `make tests` and it will run through the tests.
