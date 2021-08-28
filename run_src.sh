#!/bin/bash

src="
int main(){ 
    int a[15];
    *a = 1;
    *(a+1) = 1;
    *(a+2) = 2;
    return *(a+1+1);
}
"

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?