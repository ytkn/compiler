#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "9cc.h"

bool ends_with(const char *str, const char *suffix) {
    size_t len1 = strlen(str);
    size_t len2 = strlen(suffix);
    return len1 >= len2 && strcmp(str + len1 - len2, suffix) == 0;
}

char *read_file(char *path) {
    FILE *fp = fopen(path, "r");
    if (!fp) {
        error("cannot open %s: %s", path, strerror(errno));
    }
    if (fseek(fp, 0, SEEK_END) == -1) error("%s: fseek: %s", path, strerror(errno));
    size_t size = ftell(fp);
    if (fseek(fp, 0, SEEK_SET) == -1) error("%s: fseek: %s", path, strerror(errno));

    char *buf = calloc(1, size + 2);
    fread(buf, size, 1, fp);

    // ファイルが必ず"\n\0"で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

void gen_prepared_funcs(){
    prepared_funcs = create_vector();
    Function *printf_func = calloc(1, sizeof(Function));
    printf_func->name = "printf";
    printf_func->name_len = 6;
    push_vector(prepared_funcs, printf_func);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の個数が正しくありません\n");
        return 1;
    }
    if (ends_with(argv[1], ".c")) {
        fprintf(stderr, "read file: %s\n", argv[1]);
        user_input = read_file(argv[1]);
    } else {
        user_input = argv[1];
    }
    token = tokenize();
    gen_prepared_funcs();
    program();
    printf(".intel_syntax noprefix\n");
    if(prog->literals->size > 0){
        printf("    .section	.rodata\n");
    }
    for (int i = 0; i < prog->literals->size; i++) {
        gen_literal_def(prog->literals->data[i]);
    }
    for (int i = 0; i < prog->globals->size; i++) {
        gen_global_def(prog->globals->data[i]);
    }
    for (int i = 0; i < prog->funcs->size; i++) {
        gen_func(prog->funcs->data[i]);
    }
    return 0;
}