struct A {
  int a;
};

void print_int(int integer);
void print_string(char string[]);

int main(int argc, char *argv[]) {
  print_string("Fibonacci numbers:\n");
  struct A *ptr;
  int a = 0;
  int b = 1;
  for (int i = 0; i < 10; i++) {
    const int c = b;
    b = a + b;
    a = c;
    print_int(b);
    print_string("\n");
  }
}

void print_int(int integer) {
  __asm__(
    "li a7, 1;"
    "mv a0, %0;"
    "ecall;"
    :
    : "r" (integer)
    : "a0", "a7");
}

void print_string(char string[]) {
  __asm__(
    "li a7, 4;"
    "mv a0, %0;"
    "ecall;"
    :
    : "r" (string)
    : "a0", "a7");
}
