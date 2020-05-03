vpath application/%.cpp ../src
vpath common/%.cpp ../src
vpath modules/%.cpp ../
vpath modules/%.c ../

#common for all targets
SOURCES_COMMON := \
modules/unity/src/unity.c \
modules/dbms/src/LESSDB.cpp \
modules/EmuEEPROM/src/EmuEEPROM.cpp \
modules/midi/src/MIDI.cpp \
modules/u8g2/csrc/u8x8_string.c \
modules/u8g2/csrc/u8x8_setup.c \
modules/u8g2/csrc/u8x8_u8toa.c \
modules/u8g2/csrc/u8x8_8x8.c \
modules/u8g2/csrc/u8x8_u16toa.c \
modules/u8g2/csrc/u8x8_display.c \
modules/u8g2/csrc/u8x8_fonts.c \
modules/u8g2/csrc/u8x8_byte.c \
modules/u8g2/csrc/u8x8_cad.c \
modules/u8g2/csrc/u8x8_gpio.c \
modules/u8g2/csrc/u8x8_d_ssd1306_128x64_noname.c \
modules/u8g2/csrc/u8x8_d_ssd1306_128x32.c

#common include dirs
INCLUDE_DIRS_COMMON := \
-I"./" \
-I"./unity" \
-I"../src/application/" \
-I"../src/" \
-I"../modules/" \
-I"../src/board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/"

INCLUDE_FILES_COMMON += \
-include "../src/board/$(ARCH)/variants/$(MCU_FAMILY)/$(MCU)/$(BOARD_DIR)/Hardware.h"
