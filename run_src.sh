#!/bin/bash

make
./9cc test_srcs/literal.c > tmp.s
cc -o tmp tmp.s
./tmp
echo $?