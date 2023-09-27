#include "stack.h"

static const ssize_t MultiplyConst = 2;
static const elem_t Nullifier = 0xDEAD;

#ifdef _CANARY_PROT
static const canary_t Canary = 0xDEADFA11;
#endif // _CANARY_PROT

//---------------------------------------------------------

static StackError StackRecalloc(Stack* stk);

#ifdef _HASH_PROT
static StackError StackOkData(Stack* stk);
#endif // _HASH_PROT

#ifdef _CANARY_PROT
static inline canary_t* AdrLCanary(Stack* stk);
static inline canary_t* AdrRCanary(Stack* stk);
#endif // _CANARY_PROT

static inline elem_t* AdrDataElem(Stack* stk, ssize_t index);

//---------------------------------------------------------

StackError StackCtor(Stack* stk, ssize_t initCap) {
    ASSERT(stk != nullptr);

    StackError error = 0;

    #ifdef _CANARY_PROT
    stk->lcanary = Canary;
    stk->rcanary = Canary;
    #endif // _CANARY_PROT

    stk->size = 0;
    stk->capacity = (initCap < StandardAllocSize) ? StandardAllocSize : initCap;

    #ifdef _CANARY_PROT
    stk->data = (elem_t*)calloc(sizeof(elem_t)*(size_t)stk->capacity + 2*(sizeof(canary_t)), sizeof(char));
    FillBytes(AdrDataElem(stk, 0), &Nullifier, (size_t)stk->capacity, sizeof(elem_t));
    #else
    stk->data = (elem_t*)calloc(sizeof(elem_t)*(size_t)stk->capacity, sizeof(char));
    FillBytes(AdrDataElem(stk, 0), &Nullifier, (size_t)stk->capacity, sizeof(elem_t));
    #endif // _CANARY_PROT

    if (stk->data == nullptr) {
        error |= StackState::ErrorCtor;
        return error;
    }

    #ifdef _CANARY_PROT
    memcpy(AdrLCanary(stk), &Canary, sizeof(canary_t)); // bytes misalligment fix
    memcpy(AdrRCanary(stk), &Canary, sizeof(canary_t));
    #endif // _CANARY_PROT

    #ifdef _HASH_PROT
    uint32_t holdHash = 0;
    stk->stackHash = 0;
    stk->dataHash = 0;

    #ifdef _CANARY_PROT
    holdHash = Hash((const uint8_t*)stk->data, sizeof(elem_t)*(size_t)stk->capacity + 2*sizeof(canary_t), 0);
    #else
    holdHash = Hash((const uint8_t*)stk->data, sizeof(elem_t)*(size_t)stk->capacity, 0);
    #endif // _CANARY_PROT
    stk->dataHash = holdHash;

    holdHash = Hash((const uint8_t*)stk, sizeof(Stack), 0);
    stk->stackHash = holdHash;
    holdHash = 0;

    #endif // _HASH_PROT

    return error;
}

StackError StackDtor(Stack* stk) {
    ASSERT(stk != nullptr);

    StackError error = 0;

    free(stk->data);
    stk->data = nullptr;

    #ifdef _HASH_PROT
    stk->stackHash = 0;
    stk->dataHash = 0;
    #endif

    stk->size = 0;
    stk->capacity = 0;

    return error;
}

StackError StackOk(Stack* stk) { //FIXME - shhiiiit
    StackError error = 0;

    if (stk == nullptr) {
        error |= StackState::ErrorStkNull;
        return error;
    } else {
        #ifdef _CANARY_PROT
        if (stk->lcanary != Canary) {
            error |= StackState::ErrorStkLCanary;
            return error;
        } else if (stk->rcanary != Canary) {
            error |= StackState::ErrorStkRCanary;
            return error;
        }
        #endif // _CANARY_PROT

        if (stk->size < 0) {
            error |= StackState::ErrorStkSizeNeg;
            return error;
        }

        if (stk->capacity < 0) {
            error |= StackState::ErrorStkCapNeg;
            return error;
        }

        #ifdef _HASH_PROT
// printf("stk hash %u\n", stk->stackHash);
        uint32_t holdHash = stk->stackHash;
        stk->stackHash = 0;
        uint32_t hashOk = Hash((const uint8_t*)stk, sizeof(Stack), 0);
// printf("hash ok %u\n", hashOk);
        stk->stackHash = holdHash;
        if (stk->stackHash != hashOk) {
            error |= StackState::ErrorStkHash;
            return error;
        }
        #endif // _HASH_PROT

        if (stk->data == nullptr) {
            error |= StackState::ErrorDataNull;
            return error;
        } else {
            error |= StackOkData(stk);
            if (error != StackState::Success) {
                return error;
            }
        }
    }

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

    #ifdef _HASH_PROT
    uint32_t holdHash = 0;
    stk->stackHash = 0;
    stk->dataHash = 0;

    #ifdef _CANARY_PROT
    holdHash = Hash((const uint8_t*)stk->data, sizeof(elem_t)*(size_t)stk->capacity + 2*sizeof(canary_t), 0);
    #else
    holdHash = Hash((const uint8_t*)stk->data, sizeof(elem_t)*(size_t)stk->capacity, 0);
    #endif // _CANARY_PROT
    stk->dataHash = holdHash;

    holdHash = Hash((const uint8_t*)stk, sizeof(Stack), 0);
    stk->stackHash = holdHash;
    holdHash = 0;
    #endif // _HASH_PROT

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

    #ifdef _HASH_PROT
    uint32_t holdHash = 0;
    stk->stackHash = 0;
    stk->dataHash = 0;

    #ifdef _CANARY_PROT
    holdHash = Hash((const uint8_t*)stk->data, sizeof(elem_t)*(size_t)stk->capacity + 2*sizeof(canary_t), 0);
    #else
    holdHash = Hash((const uint8_t*)stk->data, sizeof(elem_t)*(size_t)stk->capacity, 0);
    #endif // _CANARY_PROT
    stk->dataHash = holdHash;

    holdHash = Hash((const uint8_t*)stk, sizeof(Stack), 0);
    stk->stackHash = holdHash;
    holdHash = 0;
    #endif // _HASH_PROT

    STACK_ASSERT(stk);
    return error;
}

