
#include <stdlib.h>
#include <string.h>

#include "tinydtb.h"

#define DEVICETREE_MAGIC 0xd00dfeed

#define DEVICETREE_HEAD_MAGIC  (4 * 0)
#define DEVICETREE_HEAD_SIZHDR (4 * 1)
#define DEVICETREE_HEAD_OFFSTC (4 * 2)
#define DEVICETREE_HEAD_OFFSTR (4 * 3)
#define DEVICETREE_HEAD_SIZSTC (4 * 9)
#define DEVICETREE_HEAD_SIZSTR (4 * 8)


#define DEVICETREE_TOKEN_NODE_START 1
#define DEVICETREE_TOKEN_NODE_END   2
#define DEVICETREE_TOKEN_PROP       3
#define DEVICETREE_TOKEN_NOP        4
#define DEVICETREE_TOKEN_END        9


/*
 * Raw and nlock operations
 */

static inline int dt_raw_valid(struct dt_context *ctx, int index)
{
    return ((index & 3) != 0 || index < 0 || index >= ctx->size) ? 0 : 1;
}

static inline int dt_str_valid(struct dt_context *ctx, int index)
{
    return (index >= 0 && index < ctx->size_str) ? 1 : 0;
}

static inline int dt_block_valid(struct dt_context *ctx, int index)
{
    return (index >= 0 && index < ctx->size_stc) ? 1 : 0;
}

static inline uint32_t dt_raw_num(struct dt_context *dt, int index)
{
    uint32_t ret;
    if(!dt_raw_valid(dt, index))
        return 0;

    ret = bigend(dt->start[index / sizeof(uint32_t)]);
    return ret;
}

static inline uint32_t dt_block_num(struct dt_context *dt, int index)
{
    if(! dt_block_valid(dt, index))
        return 0;

    return dt_raw_num(dt, dt->off_stc + index);
}

static inline char *dt_block_str(struct dt_context *dt, int index)
{
    char *ret;
    if(! dt_block_valid(dt, index))
        return 0;

    return (char *) &(dt->start[ ( dt->off_stc + index) / sizeof(uint32_t )]);
}

static inline char *dt_str_str(struct dt_context *dt, int index)
{
    if(! dt_str_valid(dt, index))
        return 0;
    return ((char *) dt->start) + (dt->off_str + index); /* not always aligned */
}

static int dt_block_load(struct dt_context *ctx,
                         struct dt_block *block,
                         uint32_t offset)
{
    int len, n;

    block->start = offset;
    block->level = 0;
    block->name = 0;
    block->data_len = 0;
    block->data.ptr = 0;

    if(!dt_block_valid(ctx, offset)) {
        block->token = 0;
        return 0;
    }

    block->token = dt_block_num(ctx, offset);
    offset += 4;

    switch(block->token) {
    case DEVICETREE_TOKEN_NODE_START:
        block->level ++;
        block->name = dt_block_str(ctx, offset);

        /* add one for '\0' and align to next 32-bit... */
        len = (strlen(block->name) + 4) & ~3;
        offset += len;
        break;

    case DEVICETREE_TOKEN_NODE_END:
        block->level --;
        break;

    case DEVICETREE_TOKEN_PROP:
        block->data_len = dt_block_num(ctx, offset);
        n = dt_block_num(ctx, offset + 4);
        block->data.ptr = dt_block_str(ctx, offset + 8);
        len = (block->data_len + 3) & ~3;
        offset += 8 + len;
        if(!dt_str_valid(ctx, n))
            goto error;
        block->name = dt_str_str(ctx, n);
        break;
    case DEVICETREE_TOKEN_NOP:
        break;
    case DEVICETREE_TOKEN_END:
        break;
    default:
        goto error;
    }

    block->end = offset;
    return block->token;

error:
    block->end = offset;
    return 0;
}



/*
 * for each
 */

void dt_foreach_init(struct dt_context *ctx,
                     struct dt_block *parent,
                     struct dt_foreach *fe,
                     int asnode)
{
    if(!parent)
        parent = & ctx->root;

    fe->context = ctx;
    fe->req_type = asnode ? DEVICETREE_TOKEN_NODE_START : DEVICETREE_TOKEN_PROP;
    fe->req_level = asnode ? 1 : 0;
    fe->curr_level = 0;
    fe->curr_offset = parent->end ;
}


struct dt_block *dt_foreach_next(struct dt_foreach *fe)
{
    struct dt_context *ctx;
    struct dt_block *block;
    uint32_t token;

    ctx = fe->context;
    block = &fe->storage;

    for(;;) {
        token = dt_block_load(ctx, block, fe->curr_offset);
        if(token == 0 || token == DEVICETREE_TOKEN_END)
            return 0;

        fe->curr_offset = block->end;
        fe->curr_level += block->level;
        if((int)fe->curr_level < 0)
            return 0;

        if(fe->curr_level == fe->req_level && token == fe->req_type)
            return block;
    }
}


struct dt_block *dt_foreach_next_of(struct dt_foreach *fe, char *prefix)
{
    struct dt_block *ret;
    int len = strlen(prefix);

    for(;;) {
        ret = dt_foreach_next(fe);
        if(!ret || !strncmp(ret->name, prefix, len))
            return ret;
    }
}

int dt_block_find(struct dt_context *ctx, struct dt_block *block,
                      struct dt_block *result,
                      int asnode,
                      char *name, int len)
{
    struct dt_foreach foreach;
    struct dt_block store, *tmp;
    int sublen;
    char *dir;

    while(*name == '/') {
        name++;
        len--;
    }

    if(len <= 0)
        len = strlen(name);

    /* find it's prefix if any */
    while( (dir = strchr(name, '/')) && dir < name + len) {
        sublen = dir - name;
        if(!dt_block_find(ctx, block, &store, 1, name, sublen))
            return 0;

        len -= sublen + 1;
        name = dir + 1;
        block = &store;
    }

    dt_foreach_init(ctx, block, &foreach, asnode);
    while( (tmp = dt_foreach_next(&foreach))) {
        if(!strncmp(tmp->name, name, len) && tmp->name[len] == '\0') {
            *result = *tmp;
            return 1;
        }
    }
    return 0;
}

/*
 * Context operations
 */
int dt_init(struct dt_context *ctx,
            void *start, uint32_t size)
{
    struct dt_block first;
    struct dt_context tmp;
    tmp.start = start;
    tmp.size = size;

    /* sanity check 1 */
    if(dt_raw_num(&tmp, DEVICETREE_HEAD_MAGIC) != DEVICETREE_MAGIC ||
       dt_raw_num(&tmp, DEVICETREE_HEAD_SIZHDR) != size)
        return 0;

    tmp.off_stc  = dt_raw_num(&tmp, DEVICETREE_HEAD_OFFSTC);
    tmp.off_str  = dt_raw_num(&tmp, DEVICETREE_HEAD_OFFSTR);
    tmp.size_stc = dt_raw_num(&tmp, DEVICETREE_HEAD_SIZSTC);
    tmp.size_str = dt_raw_num(&tmp, DEVICETREE_HEAD_SIZSTR);

    /* sanity check 2 */
    if(tmp.off_stc + tmp.size_stc > size ||
       tmp.off_str + tmp.size_str > size)
        return 0;

    *ctx = tmp;

    /* load root */
    int token = dt_block_load(ctx, & ctx->root, 0);
    return (token == DEVICETREE_TOKEN_NODE_START) ? 1 : 0;
}
