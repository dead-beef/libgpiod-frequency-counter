AR := ar
CC := gcc
INSTALL := install -m 644
CFLAGS := -Wall -Werror -std=gnu99 -O2 -fPIC
LDFLAGS := -lpthread -lgpiod

# Directories with source code
EXECUTABLE := libgpiod-frequency-counter.so libgpiod-frequency-counter.a
SRC_DIR := .
INCLUDE_DIRS := include
DATA_DIR := data

BUILD_DIR = .
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)
DEP_DIR := $(BUILD_DIR)/dep

INCLUDE_DIRS := $(addprefix -I,$(INCLUDE_DIRS))
CFLAGS += $(INCLUDE_DIRS)

# All source files in our project (without libraries!)
CFILES := $(wildcard $(SRC_DIR)/*.c)
#DATA := $(wildcard $(DATA_DIR)/*)
#DATA := $(DATA_DIR)

# Helper macros
make_path = $(addsuffix $(1), $(basename $(subst $(2), $(3), $(4))))
# Takes path to source file and returns path to corresponding object
src_to_obj = $(call make_path,.o, $(SRC_DIR), $(OBJ_DIR), $(1))
src_to_dep = $(call make_path,.d, $(SRC_DIR), $(DEP_DIR), $(1))

OBJECTS := $(foreach src, $(CFILES), $(call src_to_obj, $(src)))
DEPS := $(foreach src, $(CFILES), $(call src_to_dep, $(src)))

# Default target (make without specified target).
.DEFAULT_GOAL := all
NODEPS = clean

all: $(EXECUTABLE)

# Delete all temprorary and binary files
clean:
	rm -rvf $(OBJ_DIR)/* $(DEP_DIR)/* $(EXECUTABLE)

install:
	$(INSTALL) libgpiod-frequency-counter.a $(DESTDIR)/usr/lib/
	$(INSTALL) libgpiod-frequency-counter.so $(DESTDIR)/usr/lib/
	$(INSTALL) gpiod_frequency_counter.h $(DESTDIR)/usr/include/

# Rules for compiling targets
libgpiod-frequency-counter.a: $(OBJECTS) $(DEPS)
	$(AR) rcs $@ $(OBJECXTS)
#	cp -rvf $(DATA) $(BIN_DIR)

libgpiod-frequency-counter.so: $(OBJECTS) $(DEPS)
	$(CC) -shared -o $@ $(OBJECTS)
#	cp -rvf $(DATA) $(BIN_DIR)

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
