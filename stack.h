#ifndef STACK_H
#define STACK_H

#include <stddef.h>


typedef int Elem_t;

struct Stack {
    Elem_t* data;
    size_t size;
    size_t capasity;
};


typedef size_t StackError;

enum StackState {
    Succes       = 0,
    ErrorCtor    = 1 << 0,
    ErrorDtor    = 1 << 1,
    ErrorRealloc = 1 << 2,
    ErrorPopNoEl = 1 << 3,
};


StackError StackCtor(Stack* stk);

StackError StackDtor(Stack* stk);

StackError StackPush(Stack* stk, Elem_t value);

StackError StackPop(Stack* stk, Elem_t* retValue);

#endif
