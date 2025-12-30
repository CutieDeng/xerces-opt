// TU B: Container access
#include "shared.h"

int container_get(Container* c, int idx) {
    if (idx >= 0 && idx < c->size) {
        return c->data[idx];
    }
    return -1;
}

// Usage function
int main() {
    Container c;
    container_init(&c, 100);

    container_push(&c, 42);
    container_push(&c, 100);

    int val = container_get(&c, 0);

    container_destroy(&c);

    return val;
}
