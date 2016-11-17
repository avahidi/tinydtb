
BASE = ../..
BUILD= $(BASE)/build
DTB = $(BASE)/build/example.dtb

all: $(BUILD)/intro.exe

$(BUILD)/intro.exe: src/intro.c
	gcc -I$(BASE)/src -L $(BUILD) $< -o $@ -ltinydtb

run: $(BUILD)/intro.exe
	$(BUILD)/intro.exe $(DTB)


clean:
	rm -f $(BUILD)/intro.exe
