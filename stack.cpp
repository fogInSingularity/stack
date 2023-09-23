#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include <malloc.h>
#include <memory.h>

#include "stack.h"

static const size_t StandardAllocSize = 8;
static const ssize_t MultiplyConst = 2;
static const uint16_t Nullifier = 0;

//---------------------------------------------------------

static StackError StackRecalloc(Stack* stk);

//---------------------------------------------------------

StackError StackCtor(Stack* stk) {
    assert(stk != nullptr);

    StackError error = 0;

    stk->size = 0;
    stk->capasity = StandardAllocSize;

    stk->data = (Elem_t*)calloc(StandardAllocSize, sizeof(Elem_t));

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
    } else {
        if (stk->data == nullptr) {
            error |= StackState::ErrorNotOk;
        } else {
            if (stk->capasity < 0 || stk->size < 0) {
                error |= StackState::ErrorNotOk;
            } else if (stk->capasity < stk->size) {
                error |= StackState::ErrorNotOk;
            } else {
                for (ssize_t i = stk->size; i < stk->capasity; i++) {
                    if (stk->data[i] != Nullifier) {
                        error |= StackState::ErrorNotOk;
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

    stk->data[stk->size++] = value;

    STACKASRT(stk);
    return error;
}

StackError StackPop(Stack* stk, Elem_t* retValue) {
    STACKASRT(stk);

    StackError error = 0;

    if (stk->size <= 0) {
        error |= StackState::ErrorPopNoEl;
    }

    *retValue = stk->data[--stk->size];
    stk->data[stk->size] = 0;

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

    printf("#    size = %ld\n", stk->size);
    printf("#    capasity = %ld\n", stk->capasity);

    printf("#    data[ %p ]\n", stk->data);
    printf("#    {\n");


    for (ssize_t i = 0; i < stk->size; i++) {
        printf("#      *[%ld] %d\n", i, stk->data[i]);
    }

    for (ssize_t i = stk->size; i < stk->capasity; i++) {
        printf("#       [%ld] %d\n", i, stk->data[i]);
    }

    printf("#    }\n");
    printf("#  }\n");
    printf("\n");

    return error;
}

//---------------------------------------------------------------

static StackError StackRecalloc(Stack* stk) {
    // STACKASRT(stk);

    StackError error = 0;

    Elem_t* holdPtr = stk->data;
    stk->data = (Elem_t*)realloc(stk->data, (size_t)stk->capasity*sizeof(Elem_t));

    if (stk->data == nullptr) {
        stk->data = holdPtr;

        error |= StackState::ErrorRealloc;
    } else {
        memset(stk->data + stk->size, Nullifier, (size_t)(stk->capasity - stk->size)*sizeof(Elem_t));
    }

    holdPtr = nullptr;

    STACKASRT(stk);
    return error;
}
