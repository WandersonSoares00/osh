#ifndef STACK_H
#define STACK_H

#include <stddef.h>

typedef struct {
    void **data;
    int top;
    int size;
    void (*dealloc_member)(void*);
} Stack;

Stack *stack_init(int size, void (*dealloc_member)(void*));

int stack_is_empty(Stack *s);

int stack_is_full(Stack *s);

int stack_push(Stack *s, void *element);

void *stack_top(Stack *s);

void *stack_get_cached_element(Stack *s);

void *stack_pop(Stack *s);

void stack_free(Stack *s);

#endif
