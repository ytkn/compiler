#!/bin/bash

src="
int rem(int a, int b){
    while(a >= b){
        a = a-b;
    }
    return a;
}

int main(){ 
    int fib[15];
    fib[0] = 1;
    fib[1] = 1;
    int i;
    for(i = 2; i < 15; i = i+1){
        fib[i] = fib[i-1]+fib[i-2];
    }
    int x;
    x = fib[14];
    // fib[14] = 10
    return rem(x, 100);
}
"


# (aa+12)/3+20;

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?