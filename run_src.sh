#!/bin/bash

src="
main(){
    x = 3;
    y = 5;
    z = &y + 8;
    return *z;
}
"

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?