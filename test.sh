#!/bin/bash

assert(){
    expected=$1
    input=$2
    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual=$?
    if [ $actual = $expected ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 2 12+34+56-100
assert 72 12*8-4*6
assert 3 5+2*3-4*2
assert 4 "(12*8)/(4*6)"
assert 2 12+34+56-100
assert 0 "11 + 11 - 22"
assert 41 " 12 + 34 - 5 "


echo OK