
#ifndef DARRAY_H
#define DARRAY_H

#include <stddef.h>

typedef struct {
    void **data;
    int curr_size;
    int last_data;
    int first_data;
    void (*dealloc_member)(void*);
} Darray;

Darray *darray_init(unsigned int initial_size, void (*dealloc_member)(void*));

void *darray_get(int i, Darray *arr);

int darray_size(Darray *arr);

int darray_assign(int i, void *element, Darray *arr);

int darray_resize(Darray *arr);

int darray_push_back(Darray *arr, void *element);

void *darray_get_front(Darray *arr);

void darray_pop_back(Darray *arr);

void darray_pop_front(Darray *arr);

void darray_free(Darray *arr);

#endif
