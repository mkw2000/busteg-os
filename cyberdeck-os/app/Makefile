CC ?= gcc
PKG_CONFIG ?= pkg-config

APP := cyberdeck-os
BUILD_DIR := build
SRC_DIR := src

rwildcard = $(foreach d,$(wildcard $1*),$(call rwildcard,$d/,$2) $(filter $(subst *,%,$2),$d))
SRCS := $(call rwildcard,$(SRC_DIR)/,*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/%.o,$(SRCS))
DEPS := $(OBJS:.o=.d)

CFLAGS ?= -std=c17 -Wall -Wextra -Wpedantic -O2
CFLAGS += -Isrc -MMD -MP
LDLIBS += $(shell $(PKG_CONFIG) --libs sdl2 SDL2_ttf 2>/dev/null || echo -lSDL2 -lSDL2_ttf)
CFLAGS += $(shell $(PKG_CONFIG) --cflags sdl2 SDL2_ttf 2>/dev/null)

.PHONY: all run clean

all: $(BUILD_DIR)/$(APP)

$(BUILD_DIR)/$(APP): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDLIBS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

run: all
	./$(BUILD_DIR)/$(APP)

clean:
	rm -rf $(BUILD_DIR)

-include $(DEPS)
