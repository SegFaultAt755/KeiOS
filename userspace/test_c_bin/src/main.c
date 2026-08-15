int main(int argc, int *argv) {
    int total = 0;
    for (int i = 0; i < argc; i++) {
        total += argv[i];
    }

    return total;
}
