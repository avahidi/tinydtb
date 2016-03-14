tinyDTS
=======

tinyDTS is a minimal library for reading binary devicetrees (DTB). It is very small ins size and parses the library "in-place", hence it has minimal memory footprint.

When should I use this?
-----------------------

If you are writing software for a resource limited device, or for some other reason
want to keep resource usage to a minimum (e.g. if you are writing a bootloader) you
might find this library useful.

Building
--------
The make file accepts three parameters: CROSS_COMPILE, UFLAGS (which will be appended to CFLAGS)

::

   # native build
   make
   # build for ARMv7
   make CROSS_COMPILE=arm-none-eabi-
   # build for ARMv8 with additional flags
   make CROSS_COMPILE=aarch64-linux-gnu- UFLAGS="-mcpu=cortex-a54"

Usage
-----

The *example/* folder contains sample code that demonstrates how to use the library.
You can run the example be executing

::
   make example
