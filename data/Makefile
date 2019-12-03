

INPUT = $(wildcard *.dts)
OUTPUT = $(patsubst %.dts, %.dtb, $(INPUT))


all: $(OUTPUT)

%.dtb: %.dts
	dtc $< -O dtb -o $@

setup:
	sudo apt install -y device-tree-compiler
	
