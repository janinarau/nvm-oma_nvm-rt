CROSS_COMPILER=aarch64-linux-gnu
GEM5_PATH=/gem/gem5
NVMAIN_PATH=/gem/nvmain

CCFLAGS=-Wall -Werror -nostdlib -I. -mstrict-align -include system/config.h -fno-exceptions
LDFLAGS=-T system/ldscript.ld -nostdlib -static -nostartfiles -N -mstrict-align

MODULES=system application
ALL_DEPS=system/config.h
BUILD_DIR=build

C_SOURCES=$(shell find $(MODULES) -name "*.c")
CC_SOURCES=$(shell find $(MODULES) -name "*.cpp")
ASM_SOURCES=$(shell find $(MODULES) -name "*.S")

C_OBJECTS=$(C_SOURCES:.c=.o)
CC_OBJECTS=$(CC_SOURCES:.cpp=.o)
ASM_OBJECTS=$(ASM_SOURCES:.S=.o)

OBJECTS=$(addprefix $(BUILD_DIR)/, $(C_OBJECTS) $(CC_OBJECTS) $(ASM_OBJECTS))

BUILD_FOLDERS=$(dir $(OBJECTS))

TARGET_APP=$(BUILD_DIR)/nvmrm

default: create_build_dirs $(TARGET_APP).elf

$(TARGET_APP).elf: $(OBJECTS)
	@echo "LD		$@"
	@$(CROSS_COMPILER)-g++ $(LDFLAGS) -o $@ $(OBJECTS)

$(BUILD_DIR)/%.o : %.c $(ALL_DEPS)
	@echo "C		$< -> $@"
	@$(CROSS_COMPILER)-g++ $(CCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : %.cpp %.h $(ALL_DEPS)
	@echo "CC		$< -> $@"
	@$(CROSS_COMPILER)-g++ $(CCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : %.cpp $(ALL_DEPS)
	@echo "CC		$< -> $@"
	@$(CROSS_COMPILER)-g++ $(CCFLAGS) -c $< -o $@

$(BUILD_DIR)/%.o : %.S $(ALL_DEPS)
	@echo "ASM		$< -> $@"
	@$(CROSS_COMPILER)-g++ $(CCFLAGS) -c $< -o $@

.PHONY: clean create_build_dirs run

clean:
	@rm -R -f build m5out

m5_p=.
export M5_PATH = $(m5_p)

run:
	@echo "this is just a fake iso that gem5 needs in order to start correctly" > $(BUILD_DIR)/fake.iso
	@echo "this is just a fake iso that gem5 needs in order to start correctly" >> $(BUILD_DIR)/fake.iso
	@echo "this is just a fake iso that gem5 needs in order to start correctly" >> $(BUILD_DIR)/fake.iso
	@echo "this is just a fake iso that gem5 needs in order to start correctly" >> $(BUILD_DIR)/fake.iso
	@echo "this is just a fake iso that gem5 needs in order to start correctly" >> $(BUILD_DIR)/fake.iso
	@echo "this is just a fake iso that gem5 needs in order to start correctly" >> $(BUILD_DIR)/fake.iso
	@echo "this is just a fake iso that gem5 needs in order to start correctly" >> $(BUILD_DIR)/fake.iso
	@echo "this is just a fake iso that gem5 needs in order to start correctly" >> $(BUILD_DIR)/fake.iso
	@$(GEM5_PATH)/build/ARM/gem5.fast $(GEM5_PATH)/configs/example/fs.py --bare-metal --disk-image=$(abspath ./$(BUILD_DIR)/fake.iso) --kernel=$(abspath ./$(TARGET_APP).elf) --mem-type=NVMainMemory --nvmain-config=$(NVMAIN_PATH)/Config/printtrace.config --cpu-type=DerivO3CPU --machine-type=VExpress_GEM5_V2 --caches --l2cache --l1i_size='32kB' --l1d_size='8kB' --l2_size='8kB' --dtb-filename=none --mem-size=4GB
	@rm $(BUILD_DIR)/fake.iso
	#@mv $(NVMAIN_PATH)/Config/nvmain.nvt $(BUILD_DIR)/trace.nvt

create_build_dirs:
	@$(foreach build_folder,$(BUILD_FOLDERS), mkdir -p $(build_folder))
	@rm -Rf mkdir
