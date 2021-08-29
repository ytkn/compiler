#!/bin/bash

src="
int a;
int b;

int main(){ 
    a = 20;
    b = 10;
    return a+b;
}
"

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?