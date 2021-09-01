int rem(int a, int b){
    int c = a/b;
    return a-b*c;
}

int main(){
    return rem(25, 11);
}