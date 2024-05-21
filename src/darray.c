#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include "inc/darray.h"

/**
 *  This file implements a generic dynamic array
 *
 */

Darray *darray_init(unsigned int initial_size, void (*dealloc_member)(void*)) {
    Darray *darray = (Darray*) malloc(sizeof(Darray));
    if (!darray)    return NULL;
    darray->dealloc_member = dealloc_member;
    void **arr = malloc(sizeof(void*) * initial_size);
    if (!arr) {
        free(darray);   return NULL;
    }
    for (register int i = 0; i < initial_size; ++i)
        arr[i] = NULL;
    darray->data = arr;
    darray->curr_size = initial_size;
    darray->last_data = 0;
    darray->first_data = 0;
    return darray;
}

inline void *darray_get(int i, Darray *arr) {
    if (i < arr->curr_size)
        return arr->data[i];
    return NULL;
}

int darray_assign(int i, void *element, Darray *arr) {
    int idx = i + arr->first_data;
    if (idx < arr->curr_size) {
        arr->data[idx] = element;
        return 0;
    }
    return 1;
}

inline int darray_size(Darray *arr) {
    return (arr->last_data - arr->first_data);
}

int darray_resize(Darray *arr) {
    int arr_size = arr->curr_size;    
    void **new_arr = malloc(sizeof(void*) * arr_size*2);
    if (!new_arr)
        return 1;

    for (register int i = 0; i < arr_size; ++i) {
        new_arr[i] = arr->data[i];
        new_arr[arr_size+i] = NULL;
    }
    free(arr->data);
    arr->data = new_arr;
    arr->curr_size *= 2;
    return 0;
}

int darray_push_back(Darray *arr, void *element) {
    if (arr->last_data < arr->curr_size) {
        arr->data[arr->last_data++] = element;
        return 0;
    }
    if (darray_resize (arr))
        return 1;

    arr->data[arr->last_data++] = element;
    return 0;
}

inline void *darray_get_front(Darray *arr) {
    if (arr->first_data < arr->last_data)
        return arr->data[arr->first_data];
    return NULL;
}

inline void darray_pop_back(Darray *arr) {
    if (arr->last_data > arr->first_data) {
        (*arr->dealloc_member)(arr->data[arr->last_data--]);
    }
}

inline void darray_pop_front(Darray *arr) {
    if (arr->first_data < arr->last_data) {
        (*arr->dealloc_member)(arr->data[arr->first_data++]);
    }
    else {
        arr->first_data = 0;
        arr->last_data  = 0;
    }
}

void darray_free(Darray *arr) {
    if (!arr)
        return;
    for (register int i = 0; i < arr->last_data; ++i) {
        (*arr->dealloc_member)(arr->data[i]);
    }
    if (arr->curr_size)   free(arr->data);
    free(arr);
}

