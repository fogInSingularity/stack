#include "stack.h"

//NOTE - StackOk
//NOTE - fill vect
//NOTE - hash

static const ssize_t MultiplyConst = 2;
static const uint8_t Nullifier = 0;

#ifdef CANARY_PROT
static const canary_t Canary = 0xDEADFA11;
#endif // CANARY_PROT

//---------------------------------------------------------

static StackError StackRecalloc(Stack* stk);

#ifdef CANARY_PROT
static inline canary_t* AdrLCanary(Stack* stk);
static inline canary_t* AdrRCanary(Stack* stk);
#endif // CANARY_PROT

static inline elem_t* AdrDataElem(Stack* stk, ssize_t index);

//---------------------------------------------------------

StackError StackCtor(Stack* stk, ssize_t initCap) {
    ASSERT(stk != nullptr);

    StackError error = 0;

    #ifdef CANARY_PROT
    stk->lcanary = Canary;
    stk->rcanary = Canary;
    #endif // CANARY_PROT

    stk->size = 0;
    stk->capacity = (initCap < StandardAllocSize) ? StandardAllocSize : initCap;

    #ifdef CANARY_PROT
    stk->data = (elem_t*)calloc(sizeof(elem_t)*(size_t)stk->capacity + 2*(sizeof(canary_t)), sizeof(char));
    #else
    stk->data = (elem_t*)calloc(sizeof(elem_t)*(size_t)stk->capacity, sizeof(char));
    #endif // CANARY_PROT

    if (stk->data == nullptr) {
        error |= StackState::ErrorCtor;
        return error;
    }

    #ifdef CANARY_PROT
    memcpy(AdrLCanary(stk), &Canary, sizeof(canary_t)); // bytes misalligment fix
    memcpy(AdrRCanary(stk), &Canary, sizeof(canary_t));
    #endif // CANARY_PROT

    return error;
}

StackError StackDtor(Stack* stk) {
    ASSERT(stk != nullptr);

    StackError error = 0;

    free(stk->data);
    stk->data = nullptr;

    stk->size = 0;
    stk->capacity = 0;

    return error;
}

StackError StackOk(Stack* stk) { //FIXME - shhiiiit
    StackError error = 0;

    (void)stk;

    return error;
}

StackError StackPush(Stack* stk, elem_t value) {
    STACK_ASSERT(stk);

    StackError error = 0;

    if (stk->capacity <= stk->size) {
        stk->capacity *= MultiplyConst;
        error |= StackRecalloc(stk);

        if (error & StackState::ErrorRecalloc) {
            return error;
        }
    }

    *(AdrDataElem(stk, stk->size++)) = value;

    STACK_ASSERT(stk);
    return error;
}

StackError StackPop(Stack* stk, elem_t* retValue) {
    STACK_ASSERT(stk);

    StackError error = 0;

    if (stk->size <= 0) {
        error |= StackState::ErrorPopNoEl;
        return error;
    } else {
        *retValue = *AdrDataElem(stk, --stk->size);
        *AdrDataElem(stk, stk->size) = Nullifier;
    }

    if (stk->size * MultiplyConst * MultiplyConst <= stk->capacity) {
        stk->capacity /= MultiplyConst;

        error |= StackRecalloc(stk);
        if (error & StackState::ErrorRecalloc) {
            return error;
        }
    }

    STACK_ASSERT(stk);
    return error;
}

StackError StackDump(Stack* stk, const char* file, size_t line, const char* func) {
    ASSERT(stk != nullptr);

    StackError error = 0;

    printf("\n");
    printf("#  Stack[ %p ] called from file %s(%lu) from function %s\n", stk, file, line, func);
    printf("#  {\n");

    #ifdef CANARY_PROT
    printf("#    lcanary 0x%lX\n", stk->lcanary);
    #endif // CANARY_PROT
    printf("#    size %ld\n", stk->size);
    printf("#    capacity %ld\n", stk->capacity);

    printf("#    data[ %p ]\n", stk->data);
    printf("#    {\n");

        #ifdef CANARY_PROT
        printf("#      lcanary 0x%lX\n", *(canary_t*)stk->data);
        #endif // CANARY_PROT
    for (ssize_t i = 0; i < stk->size; i++) {
        printf("#     *[%ld] %d\n", i, *AdrDataElem(stk, i));
    }

    for (ssize_t i = stk->size; i < stk->capacity; i++) {
        printf("#      [%ld] %d\n", i, *AdrDataElem(stk, i));
    }
        #ifdef CANARY_PROT
        printf("#      rcanary 0x%lX\n", *AdrRCanary(stk));
        #endif // CANARY_PROT

    printf("#    }\n");

    #ifdef CANARY_PROT
    printf("#    rcanary 0x%lX\n", stk->rcanary);
    #endif // CANARY_PROT

    printf("#  }\n");
    printf("\n");

    return error;
}

//---------------------------------------------------------------

static StackError StackRecalloc(Stack* stk) {
    StackError error = 0;

    elem_t* holdPtr = stk->data;
    #ifdef CANARY_PROT
    stk->data = (elem_t*)realloc(stk->data, 2*sizeof(canary_t) + (size_t)stk->capacity*sizeof(elem_t));
    #else
    stk->data = (elem_t*)realloc(stk->data, (size_t)stk->capacity*sizeof(elem_t));
    #endif // CANARY_PROT

    if (stk->data == nullptr) {
        stk->data = holdPtr;

        error |= StackState::ErrorRecalloc;
    } else {
        memset(AdrDataElem(stk, stk->size), Nullifier, (size_t)(stk->capacity - stk->size)*sizeof(elem_t));
        #ifdef CANARY_PROT
        memcpy(AdrRCanary(stk), &Canary, sizeof(canary_t));
        #endif // CANARY_PROT
    }

    holdPtr = nullptr;

    STACK_ASSERT(stk);
    return error;
}

#ifdef CANARY_PROT
static inline canary_t* AdrLCanary(Stack* stk) {
    return (canary_t*)stk->data;
}

static inline canary_t* AdrRCanary(Stack* stk) {
    return (canary_t*)((char*)stk->data + (size_t)stk->capacity*sizeof(elem_t) + sizeof(canary_t));
}
#endif // CANARY_PROT

static inline elem_t* AdrDataElem(Stack* stk, ssize_t index) {
    #ifdef CANARY_PROT
    return (elem_t*)((char*)stk->data + sizeof(canary_t) + (size_t)index*sizeof(elem_t));
    #else
    return (elem_t*)((char*)stk->data + (size_t)index*sizeof(elem_t));
    #endif // CANARY_PROT
}
