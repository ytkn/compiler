#!/bin/bash

assert(){
    expected=$1
    input=$2
    ./9cc "$input" > tmp.s
    cc -o tmp tmp.s
    ./tmp
    actual=$?
    if [ $actual = $expected ]; then
        echo "$input => $actual"
    else
        echo "$input => $expected expected, but got $actual"
        exit 1
    fi
}

assert 2 "int main(){ return 12+34+56-100; }"
assert 72 "int main(){ return 12*8-4*6; }"
assert 3 "int main(){ return 5+2*3-4*2; }"
assert 9 "int main(){ return -5+2*3+4*2; }"
assert 4 "int main(){ return (12*8)/(4*6); }"
assert 2 "int main(){ return 12+34+56-100; }"
assert 0 "int main(){ return 11 + 11 - 22; }"
assert 41 "int main(){ return  12 + 34 - 5 ; }"
assert 0 "int main(){ return 0 == 1; }"
assert 1 "int main(){ return 0 != 1; }"
assert 1 "int main(){ return 0 < 1; }"
assert 1 "int main(){ return 0 <= 1; }"
assert 0 "int main(){ return 0 > 1; }"
assert 0 "int main(){ return 0 >= 1; }"

assert 1 "int main(){ return 2*3+4 == 10-3+5*2-7; }"
assert 1 "int main(){ return 2*3+4 < 10-3+5*2; }"
assert 1 "int main(){ return 48/4 == 10-3+10/2; }"
assert 13 "int main(){ int a; a=1; int b; b = 12; return a+b; }"
assert 1 "int main(){ int a; a=1; int b; b = 12; return a*b-11; }"
assert 190 "int main(){ int aa; aa=10; int Aba; Aba=20; int x=10; return aa*Aba-x; }"
assert 96  "int main(){ int ten; ten=10; int two; two=2; int ret; ret=ten*two; return (ret+12)*3; }"
assert 96 "int main(){ int ten; ten=10; int two; two=2; int ret; ret=ten*two; return (ret+12)*3; int x; x=ten+two; return 100; }"
assert 25  "int main(){ int one; one=1; int two; two=2; int aa; aa=one+two; return (aa+12)/3+20; }"
assert 33 "int main(){ int a; a=1; if(a!=1) return 10; else a=a+1; if(a==2) return 33; }"
assert 10 "int main(){ int a; a=1; if(a==1) return 10; else a=a+1; if(a==2) return 33; }"
assert 11 "int main(){ int a; a=1; int b; for(b=0; b < 10; b=b+1) a=a+1; return a; }"
assert 6 "int main(){ int a; a=1; int b; for(b=1; b <= 10; b=b+2) a=a+1; return a; }"
assert 12 "int main(){ int a; a=0; while(a <= 10) a=a+3; return a; }"
assert 11 "int main(){ int a; a=0; while(a <= 10) a=a+1; return a; }"
assert 100 "int main(){ int a; a=0; int b; int c; for(b=0; b < 10; b=b+1) for(c=0; c < 10; c=c+1) a=a+1; return a; }"
assert 42 "
int add(int a, int b){
    return a+b;   
}

int main(){ 
    return add(10, 32); 
}
"
assert 89 "
int fib(int a){ 
    if(a <= 1) return 1;
    return fib(a-1)+fib(a-2);
}

int main(){ 
    return fib(10); 
}
"

assert 110 "
int sum(int a, int b){
    int ans;
    ans = 0;
    int i;
    for(i = a; i <= b; i=i+1){
        ans = ans+i;
        ans = ans+i;
    }
    return ans;
}
int main(){
    return sum(1, 10);
}
"

assert 2 "
int sub(int a, int b){
    return a-b;
}
int gcd(int a, int b){
    if(a > b) return gcd(b, a);
    if(a == 0) return b;
    return gcd(a, sub(b, a));   
}

int main(){ 
    return gcd(10, 32); 
}
"

assert 3 "
int main(){
    int x;
    int y;
    int z;
    x = 3;
    y = 5;
    z = &y + 8;
    return *z;
}
"

assert 3 "
int main(){
    int x;
    int *y;
    y = &x;
    *y = 3;
    return x;
}
"

assert 12 "
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

assert 4 "
int main(){ 
    int *x;
    int y;
    y = 2;
    return sizeof(y+5);
}
"

assert 8 "
int main(){ 
    int **x;
    int *y;
    y = 2;
    return sizeof(*x);
}
"

echo OK