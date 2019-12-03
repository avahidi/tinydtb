
WORKDIR ?= build

# files and directories
FUZZERS_C=$(wildcard src/*.c)
FUZZERS_EXE=$(patsubst src/%.c,$(WORKDIR)/%.exe,$(FUZZERS_C))
FUZZERS_LOG=$(patsubst src/%.c,$(WORKDIR)/%.log,$(FUZZERS_C))

TARGET_C = ../src/tinydtb.c

# fresh corpus
CORPUS_DIR = $(WORKDIR)/corpus


CFLAGS += -I ../src -I src


all: 
	echo targets are: setup, clean, fuzz


fuzz: $(WORKDIR) $(FUZZERS_LOG)

.PRECIOUS: %.exe
.PRECIOUS: %.log
.PRECIOUS: %.dts

$(DTB_DIR)/%.dtb: $(DTS_DIR)/%.dts
	dtc $< -O dtb -o $@


# setup the system with the required repos and tools
setup: $(WORKDIR) $(CORPUS_DIR)
	#sudo apt install -y --no-install-recommends clang
	git clone https://github.com/google/fuzzing $(WORKDIR)/fuzzing
	cd $(WORKDIR) && fuzzing/tutorial/libFuzzer/install-deps.sh  # Get deps
	cd $(WORKDIR) && fuzzing/tutorial/libFuzzer/install-clang.sh # Get fresh clang binaries
	cp ../data/*.dtb $(CORPUS_DIR)
	
$(WORKDIR):
	mkdir -p $(WORKDIR)

$(CORPUS_DIR):
	mkdir -p $(CORPUS_DIR)

clean:
	rm -f $(WORKDIR)/*.exe
	rm -f $(WORKDIR)/*.log

cleanall:
	rm -rf $(WORKDIR)

# ---------------------------------------------------------

$(WORKDIR)/dt_init.log: src/dt_init.c
	clang -g -fsanitize=address,fuzzer $(CFLAGS) $(TARGET_C) $< -o $(WORKDIR)/dt_init.exe
	cd $(WORKDIR) && ./dt_init.exe $(CORPUS_DIR) 2> $@ || echo DONE
	cat $@

