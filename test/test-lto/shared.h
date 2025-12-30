// Shared header for LTO test
#ifndef SHARED_H
#define SHARED_H

#include <stdlib.h>

// A simple container type used across multiple TUs
struct Container {
    int* data;
    int size;
    int capacity;
};

// Forward declarations
void container_init(Container* c, int cap);
void container_push(Container* c, int val);
int container_get(Container* c, int idx);
void container_destroy(Container* c);

#endif // SHARED_H
