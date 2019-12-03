
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
 * Raw and block operations
 */

static inline bool dt_raw_valid(struct dt_context *ctx, uint32_t index)
{
    return ((index & 3) != 0 || (index + sizeof(uint32_t) >= ctx->size) ) ? false : true;
}

static inline bool dt_str_valid(struct dt_context *ctx, uint32_t index)
{
    return index >= 0 && index < ctx->size_str;
}

static inline bool dt_block_valid(struct dt_context *ctx, uint32_t index)
{
    return index >= 0 && index < ctx->size_stc;
}

static inline bool dt_raw_num(struct dt_context *dt, uint32_t index, uint32_t *ret)
{
    if(!dt_raw_valid(dt, index))
        return false;

   *ret = bigend(dt->start[index / sizeof(uint32_t)]);
    return true;
}

static inline bool dt_block_num(struct dt_context *dt, uint32_t index, uint32_t *ret)
{
    if(! dt_block_valid(dt, index))
        return false;

    return dt_raw_num(dt, dt->off_stc + index, ret);
}

static inline bool dt_block_str(struct dt_context *dt, uint32_t index, char **ret)
{
    if(! dt_block_valid(dt, index))
        return false;

    *ret = (char *) &(dt->start[ ( dt->off_stc + index) / sizeof(uint32_t )]);
    return true;
}

static inline bool dt_str_str(struct dt_context *dt, uint32_t index, char **ret)
{
    if(! dt_str_valid(dt, index))
        return false;
    *ret = ((char *) dt->start) + (dt->off_str + index); /* not always aligned */
    return true;
}

/* this is our own strnstr, just to not read outside the memory */
static bool dt_strlen(struct dt_context *dt, char *str, uint32_t *len) {
	char *end = (char *)dt->start + dt->size;
	char *tmp;
	for(tmp = str; tmp < end; tmp++) {
		if(*tmp == 0) {
			*len = (uint32_t)(tmp - str);
			return true;
		}
	}
	return false;
}

static bool dt_block_load(struct dt_context *ctx,
                         struct dt_block *block,
                         uint32_t offset, uint32_t *token)
{
    uint32_t len, n;

    block->start = offset;
    block->level = 0;
    block->name = 0;
    block->data_len = 0;
    block->data.ptr = 0;

    if(!dt_block_valid(ctx, offset)) {
        block->token = 0;
        return false;
    }

    if(!dt_block_num(ctx, offset, &block->token))
        goto error;

    offset += 4;

    switch(block->token) {
    case DEVICETREE_TOKEN_NODE_START:
        if(! dt_block_str(ctx, offset, &block->name))
            goto error;
		if(!dt_strlen(ctx, block->name, &len))
			goto error;

        /* add one for '\0' and align to next 32-bit... */
        len = (len + 4) & ~3;
        offset += len;
		block->level ++;
        break;

    case DEVICETREE_TOKEN_NODE_END:
        block->level --;
        break;

    case DEVICETREE_TOKEN_PROP:
         if(!dt_block_num(ctx, offset, & block->data_len)
            || !dt_block_num(ctx, offset + 4, &n)
            || !dt_block_str(ctx, offset + 8, (char **) &block->data.ptr))
            goto error;
        len = (block->data_len + 3) & ~3;
        offset += 8 + len;
        if(!dt_str_str(ctx, n, & block->name))
            goto error;
        break;
    case DEVICETREE_TOKEN_NOP:
        break;
    case DEVICETREE_TOKEN_END:
        break;
    default:
        goto error;
    }

    block->end = offset;
    *token = block->token;
    return true;

error:
    block->end = offset;
    return false;
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
        if(!dt_block_load(ctx, block, fe->curr_offset, &token))
            return 0;
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

bool dt_block_find(struct dt_context *ctx, struct dt_block *block,
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
            return false;

        len -= sublen + 1;
        name = dir + 1;
        block = &store;
    }

    dt_foreach_init(ctx, block, &foreach, asnode);
    while( (tmp = dt_foreach_next(&foreach))) {
        if(!strncmp(tmp->name, name, len) && tmp->name[len] == '\0') {
            *result = *tmp;
            return true;
        }
    }
    return false;
}

/*
 * Context operations
 */
bool dt_init(struct dt_context *ctx,
            void *start, uint32_t size)
{
    struct dt_block first;
    struct dt_context tmp;
    uint32_t hmagic, hsize, token;

    tmp.start = start;
    tmp.size = size;

    /* sanity check 1 */
    if(!dt_raw_num(&tmp, DEVICETREE_HEAD_MAGIC, & hmagic)
        || !dt_raw_num(&tmp, DEVICETREE_HEAD_SIZHDR, &hsize))
        return false;

    if(hmagic != DEVICETREE_MAGIC || hsize > size)
        return false;

    /* actual dtb might be smaller than the memory area allocated for it */
    tmp.size = hsize;

    if(!dt_raw_num(&tmp, DEVICETREE_HEAD_OFFSTC, &tmp.off_stc)
        || !dt_raw_num(&tmp, DEVICETREE_HEAD_OFFSTR, &tmp.off_str)
        || !dt_raw_num(&tmp, DEVICETREE_HEAD_SIZSTC, &tmp.size_stc)
        || !dt_raw_num(&tmp, DEVICETREE_HEAD_SIZSTR, &tmp.size_str))
        return false;

    /* sanity check 2 */
    if(tmp.off_stc + tmp.size_stc > size ||
       tmp.off_str + tmp.size_str > size)
        return false;

    *ctx = tmp;

    /* load root */
    if(dt_block_load(ctx, & ctx->root, 0, &token))
        if(token == DEVICETREE_TOKEN_NODE_START)
            return true;
    return false;
}
