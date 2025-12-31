# Compiler and flags
CC = clang
CXX = clang++
CFLAGS = -Wall -Wextra -std=c11 -Iengine/include -Ithird_party
CXXFLAGS = -Wall -Wextra -std=c++17 -Iengine/include -Ithird_party
LDFLAGS = -lglfw -lwebgpu

# Platform-specific settings
UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
    LDFLAGS += -framework Cocoa -framework QuartzCore -framework Metal -framework CoreGraphics -framework Foundation
    CFLAGS += -I/opt/homebrew/include -I/usr/local/include
    CXXFLAGS += -I/opt/homebrew/include -I/usr/local/include
    LDFLAGS += -L/opt/homebrew/lib -L/usr/local/lib -Wl,-rpath,/usr/local/lib -Wl,-rpath,/opt/homebrew/lib
endif

ifeq ($(UNAME_S),Linux)
    LDFLAGS += -lX11 -lm
endif

# Directories
ENGINE_SRC_DIR = engine/src
ENGINE_INC_DIR = engine/include
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/lib
OBJ_DIR = $(BUILD_DIR)/obj

# Engine library
ENGINE_LIB = $(LIB_DIR)/libungrund.a
ENGINE_SRCS = $(wildcard $(ENGINE_SRC_DIR)/*.c)
ENGINE_OBJS = $(patsubst $(ENGINE_SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(ENGINE_SRCS))

# Platform-specific engine sources
ifeq ($(UNAME_S),Darwin)
    ENGINE_OBJC_SRCS = $(wildcard $(ENGINE_SRC_DIR)/*.m)
    ENGINE_OBJC_OBJS = $(patsubst $(ENGINE_SRC_DIR)/%.m,$(OBJ_DIR)/%.o,$(ENGINE_OBJC_SRCS))
    ENGINE_OBJS += $(ENGINE_OBJC_OBJS)
endif

# Examples
TRIANGLE_EXAMPLE = $(BUILD_DIR)/triangle
TEXT_EXAMPLE = $(BUILD_DIR)/text_render
PONG_EXAMPLE = $(BUILD_DIR)/pong
INPUT_CALLBACKS_EXAMPLE = $(BUILD_DIR)/input_callbacks
GEOMETRY_DEMO_EXAMPLE = $(BUILD_DIR)/geometry_demo

# Default target
.PHONY: all
all: engine examples

# Engine library target
.PHONY: engine
engine: $(ENGINE_LIB)

$(ENGINE_LIB): $(ENGINE_OBJS) | $(LIB_DIR)
	ar rcs $@ $^
	@echo "Built engine library: $@"

$(OBJ_DIR)/%.o: $(ENGINE_SRC_DIR)/%.c | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: $(ENGINE_SRC_DIR)/%.m | $(OBJ_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Examples target
.PHONY: examples
examples: triangle text pong input_callbacks geometry_demo

# Triangle example
.PHONY: triangle
triangle: $(TRIANGLE_EXAMPLE)

$(TRIANGLE_EXAMPLE): examples/triangle/main.c $(ENGINE_LIB) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -lungrund $(LDFLAGS)
	@echo "Built triangle example: $@"

# Text rendering example
.PHONY: text
text: $(TEXT_EXAMPLE)

$(TEXT_EXAMPLE): examples/text_render/main.c $(ENGINE_LIB) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -lungrund $(LDFLAGS)
	@echo "Built text rendering example: $@"

# Pong game example
.PHONY: pong
pong: $(PONG_EXAMPLE)

$(PONG_EXAMPLE): examples/pong/main.c $(ENGINE_LIB) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -lungrund $(LDFLAGS)
	@echo "Built pong game example: $@"

# Input callbacks example
.PHONY: input_callbacks
input_callbacks: $(INPUT_CALLBACKS_EXAMPLE)

$(INPUT_CALLBACKS_EXAMPLE): examples/input_callbacks/main.c $(ENGINE_LIB) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -lungrund $(LDFLAGS)
	@echo "Built input callbacks example: $@"

# Geometry demo example
.PHONY: geometry_demo
geometry_demo: $(GEOMETRY_DEMO_EXAMPLE)

$(GEOMETRY_DEMO_EXAMPLE): examples/geometry_demo/main.c $(ENGINE_LIB) | $(BUILD_DIR)
	$(CC) $(CFLAGS) $< -o $@ -L$(LIB_DIR) -lungrund $(LDFLAGS)
	@echo "Built geometry demo example: $@"

# Run targets
.PHONY: run-triangle
run-triangle: $(TRIANGLE_EXAMPLE)
	@echo "Running triangle example..."
	./$(TRIANGLE_EXAMPLE)

.PHONY: run-text
run-text: $(TEXT_EXAMPLE)
	@echo "Running text rendering example..."
	./$(TEXT_EXAMPLE)

.PHONY: run-pong
run-pong: $(PONG_EXAMPLE)
	@echo "Running pong game..."
	./$(PONG_EXAMPLE)

.PHONY: run-input
run-input: $(INPUT_CALLBACKS_EXAMPLE)
	@echo "Running input callbacks example..."
	./$(INPUT_CALLBACKS_EXAMPLE)

.PHONY: run-geometry
run-geometry: $(GEOMETRY_DEMO_EXAMPLE)
	@echo "Running geometry demo..."
	./$(GEOMETRY_DEMO_EXAMPLE)

# Create directories
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(LIB_DIR):
	mkdir -p $(LIB_DIR)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

# Clean
.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

# Help
.PHONY: help
help:
	@echo "Ungrund Game Engine - Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all             - Build engine library and all examples (default)"
	@echo "  engine          - Build only the engine library"
	@echo "  examples        - Build all examples"
	@echo "  triangle        - Build triangle rendering example"
	@echo "  text            - Build text rendering example"
	@echo "  pong            - Build pong game example"
	@echo "  input_callbacks - Build input callbacks example"
	@echo "  run-triangle    - Build and run triangle example"
	@echo "  run-text        - Build and run text rendering example"
	@echo "  run-pong        - Build and run pong game"
	@echo "  run-input       - Build and run input callbacks example"
	@echo "  clean           - Remove all build artifacts"
	@echo "  help            - Show this help message"

