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