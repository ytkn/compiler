# Compiler

[低レイヤを知りたい人のためのCコンパイラ作成入門](https://www.sigbus.info/compilerbook)の実装。

```c
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

int main(){
    int x;
    int i = 0;
    for(i = 0; i < 40; i=i+1){
        x = fib(i);
        printf("fib[%d] = %d\n", i, x);
    }
    return 0;
}
```

で

```
fib[0] = 1
fib[1] = 1
fib[2] = 2
fib[3] = 3
fib[4] = 5
fib[5] = 8
fib[6] = 13
fib[7] = 21
fib[8] = 34
fib[9] = 55
fib[10] = 89
fib[11] = 144
fib[12] = 233
fib[13] = 377
fib[14] = 610
fib[15] = 987
fib[16] = 1597
fib[17] = 2584
fib[18] = 4181
fib[19] = 6765
fib[20] = 10946
fib[21] = 17711
fib[22] = 28657
fib[23] = 46368
fib[24] = 75025
fib[25] = 121393
fib[26] = 196418
fib[27] = 317811
fib[28] = 514229
fib[29] = 832040
fib[30] = 1346269
fib[31] = 2178309
fib[32] = 3524578
fib[33] = 5702887
fib[34] = 9227465
fib[35] = 14930352
fib[36] = 24157817
fib[37] = 39088169
fib[38] = 63245986
fib[39] = 102334155
```

を出力させるまでは出来た。