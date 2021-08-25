#!/bin/bash

src="
int main(){
    int x;
    int *y;
    int z;
    y = &z;
    y = y+2;
    *y = 2;
    return x;
}
"

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?