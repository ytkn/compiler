#!/bin/bash

src="
int main(){
    int x;
    int *y;
    int **z;
    y = &x;
    z = &y;
    **z = 12;
    return x;
}
"

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?