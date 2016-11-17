tinyDTB
=======

tinyDTB is a minimal library for reading flattened devicetrees (DTS) in binary form (DTB). It is very small in size and parses the binary "in-place", hence it also has a minimal memory footprint.

When should I use this?
-----------------------

If you are writing software for resource constrained devices, or for some other reason
want to keep resource usage to a minimum (e.g. if you are writing a bootloader) you
might find this library useful.

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

The *examples/* folder contains sample code that demonstrates how to use the library.
