
BASE = ../..
NAME = dtbdump

BUILD= $(BASE)/build
DTB = $(BASE)/data/example.dtb

all: $(BUILD)/$(NAME).exe

$(BUILD)/$(NAME).exe: src/*.c
	gcc -I$(BASE)/src -L $(BUILD) src/*.c -o $@ -ltinydtb

run: $(BUILD)/$(NAME).exe
	$(BUILD)/$(NAME).exe $(DTB)


clean:
	rm -f $(BUILD)/$(NAME).exe