void StackDump(Stack* stk, StackError error, const char* file, size_t line, const char* func) {
    ASSERT(stk != nullptr);

    printf("\n");
    printf("#  Stack[ %p ] called from file %s(%lu) from function %s\n", stk, file, line, func);
    printf("#  Error: %lu\n", error);
    printf("#  {\n");

    #ifdef _CANARY_PROT
    printf("#    lcanary 0x%lX\n", stk->lcanary);
    #endif // _CANARY_PROT
    printf("#    size %ld\n", stk->size);
    printf("#    capacity %ld\n", stk->capacity);
    #ifdef _HASH_PROT
    printf("#    stackHash %u\n", stk->stackHash);
    printf("#    dataHash %u\n", stk->dataHash);
    #endif // _HASH_PROT

    printf("#    data[ %p ]\n", stk->data);
    printf("#    {\n");

        #ifdef _CANARY_PROT
        printf("#      lcanary 0x%lX\n", *(canary_t*)stk->data);
        #endif // _CANARY_PROT
    for (ssize_t i = 0; i < stk->size; i++) {
        printf("#     *[%ld] %d\n", i, *AdrDataElem(stk, i));
    }

    for (ssize_t i = stk->size; i < stk->capacity; i++) {
        printf("#      [%ld] 0x%X\n", i, *(unsigned int*)AdrDataElem(stk, i));
    }
        #ifdef _CANARY_PROT
        printf("#      rcanary 0x%lX\n", *AdrRCanary(stk));
        #endif // _CANARY_PROT

    printf("#    }\n");

    #ifdef _CANARY_PROT
    printf("#    rcanary 0x%lX\n", stk->rcanary);
    #endif // _CANARY_PROT

    printf("#  }\n");
    printf("\n");
}

//---------------------------------------------------------------

static StackError StackRecalloc(Stack* stk) {
    ASSERT(stk != nullptr);

    StackError error = 0;

    elem_t* holdPtr = stk->data;
    #ifdef _CANARY_PROT
    stk->data = (elem_t*)realloc(stk->data, 2*sizeof(canary_t) + (size_t)stk->capacity*sizeof(elem_t));
    #else
    stk->data = (elem_t*)realloc(stk->data, (size_t)stk->capacity*sizeof(elem_t));
    #endif // _CANARY_PROT

    if (stk->data == nullptr) {
        stk->data = holdPtr;

        error |= StackState::ErrorRecalloc;
    } else {
        FillBytes(AdrDataElem(stk, stk->size), &Nullifier, (size_t)(stk->capacity - stk->size), sizeof(elem_t));
        #ifdef _CANARY_PROT
        memcpy(AdrRCanary(stk), &Canary, sizeof(canary_t));
        #endif // _CANARY_PROT
    }

    holdPtr = nullptr;

    return error;
}

static StackError StackOkData(Stack* stk) {
    ASSERT(stk != nullptr);

    StackError error = 0;

    #ifdef _CANARY_PROT
    if (*AdrLCanary(stk) != Canary) {
        error |= StackState::ErrorDataLCanary;
        return error;
    } else if (*AdrRCanary(stk) != Canary) {
        error |= StackState::ErrorDataRCanary;
        return error;
    }
    #endif

    #ifdef _HASH_PROT
    uint32_t hashOkdata = 0;

    #ifdef _CANARY_PROT
    hashOkdata = Hash((const uint8_t*)stk->data, 2*sizeof(canary_t) + (size_t)stk->capacity*sizeof(elem_t), 0);
    #else
    hashOkdata = Hash((const uint8_t*)stk->data, (size_t)stk->capacity*sizeof(elem_t), 0);
    #endif // _CANARY_PROT
    if (hashOkdata != stk->dataHash) {
        error |= StackState::ErrorDataHash;
        return error;
    }
    #endif // _HASH_PROT

    return error;
}

#ifdef _CANARY_PROT
static inline canary_t* AdrLCanary(Stack* stk) {
    ASSERT(stk != nullptr);

    return (canary_t*)stk->data;
}

static inline canary_t* AdrRCanary(Stack* stk) {
    ASSERT(stk != nullptr);

    return (canary_t*)((char*)stk->data + (size_t)stk->capacity*sizeof(elem_t) + sizeof(canary_t));
}
#endif // _CANARY_PROT

static inline elem_t* AdrDataElem(Stack* stk, ssize_t index) {
    ASSERT(stk != nullptr);

    #ifdef _CANARY_PROT
    return (elem_t*)((char*)stk->data + sizeof(canary_t) + (size_t)index*sizeof(elem_t));
    #else
    return (elem_t*)((char*)stk->data + (size_t)index*sizeof(elem_t));
    #endif // _CANARY_PROT
}
