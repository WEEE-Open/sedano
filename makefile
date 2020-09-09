# Compilation directories
OBJ_DIR := obj
DBG_DIR := obj/dbg
SRC_DIR := src
BIN_DIR := bin

# Compilation objects
SOURCES := $(wildcard $(SRC_DIR)/*.c)
OBJECTS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
DBGOBJS := $(patsubst $(SRC_DIR)/%.c, $(DBG_DIR)/%.o, $(SOURCES))

# Compilation options
COMPILER := gcc
LINK_OPTIONS := -lX11
ASSEMBLER_OPTIONS_DBG := -ggdb

all: directories release debug

directories:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(DBG_DIR)

clean:
	rm -f $(wildcard $(OBJ_DIR)/*.o)
	rm -f $(BIN_DIR)/sedano

release: $(OBJECTS)
	$(COMPILER) $(LINK_OPTIONS) -o $(BIN_DIR)/release $^

debug: $(DBGOBJS)
	$(COMPILER) $(LINK_OPTIONS) -o $(BIN_DIR)/debug $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(COMPILER) -c -o $@ $<

$(DBG_DIR)/%.o: $(SRC_DIR)/%.c
	$(COMPILER) $(ASSEMBLER_OPTIONS_DBG) -c -o $@ $<