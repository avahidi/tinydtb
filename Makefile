#

CROSS_COMPILE ?=
NAME ?= tinydtb

#
SRC = $(wildcard src/*.c)
INC = $(wildcard src/*.h)
OBJ = $(patsubst src/%.c,build/%.o,$(SRC))

CFLAGS += -nostdlib -ffreestanding
CFLAGS += -Os -fno-common
CFLAGS += -I src
CFLAGS += $(UFLAGS)

#

all: build/lib$(NAME).a examples
	ls -l build/lib$(NAME).a

examples: build/lib$(NAME).a
ifeq (,$(CROSS_COMPILE))
	make -C examples
endif

run: examples
	make -C examples run

.PHONY: fuzz
fuzz:
	cd fuzz && ./lxc.sh

.PHONY: test
test:
	@echo "TODO :("


build/lib$(NAME).a: $(OBJ) Makefile
	@echo AR $@
	@$(CROSS_COMPILE)ar rcs $@ $(OBJ)
	@$(CROSS_COMPILE)objdump -dth $@ > $@.dis

build/%.o: src/%.c Makefile build $(INC)
	@echo GCC $<
	@$(CROSS_COMPILE)gcc $(CFLAGS) -c $< -o $@
	@$(CROSS_COMPILE)objdump -d $@ > $@.dis
	@$(CROSS_COMPILE)objdump -h $@
#

build:
	mkdir build

clean:
	rm -rf build
	make -C examples clean
