CC := gcc
INSTALL := install -m 644
INSTALL_BIN := install -m 755
CFLAGS := -Wall -Werror -O2 -fPIC
LDFLAGS := -L../bin -lgpiod -lgpiod-frequency-counter

SRC_DIR := .
INCLUDE_DIRS := ../include
DATA_DIR := data

BUILD_DIR = .
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
DEP_DIR := $(BUILD_DIR)/dep

INCLUDE_DIRS := $(addprefix -I,$(INCLUDE_DIRS))
CFLAGS += $(INCLUDE_DIRS)

CFILES := $(wildcard $(SRC_DIR)/*.c)
EXECUTABLE := $(BIN_DIR)/gpio-frequency-get

make_path = $(addsuffix $(1), $(basename $(subst $(2), $(3), $(4))))
src_to_obj = $(call make_path,.o, $(SRC_DIR), $(OBJ_DIR), $(1))
src_to_dep = $(call make_path,.d, $(SRC_DIR), $(DEP_DIR), $(1))

OBJECTS := $(foreach src, $(CFILES), $(call src_to_obj, $(src)))
DEPS := $(foreach src, $(CFILES), $(call src_to_dep, $(src)))

.DEFAULT_GOAL := all
NODEPS = clean

all: $(EXECUTABLE)

clean:
	rm -rvf $(OBJ_DIR)/* $(DEP_DIR)/* $(EXECUTABLE)

install:
	mkdir -p $(DESTDIR)/usr/bin
	$(INSTALL_BIN) $(EXECUTABLE) $(DESTDIR)/usr/bin/

$(EXECUTABLE): $(OBJECTS) $(DEPS) | $(BIN_DIR)
	$(CC) -o $@ $(OBJECTS) $(LDFLAGS)

$(DEP_DIR)/%.d: $(SRC_DIR)/%.c | $(DEP_DIR)
	$(CC) $(INCLUDE_DIRS) -MM -MT $(call src_to_obj, $<) $< -MF $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEP_DIR)/%.d | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR) $(DEP_DIR) $(OBJ_DIR):
	mkdir -pv $@

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(DEPS)
endif
