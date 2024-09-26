# Makefile for building tint_sdl with Emscripten

# Compiler and flags
EMCC = em++
CXXFLAGS = -I. -L. -ltint -lm -sMODULARIZE=1 -sEXPORT_ES6=1 -sENVIRONMENT=web -sSTACK_SIZE=262144 
CXXFLAGS += -sNO_DISABLE_EXCEPTION_CATCHING
EXPORTED_FUNCS = -sEXPORTED_FUNCTIONS='[ "_SPV_TO_SPVASM", "_SPV_TO_WGSL", "_WGSL_TO_SPV", "_WGSL_TO_SPVASM", "_SPVASM_TO_WGSL", "_GetSPIRVSize", "_SPVASM_TO_SPV", "_malloc", "_free" ]' -sEXPORTED_RUNTIME_METHODS='["UTF8ToString", "stringToUTF8", "lengthBytesUTF8"]'

#  

# Input and output files
SRC = tint_wasm.cpp
BUILD_DIR = build
OUT = $(BUILD_DIR)/tint.html

# Preload files
SHELL_FILE = --shell-file ./shell.html

# Default target
all: $(BUILD_DIR) $(OUT)

# Create the build directory
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build target
$(OUT): $(SRC)
	$(EMCC) $(SRC) $(CXXFLAGS) $(EXPORTED_FUNCS) $(SHELL_FILE) -o $(OUT)

# Clean target
clean:
	rm -rf $(BUILD_DIR)

.PHONY: all clean

