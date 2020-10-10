OBJ_DIR := obj
DBG_DIR := obj/dbg
SRC_DIR := src
BIN_DIR := bin

SOURCES := $(wildcard $(SRC_DIR)/*.c)
RELOBJS := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SOURCES))
DBGOBJS := $(patsubst $(SRC_DIR)/%.c, $(DBG_DIR)/%.o, $(SOURCES))

COMPILER := gcc

REL_OPTIONS_BUILD := -Wall -pedantic
REL_OPTIONS_LINKER := -lX11
REL_OPTIONS_ASSEMBLER :=

DBG_OPTIONS_BUILD := -Wall -pedantic
DBG_OPTIONS_LINKER := -lX11
DBG_OPTIONS_ASSEMBLER := -ggdb -DDEBUG

all: directories release debug
rebuild: clean all

clean:
	rm -f $(wildcard $(OBJ_DIR)/*.o)
	rm -f $(wildcard $(DBG_DIR)/*.o)
	rm -f $(BIN_DIR)/debug
	rm -f $(BIN_DIR)/release

directories:
	mkdir -p $(OBJ_DIR)
	mkdir -p $(BIN_DIR)
	mkdir -p $(DBG_DIR)

release: $(RELOBJS)
	$(COMPILER) $(REL_OPTIONS_BUILD) $(REL_OPTIONS_LINKER) -o $(BIN_DIR)/release $^

debug: $(DBGOBJS)
	$(COMPILER) $(DBG_OPTIONS_BUILD) $(DBG_OPTIONS_LINKER) -o $(BIN_DIR)/debug $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(COMPILER) $(REL_OPTIONS_BUILD) $(REL_OPTIONS_ASSEMBLER) -c -o $@ $<

$(DBG_DIR)/%.o: $(SRC_DIR)/%.c
	$(COMPILER) $(DBG_OPTIONS_BUILD) $(DBG_OPTIONS_ASSEMBLER) -c -o $@ $<