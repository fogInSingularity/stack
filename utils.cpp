#include "utils.h"

void FillBytes(void* dest, const void* src, size_t nElem, size_t sizeElem) {
    ASSERT(dest != nullptr);
    ASSERT(src != nullptr);

    for (size_t i = 0; i < nElem; i++) {
        memcpy((uint8_t*)dest + i * sizeElem, src, sizeElem);
    }
}
