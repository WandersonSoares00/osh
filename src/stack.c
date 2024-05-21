
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include "inc/stack.h"

/**
 *  This file implements a generic stack
 *
 */

Stack *stack_init (int size, void (*dealloc_member)(void*)) {
    Stack *s = malloc(sizeof(Stack));
    if (!s)     return NULL;
    s->size = size;
    s->top = -1;
    s->data = malloc(sizeof(void*) * size);
    if (!s->data) {
        free (s); return NULL;
    }

    for (void **i = s->data, *end = i+size; i!=end; ++i) {
        *i = NULL;
    }

    s->dealloc_member = dealloc_member;
    return s;
}

inline int stack_is_empty (Stack *s) {
    return s->top == -1;
}

inline int stack_is_full (Stack *s) {
    return s->top == s->size-1;
}

int stack_push (Stack *s, void *element) {
    if (stack_is_full(s)) {
        return 0;
    }
    ++s->top;
    
    if (element == s->data[s->top])
        return 1;

    if (s->data[s->top]) {
        (*s->dealloc_member)(s->data[s->top]);
    }
    s->data[s->top] = element;
    return 1;
}

void *stack_top(Stack *s) {
    if (stack_is_empty(s))
        return NULL;
    return s->data[s->top];
}

void *stack_get_cached_element(Stack *s) {
    if (stack_is_full(s))
        return NULL;
    return s->data[s->top+1];
}

void *stack_pop(Stack *s) {
    if (stack_is_empty(s)){
        return NULL;
    }
    --s->top;
    return s->data[s->top+1];
}

void stack_free (Stack *s) {
    for (int i = 0; i < s->size; ++i) {
        if (s->data[i]) {
            (*s->dealloc_member)(s->data[i]);
        }
    }
    free (s->data);
    free (s);
}

