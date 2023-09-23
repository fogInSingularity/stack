#ifndef STACK_H
#define STACK_H

#include <stddef.h>
#include <stdint.h>

typedef int Elem_t;
typedef uint64_t Canary_t;

struct Stack {
    Canary_t lcanary;

    Elem_t* data;
    ssize_t size;
    ssize_t capasity;

    Canary_t rcanary;
};


typedef size_t StackError;

enum StackState {
    Succes       = 0,
    ErrorNotOk   = 1 << 0,
    ErrorCtor    = 1 << 1,
    ErrorDtor    = 1 << 2,
    ErrorRealloc = 1 << 3,
    ErrorPopNoEl = 1 << 4,
};

#define STACKDUMP(stk) StackDump(stk, __FILE__, __LINE__)

#ifdef DEBUG
    #define STACKASRT(stk) \
        if (StackOk(stk) & StackState::ErrorNotOk) { \
            STACKDUMP(stk); \
            abort(); \
        }
#else
    #define STACKASRT(stk) ;
#endif

StackError StackCtor(Stack* stk);

StackError StackDtor(Stack* stk);

StackError StackOk(Stack* stk);

StackError StackPush(Stack* stk, Elem_t value);

StackError StackPop(Stack* stk, Elem_t* retValue);

StackError StackDump(Stack* stk, const char* file, size_t line);

#endif
