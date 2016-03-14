#ifndef __TINYDTB_H__
#define __TINYDTB_H__

#ifndef __ASSEMBLER__

#include <stdint.h>

/* byte-order:
 * 1. little-endian, which is probably your native endian
 * 2. big-endin, used by devicetrees
 */
#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#  define bigend(x) __builtin_bswap32(x)
#  define litend(x) (x)
#elif (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#  define bigend(x) (x)
#  define litend(x) __builtin_bswap32(x)
#else
#  error "Unknown endianness"
#endif

#define dtend bigend

struct dt_block {
    uint32_t token;
    uint32_t start;
    uint32_t end;
    uint32_t data_len;
    uint32_t level;
    char *name;
    union {
        void *ptr;
        uint32_t *num;
        char *str;
    } data;
};

struct dt_context {
    uint32_t *start;
    uint32_t size;
    uint32_t off_stc;
    uint32_t off_str;
    uint32_t size_stc;
    uint32_t size_str;
    struct dt_block root;
};

struct dt_foreach {
    struct dt_context *context;
    uint32_t req_type;
    uint32_t req_level;
    uint32_t curr_level;
    uint32_t curr_offset;
    struct dt_block storage;
};


extern int dt_init(struct dt_context *ctx, void *start, uint32_t size);
extern int dt_block_find(struct dt_context *ctx, struct dt_block *block,
                         struct dt_block *result, int asnode,
                         char *name, int len);
extern void dt_foreach_init(struct dt_context *ctx,
                            struct dt_block *parent,
                            struct dt_foreach *fe,
                            int asnode);
extern struct dt_block *dt_foreach_next(struct dt_foreach *fe);
extern struct dt_block *dt_foreach_next_of(struct dt_foreach *fe, char *prefix);

#endif /* ! __ASSEMBLER__ */

#endif /* __TINYDTB_H__ */
