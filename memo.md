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