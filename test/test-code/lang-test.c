int main(int argc, char* argv[]) {
    int a = 0;
    while (a) {
        a = a + 1;
        if (a) {
            break;
        }
    }
    return a;
}