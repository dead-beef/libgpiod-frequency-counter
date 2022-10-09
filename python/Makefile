CC := gcc
PYTHON := python3
PYTHON_CONFIG := $(PYTHON)-config
INSTALL := install -m 644
CFLAGS := -Wall -Werror -O2 -fPIC -fvisibility=hidden
LDFLAGS := -L../bin -lgpiod -lgpiod-frequency-counter

PY_CFLAGS  := $(shell $(PYTHON_CONFIG) --cflags)
PY_LDFLAGS := $(shell $(PYTHON_CONFIG) --ldflags)
PY_LIB_SUFFIX := $(shell $(PYTHON_CONFIG) --extension-suffix)
PY_LIB_DIR := $(shell $(PYTHON_CONFIG) --prefix)/lib
#PY_LIBS_EMBED := $(shell $(PYTHON_CONFIG) --libs --embed)
#PY_VERSION := $(subst -l,,$(filter -lpython%,$(PY_LIBS_EMBED)))

INSTALL_DIR := $(PY_LIB_DIR)/python3/dist-packages

LIB := gpiod_frequency_counter
SRC_DIR := .
INCLUDE_DIRS := ../include

BUILD_DIR = .
OBJ_DIR := $(BUILD_DIR)/obj
BIN_DIR := $(BUILD_DIR)/bin
DEP_DIR := $(BUILD_DIR)/dep

INCLUDE_DIRS := $(addprefix -I,$(INCLUDE_DIRS))
CFLAGS := $(PY_CFLAGS) $(CFLAGS) $(INCLUDE_DIRS)
LDFLAGS := $(PY_LDFLAGS) $(LDFLAGS)
LIB := $(BIN_DIR)/$(LIB)$(PY_LIB_SUFFIX)

make_path = $(addsuffix $(1), $(basename $(subst $(2), $(3), $(4))))
src_to_obj = $(call make_path,.o, $(SRC_DIR), $(OBJ_DIR), $(1))
src_to_dep = $(call make_path,.d, $(SRC_DIR), $(DEP_DIR), $(1))

CFILES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(foreach src, $(CFILES), $(call src_to_obj, $(src)))
DEPS := $(foreach src, $(CFILES), $(call src_to_dep, $(src)))

.DEFAULT_GOAL := all
NODEPS = clean

all: $(LIB)

clean:
	rm -rvf $(OBJ_DIR)/* $(DEP_DIR)/* $(LIB)

install:
	mkdir -p $(DESTDIR)/$(INSTALL_DIR)
	$(INSTALL) $(LIB) $(DESTDIR)/$(INSTALL_DIR)

$(LIB): $(OBJECTS) $(DEPS) | $(BIN_DIR)
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
