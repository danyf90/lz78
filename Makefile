# paths
INC_PATH=include
OBJ_PATH=build
SRC_PATH=src
TEST_PATH=$(SRC_PATH)/tests
TEST_SCRIPT_PATH=tests

# parameters
CC = gcc
CFLAGS = -Wall -Werror -iquote./$(INC_PATH)
LDFLAGS = -lcrypto
SHELL = /bin/bash
DEBUG ?= 0

# if debug is enabled, compile with extra flags
ifeq ($(DEBUG),1)
CFLAGS += -g -DDEBUG
else
CFLAGS += -O2
endif

EXE = lz78

# header files
HEADERS = bitio.h common.h compressor.h decompressor.h dictionary.h main_utils.h metadata.h verbose.h

#source filese
SOURCES = bitio.c common.c compressor.c decompressor.c dictionary.c main.c main_utils.c metadata.c verbose.c

# object files
OBJECTS = $(SOURCES:.c=.o)
OBJECT_FILES = $(patsubst %, $(OBJ_PATH)/%, $(OBJECTS))
TEST_OBJ_FILES = $(patsubst %, $(OBJ_PATH)/test_%, $(HEADERS:.h=.o))

# test individual module passed by argument
ifeq (test, $(firstword $(MAKECMDGOALS)))
  # use first argument after test to do test and turn the others into
  # do-nothing targets
  ALL_ARGS := $(wordlist 2, $(words $(MAKECMDGOALS)), $(MAKECMDGOALS)) 
  MODULE := $(wordlist 2, 2, $(MAKECMDGOALS))
  TEST_NAME := test_$(MODULE)
  TEST_OBJ_NAME := $(OBJ_PATH)/$(TEST_NAME)
  $(eval $(ALL_ARGS):;@:)
  
# Compile, execute tests and check results of modules.
.PHONY: test  
test: obj $(TEST_OBJ_NAME).o
	@$(CC) -o $(TEST_OBJ_NAME) $(TEST_OBJ_NAME).o $(patsubst %, $(OBJ_PATH)/%, $(HEADERS:.h=.o)) $(LDFLAGS);
	@if [[ -x $(TEST_SCRIPT_PATH)/$(TEST_NAME).sh ]]; then \
		if [[ $(TEST_SCRIPT_PATH)/$(TEST_NAME).sh ]]; then \
			echo "$(MODULE) test passed"; \
		else \
			echo "$(MODULE) test failed"; \
		fi \
	else \
		if [[ $(TEST_OBJ_NAME) ]]; then \
			echo "$(MODULE) test passed"; \
		else \
			echo "$(MODULE) test failed"; \
		fi \
	fi
	@rm -f $(OBJ_PATH)/$(TEST_OBJ_NAME)
endif

# make targets with debug flags up
ifeq (debug, $(firstword $(MAKECMDGOALS)))
  # use the rest as arguments for "debug"
  DEBUG_TARGETS := $(wordlist 2, $(words $(MAKECMDGOALS)), $(MAKECMDGOALS))
  # ...and turn them into do-nothing targets
  $(eval $(DEBUG_TARGETS):;@:)
  
.PHONY: debug
debug:
	$(MAKE) --always-make DEBUG=1 $(DEBUG_TARGETS)
endif

# make final executable and all self-checking tests
.PHONY: all
all: $(EXE)

$(EXE): $(OBJECT_FILES)
	$(CC) -o $@ $^ $(LDFLAGS)

# Compile objects without linking.
.PHONY: obj
obj: $(OBJECT_FILES)

# Clean all compilation results.
.PHONY: clean
clean:
	rm -rf $(OBJ_PATH)
	rm -f $(EXE)
	@rm -f profile.txt

# Generate documentation with doxygen utility.
.PHONY: doc
doc:
	mkdir -p doc
	doxygen Doxyfile

# Include rules of object files
-include $(OBJECT_FILES:.o=.d);

# Compile source files
$(OBJ_PATH)/%.o: $(SRC_PATH)/%.c
	@mkdir -p $(OBJ_PATH)
	$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $(CFLAGS) -MQ '$@' $< -o ${@:.o=.d}

# Compile test source files
$(OBJ_PATH)/test_%.o: $(TEST_PATH)/test_%.c $(TEST_SCRIPT_PATH)/test_%.sh
	@mkdir -p $(OBJ_PATH)
	@$(CC) $(CFLAGS) -c $< -o $@
	@$(CC) -MM $(CFLAGS) -MQ '$@' $< -o ${@:.o=.d}