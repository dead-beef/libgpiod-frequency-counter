AR := ar
CC := gcc
INSTALL := install -m 644
CFLAGS := -Wall -Werror -O2 -fPIC -fvisibility=hidden
LDFLAGS := -lgpiod

PKG := libgpiod-frequency-counter
VERSION := 0.4.0
SRC_DIR := src
INCLUDE_DIR := include
INCLUDE_DIRS := $(INCLUDE_DIR)

BUILD_DIR = .
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
DEP_DIR := $(BUILD_DIR)/dep

INCLUDE_DIRS := $(addprefix -I,$(INCLUDE_DIRS))
CFLAGS += $(INCLUDE_DIRS)

make_path = $(addsuffix $(1), $(basename $(subst $(2), $(3), $(4))))
src_to_obj = $(call make_path,.o, $(SRC_DIR), $(OBJ_DIR), $(1))
src_to_dep = $(call make_path,.d, $(SRC_DIR), $(DEP_DIR), $(1))

CFILES := $(wildcard $(SRC_DIR)/*.c)
LIB_FILES := $(addprefix $(BIN_DIR)/$(PKG), .so .a)
OBJECTS := $(foreach src, $(CFILES), $(call src_to_obj, $(src)))
DEPS := $(foreach src, $(CFILES), $(call src_to_dep, $(src)))

.DEFAULT_GOAL := all
NODEPS = clean

all: $(LIB_FILES)
	make -C tools
	make -C python

clean:
	make -C tools clean
	make -C python clean
	rm -rvf $(OBJ_DIR)/* $(DEP_DIR)/* $(LIB_FILES)

pkg: clean
	tar czvf ../$(PKG)_$(VERSION).orig.tar.gz .
	dpkg-buildpackage -us -uc

install:
	mkdir -p $(addprefix $(DESTDIR)/usr/, lib include)
	$(INSTALL) $(LIB_FILES) $(DESTDIR)/usr/lib/
	$(INSTALL) $(INCLUDE_DIR)/gpiod_frequency_counter.h $(DESTDIR)/usr/include/
	make -C tools DESTDIR=$(shell realpath $(DESTDIR)) install
	make -C python DESTDIR=$(shell realpath $(DESTDIR)) install

$(BIN_DIR)/$(PKG).a: $(OBJECTS) $(DEPS) | $(BIN_DIR)
	$(AR) rcs $@ $(OBJECTS)

$(BIN_DIR)/$(PKG).so: $(OBJECTS) $(DEPS) | $(BIN_DIR)
	$(CC) -shared -o $@ $(OBJECTS) $(LDFLAGS)

$(DEP_DIR)/%.d: $(SRC_DIR)/%.c | $(DEP_DIR)
	$(CC) $(INCLUDE_DIRS) -MM -MT $(call src_to_obj, $<) $< -MF $@

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEP_DIR)/%.d | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR) $(DEP_DIR) $(OBJ_DIR):
	mkdir -pv $@

ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(DEPS)
endif
