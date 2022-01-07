tinyDTB
=======

tinyDTB is a **minimal** library for reading flattened device trees (DTS) in binary form (DTB).
It is very small in size and parses the binary "in-place", hence it also has a minimal memory footprint.

In case you have never seen a device tree, here is one before compilation::

    /* stolen from Thomas Petazzoni's ELCE-12 presentation */
    /dts-v1/;
    /memreserve/ 0x0c000000 0x04000000;
    /include/ "bcm2835.dtsi"
    / {
        compatible = "raspberrypi,model-b", "brcm,bcm2835";
        model = "Raspberry Pi Model B";
        memory {
            reg = <0 0x10000000>;
        };
        soc {
            uart@20201000 {
                status = "okay";
            };
       };
    };

Which is transformed to a flat binary format using the device tree compiler *dtc*.
The tinyDTB library contains the API needed to access that binary at runtime.


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
The makefile accepts two optional parameters: CROSS_COMPILE and UFLAGS (which is added to our CFLAGS)

::

   # native build
   make
   # build for ARMv7
   make CROSS_COMPILE=arm-none-eabi-
   # build for ARMv8 with additional flags
   make CROSS_COMPILE=aarch64-linux-gnu- UFLAGS="-mcpu=cortex-a53"


Examples
--------

The *examples/* folder contains sample code that demonstrates how to use the library::

   make examples         # build them
   make run              # run them


Fuzzing
-------

This project was just the right size for learning fuzzing with libFuzzer & clang,
so we now have some minimal fuzz tests under the *fuzz/* folder :)
::

   make fuzz            # inside an LXC container, the "right" way
   make -C fuzz fuzz    # native , the "unsupported" way
