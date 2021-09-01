#!/bin/bash

make
./9cc test_srcs/test.c > tmp.s
cc -o tmp tmp.s
./tmp
