#!/bin/bash

OBJECT=main
CFLAGS=
SOURCE=
LIB=
LIBPATH=
INCLUDE=

AddFlag() {
    CFLAGS+=("$1")
}

AddFile() {
    SOURCE+=("../$1")
}

AddInclude() {
    INCLUDE+=("-I$1/include")
}

AddIncludeRaw() {
    INCLUDE+=("-I$1")
}

AddLib() {
    LIB+=("-l$1")
    LIBPATH+=("-L$2/lib")
    AddInclude $2
}

SetObjectName() {
    OBJECT=$1
}

EchoAndRun() {
    echo "$@"
    $1
}

Compile() {
    for file in *.cpp; do
        AddFile $file;
    done
    g++ $(printf "%s " "${CFLAGS[@]}") -std=c++11 -g $(printf "%s " "${SOURCE[@]}") -o ../$OBJECT $(printf "%s " "${LIB[@]}") $(printf "%s " "${LIBPATH[@]}") $(printf "%s " "${INCLUDE[@]}")
}

