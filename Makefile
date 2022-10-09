AR := ar
CC := gcc
INSTALL := install -m 644
INSTALL_BIN := install -m 755
CFLAGS := -Wall -Werror -O2 -fPIC
LDFLAGS := -lgpiod
CMD_LDFLAGS := $(LDFLAGS) -L.

PKG := libgpiod-frequency-counter
CMD := gpio-frequency-get
VERSION := 0.2.0
LIB_NAME := gpiod-frequency-counter
LIB := $(addprefix lib$(LIB_NAME), .so .a)
SRC_DIR := src
INCLUDE_DIR := include
INCLUDE_DIRS := $(INCLUDE_DIR)
DATA_DIR := data

BUILD_DIR = .
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)
DEP_DIR := $(BUILD_DIR)/dep

INCLUDE_DIRS := $(addprefix -I,$(INCLUDE_DIRS))
CFLAGS += $(INCLUDE_DIRS)
CMD_LDFLAGS += -l$(LIB_NAME)

# All source files in our project (without libraries!)
LIB_CFILES := $(SRC_DIR)/gpiod_frequency_counter.c
CMD_CFILES := $(SRC_DIR)/gpio_frequency_get.c
#DATA := $(wildcard $(DATA_DIR)/*)
#DATA := $(DATA_DIR)

# Helper macros
make_path = $(addsuffix $(1), $(basename $(subst $(2), $(3), $(4))))
# Takes path to source file and returns path to corresponding object
src_to_obj = $(call make_path,.o, $(SRC_DIR), $(OBJ_DIR), $(1))
src_to_dep = $(call make_path,.d, $(SRC_DIR), $(DEP_DIR), $(1))

LIB_OBJECTS := $(foreach src, $(LIB_CFILES), $(call src_to_obj, $(src)))
LIB_DEPS := $(foreach src, $(LIB_CFILES), $(call src_to_dep, $(src)))

CMD_OBJECTS := $(foreach src, $(CMD_CFILES), $(call src_to_obj, $(src)))
CMD_DEPS := $(foreach src, $(CMD_CFILES), $(call src_to_dep, $(src)))

DEPFILES := $(LIB_DEPS) $(CMD_DEPS)

# Default target (make without specified target).
.DEFAULT_GOAL := all
NODEPS = clean

all: $(LIB) $(CMD)

# Delete all temprorary and binary files
clean:
	rm -rvf $(OBJ_DIR)/* $(DEP_DIR)/* $(CMD) $(LIB)

pkg: clean
#	rm -fv ../$(PKG)_*
	tar czvf ../$(PKG)_$(VERSION).orig.tar.gz .
	dpkg-buildpackage -us -uc

install:
	mkdir -p $(addprefix $(DESTDIR)/usr/, bin lib include)
	$(INSTALL_BIN) $(CMD) $(DESTDIR)/usr/bin/
	$(INSTALL) $(LIB) $(DESTDIR)/usr/lib/
	$(INSTALL) $(INCLUDE_DIR)/gpiod_frequency_counter.h $(DESTDIR)/usr/include/

# Rules for compiling targets
$(PKG).a: $(LIB_OBJECTS) $(LIB_DEPS)
	$(AR) rcs $@ $(LIB_OBJECTS)
#	cp -rvf $(DATA) $(BIN_DIR)

$(PKG).so: $(LIB_OBJECTS) $(LIB_DEPS)
	$(CC) -shared -o $@ $(LIB_OBJECTS)
#	cp -rvf $(DATA) $(BIN_DIR)

$(CMD): $(CMD_OBJECTS) $(CMD_DEPS) $(LIB)
	$(CC) -o $@ $(CMD_OBJECTS) $(CMD_LDFLAGS)

$(DEP_DIR)/%.d: $(SRC_DIR)/%.c | $(DEP_DIR)
	$(CC) $(INCLUDE_DIRS) -std=gnu99 -MM -MT $(call src_to_obj, $<) $< -MF $@

# Pattern for compiling object files (*.o)
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c $(DEP_DIR)/%.d | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c -o $@ $<

$(BIN_DIR) $(DEP_DIR) $(OBJ_DIR):
	mkdir -pv $@

#Don't create dependencies when we're cleaning
ifeq (0, $(words $(findstring $(MAKECMDGOALS), $(NODEPS))))
-include $(DEPFILES)
endif
