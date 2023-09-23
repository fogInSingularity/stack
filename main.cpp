#include <stdio.h>
#include <stdlib.h>

#include "stack.h"

int main() {
    Stack stack = {};
    StackCtor(&stack);

    for (ssize_t i = 0; i < 20; i++) {
        StackPush(&stack, (int)i);
    }

    STACKDUMP(&stack);
    
    Elem_t tmp = 0;

    for (ssize_t i = 0; i < 16; i++) {
        StackPop(&stack, &tmp);
    }

    STACKDUMP(&stack);

    StackDtor(&stack);

    return 0;
}
