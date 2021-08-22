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

assert 2 "main(){ return 12+34+56-100; }"
assert 72 "main(){ return 12*8-4*6; }"
assert 3 "main(){ return 5+2*3-4*2; }"
assert 9 "main(){ return -5+2*3+4*2; }"
assert 4 "main(){ return (12*8)/(4*6); }"
assert 2 "main(){ return 12+34+56-100; }"
assert 0 "main(){ return 11 + 11 - 22; }"
assert 41 "main(){ return  12 + 34 - 5 ; }"
assert 0 "main(){ return 0 == 1; }"
assert 1 "main(){ return 0 != 1; }"
assert 1 "main(){ return 0 < 1; }"
assert 1 "main(){ return 0 <= 1; }"
assert 0 "main(){ return 0 > 1; }"
assert 0 "main(){ return 0 >= 1; }"

assert 1 "main(){ return 2*3+4 == 10-3+5*2-7; }"
assert 1 "main(){ return 2*3+4 < 10-3+5*2; }"
assert 1 "main(){ return 48/4 == 10-3+10/2; }"
assert 13 "main(){ a=1; b = 12; return a+b; }"
assert 1 "main(){ a=1; b = 12; return a*b-11; }"
assert 190 "main(){ aa=10; Aba=20; x=10; return aa*Aba-x; }"
assert 96  "main(){ ten=10; two=2; ret=ten*two; return (ret+12)*3; }"
assert 96 "main(){ ten=10; two=2; ret=ten*two; return (ret+12)*3; x=ten+two; return 100; }"
assert 25  "main(){ one=1; two=2; aa=one+two; return (aa+12)/3+20; }"
assert 33 "main(){ a=1; if(a!=1) return 10; else a=a+1; if(a==2) return 33; }"
assert 10 "main(){ a=1; if(a==1) return 10; else a=a+1; if(a==2) return 33; }"
assert 11 "main(){ a=1; for(b=0; b < 10; b=b+1) a=a+1; return a; }"
assert 6 "main(){ a=1; for(b=1; b <= 10; b=b+2) a=a+1; return a; }"
assert 12 "main(){ a=0; while(a <= 10) a=a+3; return a; }"
assert 11 "main(){ a=0; while(a <= 10) a=a+1; return a; }"
assert 100 "main(){ a=0; for(b=0; b < 10; b=b+1) for(c=0; c < 10; c=c+1) a=a+1; return a; }"
assert 42 "
add(a, b){
    return a+b;   
}

main(){ 
    return add(10, 32); 
}
"
assert 89 "
fib(a){ 
    if(a <= 1) return 1;
    b = a-1;
    c = a-2;
    return fib(b)+fib(c);
}

main(){ 
    return fib(10); 
}
"

assert 2 "
gcd(a, b){
    if(a > b) return gcd(b, a);
    if(a == 0) return b;
    c = b-a;
    return gcd(a, c);   
}

main(){ 
    return gcd(10, 32); 
}
"

echo OK