// TU A: Container initialization and push
#include "shared.h"

void container_init(Container* c, int cap) {
    c->capacity = cap;
    c->size = 0;
    c->data = (int*)malloc(cap * sizeof(int));
}

void container_push(Container* c, int val) {
    if (c->size < c->capacity) {
        c->data[c->size++] = val;
    }
}

void container_destroy(Container* c) {
    free(c->data);
    c->data = nullptr;
    c->size = 0;
    c->capacity = 0;
}
