int main(int argc, char* argv[]) {
    int a = 0;
    if (a) {
        int b = 1;
    } else {
        int a = 3;
        return a;
    }
    return a;
}