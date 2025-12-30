#include <stdlib.h>
#include <string.h>

struct Vector {
    int* elements;
    int count;
    int capacity;
};

struct StringBuffer {
    char* data;
    size_t length;
    size_t allocated;
};

struct LinkedNode {
    void* value;
    struct LinkedNode* next;
};

struct HashMap {
    struct LinkedNode** buckets;
    int bucket_count;
    int item_count;
};

// Multiple allocations
void vector_init(Vector* v, int cap) {
    v->capacity = cap;
    v->count = 0;
    v->elements = (int*)malloc(cap * sizeof(int));
}

void string_buffer_init(StringBuffer* sb, size_t initial_size) {
    sb->allocated = initial_size;
    sb->length = 0;
    sb->data = (char*)malloc(initial_size);
}

// Array access patterns
void vector_push(Vector* v, int val) {
    if (v->count < v->capacity) {
        v->elements[v->count++] = val;
    }
}

void vector_set(Vector* v, int idx, int val) {
    v->elements[idx] = val;  // No bounds check
}

int vector_get(Vector* v, int idx) {
    return v->elements[idx];  // Read access
}

void string_buffer_append(StringBuffer* sb, char c) {
    if (sb->length < sb->allocated) {
        sb->data[sb->length++] = c;
    }
}

// Hash map operations  
void hashmap_init(HashMap* hm, int buckets) {
    hm->bucket_count = buckets;
    hm->item_count = 0;
    hm->buckets = (LinkedNode**)calloc(buckets, sizeof(LinkedNode*));
}

void hashmap_put(HashMap* hm, int key, void* value) {
    int idx = key % hm->bucket_count;
    LinkedNode* node = (LinkedNode*)malloc(sizeof(LinkedNode));
    node->value = value;
    node->next = hm->buckets[idx];
    hm->buckets[idx] = node;
    hm->item_count++;
}

// Test all patterns
int main() {
    Vector v;
    vector_init(&v, 100);
    vector_push(&v, 1);
    vector_push(&v, 2);
    vector_set(&v, 0, 10);
    int x = vector_get(&v, 0);
    
    StringBuffer sb;
    string_buffer_init(&sb, 256);
    string_buffer_append(&sb, 'H');
    string_buffer_append(&sb, 'i');
    
    HashMap hm;
    hashmap_init(&hm, 16);
    hashmap_put(&hm, 42, &x);
    
    return 0;
}
