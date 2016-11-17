#

CROSS_COMPILE ?=
NAME ?= tinydtb
DIR = src

#
SRC = $(foreach d,$(DIR),$(wildcard $(d)/*.c))
INC = $(foreach d,$(DIR),$(wildcard $(d)/*.h))
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

CFLAGS += -nostdlib -ffreestanding
CFLAGS += -Os -fno-common
CFLAGS += $(foreach d,$(DIR),-I $(d))
CFLAGS += $(UFLAGS)

#
all: build/lib$(NAME).a examples
	ls -l build/lib$(NAME).a

examples: build/lib$(NAME).a
	make -C examples

build/lib$(NAME).a: $(OBJ) Makefile
	@echo AR $@
	@$(CROSS_COMPILE)ar rcs $@ $(OBJ)
	@$(CROSS_COMPILE)objdump -dth $@ > $@.dis

build/%.o: src/%.c Makefile build $(INC)
	@echo GCC $<
	@$(CROSS_COMPILE)gcc $(CFLAGS) -c $< -o $@
	@$(CROSS_COMPILE)objdump -d $@ > $@.dis

#

build:
	mkdir build

clean:
	rm -rf build
	make -C examples clean
