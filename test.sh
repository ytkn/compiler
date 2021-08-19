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

assert 2 "12+34+56-100;"
assert 72 "12*8-4*6;"
assert 3 "5+2*3-4*2;"
assert 9 "-5+2*3+4*2;"
assert 4 "(12*8)/(4*6);"
assert 2 "12+34+56-100;"
assert 0 "11 + 11 - 22;"
assert 41 " 12 + 34 - 5 ;"
assert 0 "0 == 1;"
assert 1 "0 != 1;"
assert 1 "0 < 1;"
assert 1 "0 <= 1;"
assert 0 "0 > 1;"
assert 0 "0 >= 1;"

assert 1 "2*3+4 == 10-3+5*2-7;"
assert 1 "2*3+4 < 10-3+5*2;"
assert 1 "48/4 == 10-3+10/2;"
assert 13 "a=1; b = 12; a+b;"
assert 1 "a=1; b = 12; a*b-11;"
assert 190 "aa=10; Aba=20; x=10; aa*Aba-x;"
assert 96  "ten=10; two=2; ret=ten*two; return (ret+12)*3;"
assert 96 "ten=10; two=2; ret=ten*two; return (ret+12)*3; x=ten+two; return 100;"
assert 25  "one=1; two=2; aa=one+two; return (aa+12)/3+20;"
assert 33 "a=1; if(a!=1) return 10; else a=a+1; if(a==2) return 33;"
assert 10 "a=1; if(a==1) return 10; else a=a+1; if(a==2) return 33;"
assert 11 "a=1; for(b=0; b < 10; b=b+1) a=a+1; return a;"
assert 6 "a=1; for(b=1; b <= 10; b=b+2) a=a+1; return a;"
assert 12 "a=0; while(a <= 10) a=a+3; return a;"
assert 11 "a=0; while(a <= 10) a=a+1; return a;"
assert 100 "a=0; for(b=0; b < 10; b=b+1) for(c=0; c < 10; c=c+1) a=a+1; return a;"

echo OK