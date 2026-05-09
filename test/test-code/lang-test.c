int main(int argc, char* argv[]) {
    int a = 0;
    int i = 0;
    if (a) {
        while (i) {
            i = 1;
            a = 2;
        }
        if (i) {
            a = 3;
        }
    } else {
        int b = 1;
    }
    return a;
}