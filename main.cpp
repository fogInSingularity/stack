#include <stdio.h>
#include <stdlib.h>

#include "stack.h"
#include "utils.h"
#include "hash.h"

// stack code exemple
int main() {
    Stack stack = {};
    StackCtor(&stack);

    for (ssize_t i = 0; i < 20; i++) {
        StackPush(&stack, (int)i);
    }

    STACK_DUMP(&stack, 0);

    elem_t tmp = 0;

    for (ssize_t i = 0; i < 16; i++) {
        StackPop(&stack, &tmp);
    }

    STACK_DUMP(&stack, 0);

    StackDtor(&stack);

    return 0;
}
