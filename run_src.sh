#!/bin/bash

src="
int main(){ 
    int x[10];
    int y;
    y = 2;
    return sizeof(y);
}
"

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?