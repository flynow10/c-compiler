int factorial(int);

int main() {
    return factorial(10);
}

int factorial(int n) {
    if (n == 0) {
        return 1;
    }

    return n * factorial(n - 1);
}