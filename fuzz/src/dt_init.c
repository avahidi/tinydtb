
#include <stdlib.h>

#include "tinydtb.h"

int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size) {
    struct dt_context ctx;
    dt_init(&ctx, (void *)Data, Size);
    return 0;
}





