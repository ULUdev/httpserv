#!/bin/sh

set_prefix() {
    cat Makefile | sed "s/^PREFIX = .*$/PREFIX = $(echo "$1" | sed 's/\//\\\//g')" > Makefile
}

print_help() {
    echo "
SYNOPSIS
  ./configure.sh [-hp <prefix>]
OPTIONS
  -h: print this help and exit
  -p <prefix>: set the desired prefix to <prefix>
  -t: run the tests
" >&2
}

run_tests() {
    make tests -Bk
}

main() {
    local opts
    while getopts "hp:t" opts; do
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
        esac
    done
}

main "$@"
