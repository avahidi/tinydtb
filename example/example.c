
#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "tinydts.h"

void fatal(char *msg); /* forward reference */

void demo(void *data, uint32_t size)
{
   struct dt_context dtb;
   struct dt_block block, prop, *it;
   struct dt_foreach fe;

   /* 1. initialize the DTB context from memory */
   if(!dt_init(&dtb, data, size))
        fatal("not a devicetree");


   /* 1.A find a property in the root of the tree
    *    in this case it is an string so we will use prop.data.str
    */
   if(!dt_block_find(&dtb,
		     NULL, /* no parent */
		     &prop, /* put the result here */
		     0, /* 0 is a property, 1 is a node/block */
		     "prop1", /* name of our property */
		     -1 /* size of the name, computed by the lib if set to -1 */))
       fatal("Could not find prop1");

   printf("prop1: data='%s', size %d\n", prop.data.str, prop.data_len);

   /* 1.B same thing, now with an integer.
    *    note that we need dtend() to convert from DTB endian to our endian format */
   if(!dt_block_find(&dtb, NULL, &prop, 0, "prop2", -1))
       fatal("Could not find prop2");

   printf("prop2: data=%08lx %08lx, size %d (native endian)\n",
	  prop.data.num[0], prop.data.num[1], prop.data_len);

   printf("prop2: data=%08lx %08lx, size %d (corrected endian)\n",
	  dtend(prop.data.num[0]),
	  dtend(prop.data.num[1]),
	  prop.data_len);

   /* 2.A now, access /node1/prop3, which is a string */
   if(!dt_block_find(&dtb, NULL, &prop, 0, "/node1/prop3", -1))
       fatal("Could not find prop3");

   printf("prop3: data='%s', size %d\n", prop.data.str, prop.data_len);

   /* 2.A we can also first extract node1 and from that get prop3 */
   if(!dt_block_find(&dtb, NULL, &block, 1, "/node1", -1))
       fatal("Could not find node1");

   if(!dt_block_find(&dtb, &block, &prop, 0, "prop3", -1))
       fatal("Could not find prop3");

   printf("prop3: data='%s', size %d\n", prop.data.str, prop.data_len);

   /* 3.A we can also iterate over all data at a certain level
    *    this is useful if we don't know exactly what elements the DTB contains.
    *    the code below iterates over all blocks in the root
    */

   dt_foreach_init(&dtb, NULL, &fe, 1);
   printf("FOREACH block in /:\n");
   while( (it = dt_foreach_next(&fe))) {
       printf(" foreach block %s -> data at %p, size = %d\n", it->name, it->data.ptr, it->data_len);
   }

   /* 3.B, same thing with properties in / */
   dt_foreach_init(&dtb, NULL, &fe, 0);
   printf("FOREACH property in /:\n");
   while( (it = dt_foreach_next(&fe))) {
       printf(" foreach block %s -> data at %p, size = %d\n", it->name, it->data.ptr, it->data_len);
   }

   /* 3.C You don't have to be in root: */
   if(!dt_block_find(&dtb, NULL, &block, 1, "/node1/node2", -1))
       fatal("Could not find node1/node2:\n");

   dt_foreach_init(&dtb, &block, &fe, 0);
   printf("FOREACH property in /node1/node2:\n");
   while( (it = dt_foreach_next(&fe))) {
       printf(" foreach property %s -> data at %p, size = %d\n", it->name, it->data.ptr, it->data_len);
   }


   /* 3.D  you can also iterate over specifc prefixez: */
   dt_foreach_init(&dtb, NULL, &fe, 1);
   printf("FOREACH data@xxx block in /:\n");
   while( (it = dt_foreach_next_of(&fe, "data@"))) {
       printf(" foreach property %s -> data at %p, size = %d\n", it->name, it->data.ptr, it->data_len);
   }

   #if 0

/ {
  prop1 = "stuff";
  prop2 = <0x01234567 0xAABBCCDD>;
  node1 {
    prop3 = "more stuff";
    node2  {
      prop4 = <0x555>;
    };
  };
};
#endif


}


/* This code will do load the DTB from file into memory.
 * In an embedded system, we would probably use the DTB as-is from flash
 */
void load_dtb(char *filename, void **adr, uint32_t *size)
{
    struct stat st;
    int fd;
    void *mem;

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
    *adr = mem;
    *size = st.st_size;
}

void fatal(char *msg)
{
    fprintf(stderr, "ERROR: %s\n", msg);
    exit(20);
}

int main(int argc, char **argv)
{
    void *dtb;
    uint32_t size;

    if(argc != 2)
	fatal("Usage: example <DTB file>");

    load_dtb(argv[1], &dtb, &size);

    demo(dtb, size);
    return 0;
}
