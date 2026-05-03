struct A {
  int a;
};

int main(int argc, char *argv[]) {
  // printf("Fibonacci numbers:\n");
  struct A *ptr;
  int a = 0;
  int b = 1;
  for (int i = 0; i < 10; i++) {
    const int c = b;
    b = a + b;
    a = c;
    // printf("%i\n", b);
  }
}