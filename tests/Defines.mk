#slightly modified Defines.mk from src directory

BOARD_DIR := $(subst fw_,,$(TARGETNAME))

#determine the architecture, mcu and mcu family by directory in which the board dir is located
ARCH := $(shell $(FIND) ../src/board -type d ! -path *build -name *$(BOARD_DIR) | cut -d/ -f4 | head -n 1)
MCU := $(shell $(FIND) ../src/board -type d -name *$(BOARD_DIR) | cut -d/ -f7 | head -n 1)
MCU_FAMILY := $(shell $(FIND) ../src/board -type d -name *$(BOARD_DIR) | cut -d/ -f6 | head -n 1)

#mcu specific
ifeq ($(MCU),atmega32u4)
    FLASH_SIZE_START_ADDR := 0xAC
else ifeq ($(MCU),at90usb1286)
    FLASH_SIZE_START_ADDR := 0x98
else ifeq ($(MCU),atmega16u2)
    FLASH_SIZE_START_ADDR := 0x74
else ifeq ($(MCU),atmega8u2)
    FLASH_SIZE_START_ADDR := 0x74
else ifeq ($(MCU),atmega2560)
    FLASH_SIZE_START_ADDR := 0xE4
else ifeq ($(MCU),atmega328p)
    FLASH_SIZE_START_ADDR := 0x68
endif

DEFINES_COMMON += APP_LENGTH_LOCATION=$(FLASH_SIZE_START_ADDR)
DEFINES_COMMON += OD_BOARD_$(shell echo $(BOARD_DIR) | tr 'a-z' 'A-Z')

ifneq ($(HARDWARE_VERSION_MAJOR), )
    DEFINES_COMMON += HARDWARE_VERSION_MAJOR=$(HARDWARE_VERSION_MAJOR)
endif

ifneq ($(HARDWARE_VERSION_MINOR), )
    DEFINES_COMMON += HARDWARE_VERSION_MINOR=$(HARDWARE_VERSION_MINOR)
endif