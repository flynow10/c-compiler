#include <stdio.h>

void recurse(const int i) {
    if (i == 0xfffff) {
        printf("Yay!\n");
        return;
    }
    recurse(1 + i);
}

int main() {
    recurse(0);
}