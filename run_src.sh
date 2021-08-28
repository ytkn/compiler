#!/bin/bash

src="
int main(){ 
    int fac[5];
    fac[0] = 1;
    int i;
    for(i = 1; i <= 4; i = i+1){
        fac[i] = fac[i-1]*i;
    }
    return fac[4];
}
"

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?