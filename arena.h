#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define DA_RESIZE_FACTOR 2

typedef struct 
DynamicArena {
    char* data;
    size_t element_size; 
    size_t element_size_plus; /* element_size_plus includes sizeof(int) to hold pointer */
    size_t len;
    size_t capacity;
    int free_list; /* -1 represents freelist is empty */
} DynamicArena;

DynamicArena da_new_with_capacity(size_t element_size, size_t capacity) {
    size_t element_size_plus = element_size + sizeof(int); 
    void* data = malloc(element_size_plus * capacity);
    DynamicArena da = { .data = data, .element_size = element_size, .element_size_plus = element_size_plus, .len = 0, .capacity = capacity, .free_list = -1 };
    return da;
}

void print_binary(char c) {
    char mask;
    for (int j = 0; j < 8; j++) {
        mask = 128 >> j;
        fprintf(stderr, "%d", (c & mask) > 0);
    }
}

void da_debug(DynamicArena* da) {
    for (int i = 0; i < (da->capacity); i++) {
        for (int j = 0; j < da->element_size_plus; j++) {
            print_binary(da->data[i*da->element_size_plus + j]);
        }
        fprintf(stderr, "\n");
    } 
    fprintf(stderr, "\n");
}

void _da_grow(DynamicArena* da, size_t minimum_size) {
    int new_capacity = da->capacity;
    while (new_capacity <= minimum_size) {
        new_capacity *= DA_RESIZE_FACTOR;
    }
    if ((da->data = realloc(da->data, new_capacity * da->element_size_plus)) == NULL) {
        printf("_da_grow: buy more RAM lol\n");
        exit(1);
    }
    da->capacity = new_capacity;
}

void _da_shrink(DynamicArena* da, size_t minimum_size) {
    int new_capacity = da->capacity;
    while (new_capacity > minimum_size && new_capacity > 1) {
        new_capacity /= DA_RESIZE_FACTOR;
    }
    if ((da->data = realloc(da->data, new_capacity * da->element_size_plus)) == NULL) {
        printf("_da_grow: buy more RAM lol\n");
        exit(1);
    }
    da->capacity = new_capacity;
}

int da_push(DynamicArena* da, void* element) {
    int index;
    if (da->free_list != -1) {
    /* try to allocate with freelist first */
        index = da->free_list; 
        da->free_list = *(int*)(da->data + index * da->element_size_plus);
    } else {
    /* otherwise push at end */
        if (da->len == da->capacity) {  
            _da_grow(da, da->len + 1);
        }
        index = da->len;
        da->len += 1;
    }

    *(int*)(da->data + index * da->element_size_plus) = index;
 
    if (memcpy(
        da->data + sizeof(int) + da->element_size_plus * index, 
        element,
        da->element_size
    ) == NULL) {
        printf("Oops dynamic_arena memcpy");
        exit(1);
    }

    return index;
}

void* da_get(DynamicArena* da, int index) {
    if (
        index >= 0 && index < da->len && /* Bound Check */
        *(int*)(da->data + index * da->element_size_plus) == index /* Valid Address */
    ) {
        return da->data + index * da->element_size_plus + sizeof(int); 
    } 
    return NULL;
}

void da_remove(DynamicArena* da, int index) {  
    *(int*)(da->data + index * da->element_size_plus) = da->free_list;
    da->free_list = index;
    /* QUESTION: Do we need to shrink? */

    /* Other thing we could try: is make int* index so that we can zero it out afterwards the same way that you NULL a pointer after freeing */
}



/*

the relative index for each cell is either
itself: Indicating it is not part of the free list
another: Indicating it is in the middle of the freelist
-1: Indicating it terminates the free list

*/
