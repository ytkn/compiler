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
    int *z;
    x = 3;
    y = 5;
    z = &y + 2;
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

assert 3 "
int main(){
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p;
    p = a;
    return *p + *(p + 1);
}
"

assert 89 "
int calc_fib(int n){
    int fib[15];
    *fib = 1;
    *(fib+1) = 1;
    int i;
    for(i = 2; i < 15; i = i+1){
        *(fib+i) = *(fib+(i-1)) + *(fib+(i-2));
    }
    return *(fib+(n-1));
}

int main(){
    return calc_fib(11);
}
"

assert 233 "
int main(){ 
    int fib[15];
    fib[0] = 1;
    fib[1] = 1;
    int i;
    for(i = 2; i < 13; i = i+1){
        fib[i] = fib[i-1]+fib[i-2];
    }
    return fib[12];
}
"

assert 24 "
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

assert 3 "
int main(){ 
    int a[15];
    *a = 1;
    *(a+1) = 1;
    *(a+2) = 3;
    return *(a+1+1);
}
"

assert 1 "
int main(){ 
    int a[15];
    *a = 1;
    *(a+1) = 1;
    *(a+2) = 3;
    return *(a+1-1);
}
"

assert 30 "
int a;

int change_a(int x){
    a = x;
    return 1;
}

int main(){ 
    int b;
    int c;
    b = 20;
    c = 10;
    change_a(b+c);
    return a;
}
"

assert 3 "
int main(){ 
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    return x[0] + y;
}
"
assert 11 "
int main(){ 
    char a;
    char b;
    a = 1;
    b = 10;
    return b+a;
}
"

assert 10 "
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
    return rem(x, 100);
}
"

assert 10 "
// コメントのテスト
int main(){ // main
    int a = 10; // assign
    return a;
}
"
assert 1 "
/* ブロックコメントのテスト */
int main(){ 
    int a;
    a = 1;
    /*
    a = 10;
    */
    return a;
}
"

assert 233 "
int fibo[100];
int ok[100];
/**
 * メモ化再帰
 */
int fib(int n){
    if(n == 0) {
        fibo[0] = 1;
        ok[0] = 1;
        return fibo[0];
    }
    if(n == 1) {
        fibo[1] = 1;
        ok[1] = 1;
        return fibo[0];
    }
    if(ok[n] == 1) return fibo[n];
    int ans;
    ans = fib(n-1)+fib(n-2);
    ok[n] = 1;
    fibo[n] = ans;
    return ans;
}

int main() {
    int a[100];
    a[99] = 13;
    return fib(a[99]);
}
"

assert 100 "
// asciiで'a'は97
int main() {
    char *c;
    c = \"abcd\";    
    return c[3];
}
"

assert 97 "
// asciiで'a'は97
int main() {
    char *c;
    c = \"abcd\";    
    return c[0];
}
"

echo OK