#!/bin/bash

make
./9cc test_srcs/print_int.c > tmp.s
cc -o tmp tmp.s
./tmp
echo $?