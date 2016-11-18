/*
 * dump a dtb using tinyDTB
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "tinydtb.h"


void fatal(char *msg)
{
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(20);
}

/*
 * is the data in this node a string?
 */
bool data_is_string(char *data, int len)
{
    int i;

    for(i = 0; i < len; i++)
        if(!isprint(data[i]) && data[i] != '\0')
            return false;

    return true;
}

void data_dump_string(char *data, int len)
{
    bool first;
    int n;

    for(first = true; len > 0; first = false) {
        n = strlen(data) + 1;
        printf("%s\"%s\"", first ? "" : ",", data);
        len -= n;
        data += n;
    }
}

/*
 * dump data for this node, either as a number list or a string
 */
void dtb_dump_prop(char *parent, struct dt_block *node)
{
    int i;
    printf("%s/%s = ", parent, node->name);
    if(data_is_string(node->data.str, node->data_len)) {
        data_dump_string(node->data.str, node->data_len);
    } else {
        for(i = 0; i < node->data_len / 4; i++)
            printf("%c0x%08x", i == 0 ? '<' : ',', node->data.num[i]);
        printf(">");
    }
    printf("\n");
}

/*
 * recursive dump of a DTB block:
 *   A. print all properties at this level
 *   B. traverse all child nodes
 */
void dtb_dump_rec(struct dt_context *dtb, char *pname, struct dt_block *pblock)
{
    struct dt_foreach fe;
    struct dt_block *it;
    char *name;

    /* A. all properties */
    dt_foreach_init(dtb, pblock , &fe, 0);
    while( (it = dt_foreach_next(&fe)))
        dtb_dump_prop(pname, it);

    /* B. all children */
    dt_foreach_init(dtb, pblock , &fe, 1);
    while( (it = dt_foreach_next(&fe))) {
        name = malloc( strlen(pname) + strlen("/") + strlen(it->name) + 1);
        if(!name)
            fatal("Could not allocate memory for the name");
        sprintf(name, "%s/%s", pname, it->name);
        dtb_dump_rec(dtb, name, it);
        free(name);
    }
}

void dtb_dump(struct dt_context *dtb)
{
    dtb_dump_rec(dtb, "", NULL);
}

/*
 * load file into memory and create a dtb context from it
 */
void dtb_load(char *filename, struct dt_context *dtb)
{
    struct stat st;
    int fd;
    void *mem;

    /* read the entire file into memory */
    if(stat(filename, &st) < 0)
        fatal("Could not get DTB size");

    if(! (mem = malloc(st.st_size)))
        fatal("Could not allocate DTB memory");

    if((fd = open(filename, 0)) < 0)
        fatal("Could not open DTB file");

    if(read(fd, mem, st.st_size)  != st.st_size) {
        close(fd);
        fatal("Could no read DTB");
    }

    close(fd);

    /* initialize the DTB context from memory */
    if(!dt_init(dtb, mem, st.st_size))
        fatal("not a devicetree");
}

int main(int argc, char **argv)
{
    struct dt_context dtb;

    if(argc != 2)
        fatal("Usage: dumpdtb.exe <DTB file>");

    dtb_load(argv[1], &dtb);
    dtb_dump(&dtb);

    return 0;
}
