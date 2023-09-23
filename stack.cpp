#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include <malloc.h>
#include <memory.h>

#include "stack.h"

#define $         fprintf(stderr, ">>> %s (%d)\n",__FILE__, __LINE__);
#define $$(...) { fprintf(stderr, ">>> %s\n", #__VA_ARGS__); __VA_ARGS__; }

static const size_t StandardAllocSize = 8;
static const ssize_t MultiplyConst = 2;
static const uint8_t Nullifier = 0;
static const Canary_t Canary = 0xDEADDEAD;

//---------------------------------------------------------

static StackError StackRecalloc(Stack* stk);

//---------------------------------------------------------

StackError StackCtor(Stack* stk) {
    assert(stk != nullptr);

    StackError error = 0;

    stk->lcanary = Canary;
    stk->rcanary = Canary;

    stk->size = 0;
    stk->capasity = StandardAllocSize;

    stk->data = (Elem_t*)calloc(sizeof(Elem_t)*StandardAllocSize + 2*(sizeof(Canary_t)), sizeof(char));
    *(Canary_t*)stk->data = Canary;
    *(Canary_t*)((char*)stk->data + (size_t)stk->capasity*sizeof(Elem_t) + sizeof(Canary_t)) = Canary;

    if (stk->data == nullptr) {
        error |= StackState::ErrorCtor;
    }

    return error;
}

StackError StackDtor(Stack* stk) {
    assert(stk != nullptr);

    StackError error = 0;

    free(stk->data);
    stk->data = nullptr;

    stk->size = 0;
    stk->capasity = 0;

    return error;
}

StackError StackOk(Stack* stk) {
    StackError error = 0;

    if (stk == nullptr) {
        error |= StackState::ErrorNotOk;
        // printf("1");
    } else {
        if (stk->lcanary != Canary) {
            error |= StackState::ErrorNotOk;
            // printf("2");
        } else if (stk->rcanary != Canary) {
            error |= StackState::ErrorNotOk;
            // printf("3");
        } else if (stk->data == nullptr) {
            error |= StackState::ErrorNotOk;
            // printf("4");
        } else {
            if (stk->capasity < 0 || stk->size < 0) {
                error |= StackState::ErrorNotOk;
                // printf("5");
            } else if (stk->capasity < stk->size) {
                error |= StackState::ErrorNotOk;
                // printf("6");
            } else {
                for (size_t i = (size_t)stk->size; i < (size_t)stk->capasity; i++) {
                    // printf("%lu\n", i);
                    if (*(Elem_t*)((char*)stk->data + sizeof(Canary_t) + sizeof(Elem_t)*i) != Nullifier) {
                        error |= StackState::ErrorNotOk;
                        // printf("data:   %p\n", stk->data);
                        // printf("canary: %p\n", (char*)stk->data + sizeof(Canary_t));
                        // printf("error:  %p\n", (char*)stk->data + sizeof(Canary_t) + sizeof(Elem_t)*i);
                        // printf("7\n");
                        break;
                    }
                }
            }
        }
    }


    return error;
}

StackError StackPush(Stack* stk, Elem_t value) {
    STACKASRT(stk);

    StackError error = 0;

    if (stk->capasity <= stk->size) {
        stk->capasity *= MultiplyConst;
        error |= StackRecalloc(stk);
    }

    $$(*(Elem_t*)((char*)stk->data + sizeof(Canary_t) + sizeof(Elem_t)*(size_t)(stk->size++)) = value;)

    STACKASRT(stk);
    return error;
}

StackError StackPop(Stack* stk, Elem_t* retValue) {
    STACKASRT(stk);

    StackError error = 0;

    if (stk->size <= 0) {
        error |= StackState::ErrorPopNoEl;
    } else {
        *retValue = *(Elem_t*)((char*)stk->data + sizeof(Canary_t) + sizeof(Elem_t)*(size_t)(--stk->size));
        *((char*)stk->data + sizeof(Canary_t) + sizeof(Elem_t)*(size_t)(stk->size)) = 0;
    }

    if (stk->size * MultiplyConst * MultiplyConst <= stk->capasity) {
        stk->capasity /= MultiplyConst;

        error |= StackRecalloc(stk);
    }

    STACKASRT(stk);
    return error;
}

StackError StackDump(Stack* stk, const char* file, size_t line) {
    StackError error = 0;

    printf("\n");
    printf("#  Stack[ %p ] called from %s(%lu)\n", stk, file, line);
    printf("#  {\n");

    printf("#    lcanary 0x%lX\n", stk->lcanary);
    printf("#    size %ld\n", stk->size);
    printf("#    capasity %ld\n", stk->capasity);

    printf("#    data[ %p ]\n", stk->data);
    printf("#    {\n");

        printf("#      lcanary 0x%lX\n", *(Canary_t*)stk->data);
    for (ssize_t i = 0; i < stk->size; i++) {
        printf("#     *[%ld] %d\n", i, *(Elem_t*)((char*)stk->data + sizeof(Canary_t) + (size_t)i*sizeof(Elem_t)));
    }

    for (ssize_t i = stk->size; i < stk->capasity; i++) {
        printf("#      [%ld] %d\n", i, *(Elem_t*)((char*)stk->data + sizeof(Canary_t) + (size_t)i*sizeof(Elem_t)));
    }
        printf("#      rcanary 0x%lX\n", *(Canary_t*)((char*)stk->data + sizeof(Elem_t)*(size_t)stk->capasity));

    printf("#    }\n");

    printf("#    rcanary 0x%lX\n", stk->rcanary);
    printf("#  }\n");
    printf("\n");

    return error;
}

//---------------------------------------------------------------

static StackError StackRecalloc(Stack* stk) {
    StackError error = 0;

    Elem_t* holdPtr = stk->data;
    stk->data = (Elem_t*)realloc(stk->data,2*sizeof(Canary_t) + (size_t)stk->capasity*sizeof(Elem_t));

    if (stk->data == nullptr) {
        stk->data = holdPtr;

        error |= StackState::ErrorRealloc;
    } else {
        memset((char*)stk->data + (size_t)stk->size*sizeof(Elem_t) + sizeof(Canary_t), Nullifier, (size_t)(stk->capasity - stk->size)*sizeof(Elem_t));
    }

    holdPtr = nullptr;

    STACKASRT(stk);
    return error;
}
