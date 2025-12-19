# Project settings
TARGET = main
SRC = src/main.c

# Build output directory
BUILD_DIR = build

# Final output
OUT = $(BUILD_DIR)/$(TARGET).ihx

# Tools
SDCC = sdcc
SDCCFLAGS = --model-small --std-c23 --opt-code-size --xram-loc 0x8000 --xram-size 0x2000

# Default target
all: $(OUT)

# Ensure build directory exists
$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

# Build target
$(OUT): $(SRC) | $(BUILD_DIR)
	$(SDCC) $(SDCCFLAGS) $(SRC) -o $(OUT)

# Clean build artifacts
clean:
	rm -rf $(BUILD_DIR)
