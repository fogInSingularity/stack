#ifndef STACK_H
#define STACK_H

#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include <malloc.h>
#include <memory.h>

// #define DEBUG
// #define CANARY_PROT

typedef int elem_t;
const ssize_t StandardAllocSize = 8;

#ifdef CANARY_PROT
typedef uint64_t canary_t;
#endif // CANARY_PROT

struct Stack {
    #ifdef CANARY_PROT
    canary_t lcanary;
    #endif // CANARY_PROT

    elem_t* data;
    ssize_t size;
    ssize_t capacity;

    #ifdef CANARY_PROT
    canary_t rcanary;
    #endif // CANARY_PROT
};


typedef size_t StackError;

enum StackState {
    Success       = 0,
    ErrorNotOk    = 1 << 0,
    ErrorCtor     = 1 << 1,
    ErrorDtor     = 1 << 2,
    ErrorRecalloc = 1 << 3,
    ErrorPopNoEl  = 1 << 4,
};


#define STACK_DUMP(stk) StackDump(stk, __FILE__, __LINE__, __func__)

#ifdef DEBUG
    #define $        fprintf(stderr, ">>> %s (%d) %s\n",__FILE__, __LINE__, __func__);
    #define $$(...) {fprintf(stderr, ">>> %s\n", #__VA_ARGS__); __VA_ARGS__;}

    #define STACK_ASSERT(stk) \
        if (StackOk(stk) & StackState::ErrorNotOk) { \
            STACK_DUMP(stk); \
            abort(); \
        }

    #define  ASSERT(expr) \
        if ((expr) == 0) { \
            printf("# ERROR:\n"); \
            printf("#   file: %s\n", __FILE__); \
            printf("#   line: %d\n", __LINE__); \
            printf("#   func: %s\n", __func__); \
            abort(); \
        }
#else
    #define $ ;
    #define $$(...) ;

    #define STACK_ASSERT(stk) ;
    #define ASSERT(expr) ;
#endif

StackError StackCtor(Stack* stk, ssize_t initCap = StandardAllocSize);

StackError StackDtor(Stack* stk);

StackError StackOk(Stack* stk);

StackError StackPush(Stack* stk, elem_t value);

StackError StackPop(Stack* stk, elem_t* retValue);

StackError StackDump(Stack* stk, const char* file, size_t line, const char* func);

#endif // STACK_H
