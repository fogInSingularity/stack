#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>
#include <malloc.h>
#include <memory.h>

#include "stack.h"

static const size_t standardAllocSize = 8;
static const size_t MultiplyConst = 2;

//---------------------------------------------------------

static StackError StackRealloc(Stack* stk);

//---------------------------------------------------------

StackError StackCtor(Stack* stk) {
    assert(stk != nullptr); // verify

    StackError error = 0;

    stk->size = 0;
    stk->capasity = standardAllocSize;

    stk->data = (Elem_t*)calloc(standardAllocSize, sizeof(Elem_t));
    if (stk->data == nullptr) {
        error |= StackState::ErrorCtor;
    }

    return error;
}

StackError StackDtor(Stack* stk) {
    assert(stk != nullptr); // verify

    StackError error = 0;

    free(stk->data);
    stk->data = nullptr;

    stk->size = 0;
    stk->capasity = 0;

    return error;
}

StackError StackPush(Stack* stk, Elem_t value) {
    assert(stk != nullptr);

    StackError error = 0;

    if (stk->capasity <= stk->size) { // func
        stk->capasity *= MultiplyConst;

        error |= StackRealloc(stk);
    }

    stk->data[stk->size++] = value;

    return error;
}

StackError StackPop(Stack* stk, Elem_t* retValue) {
    assert(stk != nullptr); // verify
    assert(retValue != nullptr);

    StackError error = 0;

    if (stk->size <= 0) {
        error |= StackState::ErrorPopNoEl;
    }

    *retValue = stk->data[--stk->size];
    stk->data[stk->size] = 0;

    if (stk->size * MultiplyConst * MultiplyConst <= stk->capasity) {
        stk->capasity /= MultiplyConst;

        error |= StackRealloc(stk);
    }

    return error;
}

//---------------------------------------------------------------

StackError StackRealloc(Stack* stk) {
    assert(stk != nullptr); // verify

    StackError error = 0;

    Elem_t* holdPtr = stk->data;
    stk->data = (Elem_t*)realloc(stk->data, stk->capasity);

    if (stk->data == nullptr) {
        stk->data = holdPtr;
        error |= StackState::ErrorRealloc;
    }

    return error;
}
