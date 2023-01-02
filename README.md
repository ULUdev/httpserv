# `httpserv`
## An http server building library written in C (this project is inspired by [`rsweb`](http://gitlab.sokoll.com/moritz/rsweb))
[![pipeline status](https://gitlab.sokoll.com/moritz/httpserv/badges/main/pipeline.svg)](https://gitlab.sokoll.com/moritz/httpserv/-/commits/main) 

## Introduction
This project aims to provide a simple web server with `cweb` as well as a
powerful abstraction library with `libhttpserv`. Everything in this server is
meant to conform to `HTTP/1.1`

## Compilation
### Dependencies
- [`openssl`](http://www.openssl.org)
- [`libtree`](http://gitlab.sokoll.com/moritz/libtree) (managed as a submodule)
- [`cmocka`](http://cmocka.org) (for testing only)
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

## `cweb`
### Invoking the program
After compilation you can simply run `./cweb` in the root of the project. This
will run `cweb` with its defaults. To see the options you can pass it run
`./cweb --help`.
### Configuration
`cweb` has two configuration files: One controlling basic behavior and one for
setting routes.
#### Basic configuration
The basic configuration is a key-value configuration file. However it does have
namespaces

```
# this is a comment
http.some.value = hello
http.ip = ::1
http.port = 8080
# an actual value isn't needed here, however it needs to be parsed by the
# parser
http.v6 = true
```
A comment starts with a `#`. The parser treats all values as strings (so after
parsing the value of `http.port` is the string `8080` instead of the number)
##### SSL
For `ssl` support you only need to add the following lines:

```
ssl.certfile = certs.pem
ssl.privkeyfile = privkey.pem
```
### Route configuration
The route configuration file defines the routes that should be loaded into the
web server. Its syntax is `<regex>=><routesto>[|<kind>]`. A regular expression,
for which request paths are to be routed, followed by `=>` and the path they
will be routed to and optionally a `|` followed by the kind of route one of
- `alias` (instead of the requested path, `<routesto>` will be used and 200 should be used)
- `redirect` 301 will be used (browsers may cache this response and
  automatically redirect on their end)
- `route` 302 will be used (browsers won't cache)
