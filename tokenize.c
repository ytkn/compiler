#include "9cc.h"

char *get_line_start(char *loc) {
    int n = strlen(user_input);
    char *ret = user_input;
    for (char *p = user_input; p < loc; p++) {
        if (*p == '\n') {
            ret = p + 1;
        }
    }
    return ret;
}

int get_line_len(char *line_start) {
    int n = strlen(user_input);
    for (char *p = line_start; p < user_input + n; p++) {
        if (*p == '\n') {
            return p - line_start;
        }
    }
    return user_input + n - line_start;
}

int get_line_num(char *line_start) {
    int line_num = 1;
    for (char *p = user_input; p < line_start; p++) {
        if (*p == '\n') {
            line_num++;
        }
    }
    return line_num;
}

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

void error_at(char *loc, char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    char *line_start = get_line_start(loc);
    int pos = loc - line_start;
    int line_len = get_line_len(line_start);
    char *line = calloc(line_len + 1, sizeof(char));
    memcpy(line, line_start, line_len);
    fprintf(stderr, "at line %d\n", get_line_num(line_start));
    fprintf(stderr, "%s\n", line);
    fprintf(stderr, "%*s", pos, "");  // print pos spaces.
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
    cur->next = tok;
    return tok;
}

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

bool is_alphabet(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
}

Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;
    while (*p) {
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (startswith(p, "//")) {
            p += 2;
            while (*p != '\n') p++;
            continue;
        }

        if (*p == '"') {
            p++;
            char *q = p;
            while(*q != '"'){
                if(!(*q)) error_at(p, "文字列が閉じられていません");
                q++;
            }
            int len = q-p;
            cur = new_token(TK_LITERAL, cur, p, len);
            p = q+1;
            continue;
        }

        if (startswith(p, "/*")) {
            p += 2;
            char *q = p;
            while(!startswith(q, "*/")){
                if(!(*q)) error_at(p, "コメントが閉じられていません");
                q++;
            }
            p = q + 2;
            continue;
        }

        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        if (startswith(p, "if") && !is_alnum(2)) {
            cur = new_token(TK_IF, cur, p, 2);
            p += 2;
            continue;
        }

        if (startswith(p, "for") && !is_alnum(3)) {
            cur = new_token(TK_FOR, cur, p, 3);
            p += 3;
            continue;
        }

        if (startswith(p, "while") && !is_alnum(5)) {
            cur = new_token(TK_WHILE, cur, p, 5);
            p += 5;
            continue;
        }

        if (startswith(p, "else") && !is_alnum(4)) {
            cur = new_token(TK_ELSE, cur, p, 4);
            p += 4;
            continue;
        }

        if (startswith(p, "return") && !is_alnum(6)) {
            cur = new_token(TK_RETURN, cur, p, 6);
            p += 6;
            continue;
        }

        if (startswith(p, "int") && !is_alnum(3)) {
            cur = new_token(TK_INT, cur, p, 3);
            p += 3;
            continue;
        }

        if (startswith(p, "char") && !is_alnum(4)) {
            cur = new_token(TK_CHAR, cur, p, 4);
            p += 4;
            continue;
        }

        if (startswith(p, "sizeof") && !is_alnum(6)) {
            cur = new_token(TK_SIZEOF, cur, p, 6);
            p += 6;
            continue;
        }

        if (is_alphabet(*p)) {
            int len = 0;
            char *q = p;
            while (is_alnum(*p)) {
                p++;
            }
            cur = new_token(TK_IDENT, cur, q, p - q);
            continue;
        }

        if (strchr("+-*/()<>;={},&[]", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(token->str, "トークナイズ出来ません\n");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}