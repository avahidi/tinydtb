tinyDTB
=======

tinyDTB is a **minimal** library for reading flattened `device trees (DTS) <https://en.wikipedia.org/wiki/Devicetree>`_ in binary form (DTB). 
It is very small in size and parses the binary "in-place", hence it also has a minimal memory footprint.


When should I use this?
-----------------------

If you are writing software for resource constrained devices, or for some other reason
want to keep resource usage to a minimum (e.g. if you are writing a bootloader) you
might find this library useful.

The library is very small, 1 - 2 K bytes depending on the architecture. This is on AMD64

::

    $ objdump -h build/libtinydtb.a
    In archive build/libtinydtb.a:
    
    tinydtb.o:     file format elf64-x86-64
    
    Sections:
    Idx Name          Size      VMA               LMA               File off  Algn
      0 .text         000004c1  0000000000000000  0000000000000000  00000040  2**0
                      CONTENTS, ALLOC, LOAD, RELOC, READONLY, CODE
      1 .data         00000000  0000000000000000  0000000000000000  00000501  2**0
                      CONTENTS, ALLOC, LOAD, DATA
      2 .bss          00000000  0000000000000000  0000000000000000  00000501  2**0
                      ALLOC
      3 .rodata       00000024  0000000000000000  0000000000000000  00000504  2**2
                      CONTENTS, ALLOC, LOAD, RELOC, READONLY, DATA
      ...

Building
--------
The makefile accepts CROSS_COMPILE

::

   # native build
   make

   # build for ARMv7
   make CROSS_COMPILE=arm-none-eabi-
   
   # build for ARMv8 with additional flags
   make CROSS_COMPILE=aarch64-linux-gnu- CFLAGS="-mcpu=cortex-a53"


Usage
--------

With the binary device tree is already in memory, the library is initialized as follows:

::

    #include "tinydtb.h"
    ...    

    /* adr and size refer to location and length of the DTB in memory */
    struct dt_context dtb;
    dt_init(&dtb, adr, size);
    

The function *dt_block_find* is used to find nodes or properties in the DTB:

::


    struct dt_block block prop;

    /*
     * The parameters are as follows:
     *  dtb is the previously loaded DTB
     *  We start from root, hence parent is NULL. This can also be another node
     *  prop is where the result is stored
     *  the next parameter indicates if we are iterating over nodes (1) or properties (0)
     *  "prop1" is the name of the node, and can also be a path such as node1/node2/prop1
     *  the last number is the length of the name, -1 means the entire string
     */
    if(dt_block_find(&dtb, NULL /* parent */, &prop, 0 /* find prop */, "prop1", -1))
        printf("prop1: data='%s', size %d\n", prop.data.str, prop.data_len);


You can iterate over nodes/properties anywhere in the tree

::

    /* print all items in / */
    struct dt_block block *it;
    struct dt_foreach fe;

    /*
     * The parameters are as follows:
     *  dtb is the previously loaded DTB
     *  We start from root, hence parent is NULL.
     *  fe is the resulting iterator
     *  the last parameter indicates if we are iterating over nodes (1) or properties (0)
     */
    dt_foreach_init(&dtb, NULL /* from root */, &fe, 1);
    while( (it = dt_foreach_next(&fe))) {
        printf(" foreach block %s -> data at %p, size = %d\n", it->name, it->data.ptr, it->data_len);
    }


For more examples, refer to the *examples/* folder. 


Fuzzing
-------

This project was just the right size for learning fuzzing with libFuzzer & clang,
so we now have some minimal fuzz tests under the *fuzz/* folder :)

To run the fuzzer inside an LXC container
::

    # setup everything and run fuzzer inside an LXC container
    make fuzz

To instead run it naively
::

   make -C fuzz setup
   make -C fuzz fuzz
