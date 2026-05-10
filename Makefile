.PHONY: init build start clean

BUILD_DIR := build
BINARY := $(BUILD_DIR)/dungeon

init:
	cmake -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR)

start: build
	./$(BINARY)

clean:
	rm -rf $(BUILD_DIR)
