#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>

#ifdef _DEBUG
#define $        fprintf(stderr, ">>> %s (%d) %s\n",__FILE__, __LINE__, __func__);
#define $$(...) {fprintf(stderr, ">>> %s\n", #__VA_ARGS__); __VA_ARGS__;}

#define ASSERT(expr) \
    if ((expr) == 0) { \
        printf("# ERROR:\n"); \
        printf("#   file: %s\n", __FILE__); \
        printf("#   line: %d\n", __LINE__); \
        printf("#   func: %s\n", __PRETTY_FUNCTION__); \
        abort(); \
    }
#else
#define $ ;
#define $$(...) ;

#define ASSERT(expr) ;
#endif // _DEBUG

void FillBytes(void* dest, const void* src, size_t nElem, size_t sizeElem);

#endif
