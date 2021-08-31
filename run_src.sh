#!/bin/bash

src="
int main(){ 
    char a;
    char b;
    a = 1;
    b = 10;
    return b+a;
}
"


# (aa+12)/3+20;

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?