#!/bin/sh

run_libtree() {
    cd libtree
    $@
    cd ..
}

set_prefix() {
    cat Makefile | sed "s/^PREFIX = .*$/PREFIX = $(echo "$1" | sed 's/\//\\\//g')/" > Makefile
    run_libtree "./configure.sh -p $1"
}

print_help() {
    echo "
SYNOPSIS
  ./configure.sh [-hrdp <prefix>]
OPTIONS
  -h: print this help and exit
  -p <prefix>: set the desired prefix to <prefix>
  -t: run the tests
  -r: toggle release mode
  -d: toggle debug mode
  -e <env>: set the compile environment to <env>
" >&2
}

prepare_release() {
    sed -i 's/-ggdb/-O3/g' Makefile
    run_libtree "./configure.sh -r"
}

prepare_debug() {
    sed -i 's/-O3/-ggdb/g' Makefile
    run_libtree "./configure.sh -d"
}

run_tests() {
    make tests -Bk
}

set_comp_env() {
    sed -i "s/^ENV = .*\$/ENV = $OPTARG/" test/Makefile
}

main() {
    local opts
    while getopts "hp:trde:" opts; do
        case $opts in
            h)
                print_help
                exit 0
                ;;
            p)
                set_prefix "$OPTARG"
                ;;
            t)
                run_tests
                ;;
            r)
                prepare_release
                ;;
            d)
                prepare_debug
                ;;
            e)
                set_comp_env "$OPTARG"
                ;;
        esac
    done
}

main "$@"
