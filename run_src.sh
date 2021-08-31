#!/bin/bash

src="
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


# (aa+12)/3+20;

make
./9cc "$src" > tmp.s
cc -o tmp tmp.s
./tmp
echo $?