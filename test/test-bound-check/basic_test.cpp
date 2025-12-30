#include <stdlib.h>

struct ArrayContainer {
    int* data;
    int size;
    int capacity;
};

struct SimpleBuffer {
    char* buffer;
};

void init(ArrayContainer* ac, int n) {
    ac->size = n;
    ac->data = (int*)malloc(n * sizeof(int));
}

void access_element(ArrayContainer* ac, int i) {
    if (i < ac->size) {
        ac->data[i] = 42;
    }
}

void no_bounds_check(ArrayContainer* ac, int i) {
    ac->data[i] = 100;
}

void init_simple(SimpleBuffer* sb) {
    sb->buffer = (char*)malloc(256);
}

int main() {
    ArrayContainer ac;
    init(&ac, 10);
    access_element(&ac, 5);
    no_bounds_check(&ac, 3);
    
    SimpleBuffer sb;
    init_simple(&sb);
    
    return 0;
}
