CXX := g++ -std=c++20
CC := gcc

SRC_DIR := src
BUILD_DIR := build
BIN_DIR := bin

# Auto-detect source files
CPP_FILES := $(shell find $(SRC_DIR) -name "*.cpp")
C_FILES := $(shell find $(SRC_DIR) -name "*.c")
OBJ_FILES_DEBUG := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/debug/%.o,$(CPP_FILES)) \
                   $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/debug/%.o,$(C_FILES))
OBJ_FILES_RELEASE := $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/release/%.o,$(CPP_FILES)) \
                     $(patsubst $(SRC_DIR)/%.c,$(BUILD_DIR)/release/%.o,$(C_FILES))

CPPFLAGS := -I deps/spdlog/include/ -I deps/json/include/ -I deps/CLI11/include/ -I deps/cpp-linenoise/ -MMD -MP 
CFLAGS := -Wall -Wextra

DEBUG_FLAGS := -g -O0 -DDEBUG
RELEASE_FLAGS := -O3 -DNDEBUG

LIBS := -lm -lcurl -lpthread -lssl -lcrypto
LDFLAGS := 

# Output executables
TARGET_DEBUG := $(BIN_DIR)/debug/program
TARGET_RELEASE := $(BIN_DIR)/release/program

# Number of parallel jobs for make
MAKEFLAGS += -j3

# Default target
all: debug release

# Create necessary directories
$(shell mkdir -p $(BIN_DIR)/debug $(BIN_DIR)/release $(BUILD_DIR)/debug $(BUILD_DIR)/release $(addprefix $(BUILD_DIR)/debug/,$(dir $(CPP_FILES:$(SRC_DIR)/%.cpp=%))) $(addprefix $(BUILD_DIR)/release/,$(dir $(CPP_FILES:$(SRC_DIR)/%.cpp=%))))

# Debug build
debug: $(TARGET_DEBUG)

$(TARGET_DEBUG): $(OBJ_FILES_DEBUG)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LIBS)

# Release build
release: $(TARGET_RELEASE)

$(TARGET_RELEASE): $(OBJ_FILES_RELEASE)
	$(CXX) $(LDFLAGS) $^ -o $@ $(LIBS)

# Pattern rules for compilation
$(BUILD_DIR)/debug/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(BUILD_DIR)/debug/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(DEBUG_FLAGS) -c $< -o $@

$(BUILD_DIR)/release/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CPPFLAGS) $(CFLAGS) $(RELEASE_FLAGS) -c $< -o $@

$(BUILD_DIR)/release/%.o: $(SRC_DIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(RELEASE_FLAGS) -c $< -o $@

# Include dependency files
-include $(patsubst %.o,%.d,$(OBJ_FILES_DEBUG))
-include $(patsubst %.o,%.d,$(OBJ_FILES_RELEASE))

# Clean build files
clean:
	rm -rf $(BUILD_DIR)/*

# Clean all generated files
distclean: clean
	rm -rf $(BIN_DIR)/*

# Help target
help:
	@echo "Available targets:"
	@echo "  all       - Build both debug and release versions (default)"
	@echo "  debug     - Build debug version only"
	@echo "  release   - Build release version only"
	@echo "  clean     - Remove object files"
	@echo "  distclean - Remove all generated files"
	@echo "  help      - Display this help message"

# Phony targets
.PHONY: all debug release clean distclean help