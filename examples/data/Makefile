
BASE = ../..
BUILD= $(BASE)/build


INPUT = $(wildcard src/*.dts)
OUTPUT = $(patsubst src/%.dts, $(BUILD)/%.dtb, $(INPUT))


all: $(OUTPUT)

$(BUILD)/%.dtb: src/%.dts
	dtc $< -O dtb -o $@

clean:
	rm -rf $(OUTPUT)
