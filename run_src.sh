#!/bin/bash

src="
int sub(int a, int b){
    int ret;
    ret = a-b;
    return ret;
}
int gcd(int a, int b){
    if(a > b) return gcd(b, a);
    if(a == 0) return b;
    return gcd(a, sub(b, a));   
}

int main(){
    int a;
    int b;
    a = 10;
    b = 32;
    return gcd(10, 32); 
}
"

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?