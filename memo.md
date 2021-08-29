アセンブリでの関数呼び出しの例

```s
.intel_syntax noprefix
.globl  add_
add_:
    push rbp
    mov rbp, rsp
    mov rax, [rbp+24] # 第一引数はここらしい
    mov rdi, [rbp+16] # 第二引数はここらしい
    add rax, rdi
    mov rsp, rbp
    pop rbp
    ret
.globl main
main:
    push rbp
    mov rbp, rsp
    push 23
    push 2
    push 5
    call add_
    add rsp, 16 # 関数を読んだ分スタックポインタを戻す
    mov rax, [rsp] # raxには23があるらしい(関数呼び出しの前にpushしてたところまで戻った)
    mov rsp, rbp
    pop rbp
    ret

```

アセンブリを出力する

```bash
gcc -fno-asynchronous-unwind-tables -masm=intel -S test.c
```

## やりたいこと
- [ ] 関数の呼び出しの際の引数チェック

```c
int y;
y = int z;
```
みたいのが許されている

```c
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
    x = fib[12];
    return rem(x, 100);
}

```
が終わらない。8byte分読んでいるからのはず。

- `lea`命令ってなんだ？

```c
int a;
int b;

int main(){ 
    a = 20;
    b = 10;
    return a+b;
}
```
で`10`になる。これも8byte書き換えちゃうから。