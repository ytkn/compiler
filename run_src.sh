#!/bin/bash

make
./9cc "test_srcs/main.c" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?