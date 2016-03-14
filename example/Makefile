
BUILD=../build

all:
	dtc example.dts -O dtb -o $(BUILD)/example.dtb
	gcc -I../src -L $(BUILD) example.c -o $(BUILD)/example.exe -ltinydts
	$(BUILD)/example.exe $(BUILD)/example.dtb
