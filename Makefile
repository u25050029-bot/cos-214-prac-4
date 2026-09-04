CXX      := g++
CXXFLAGS := -std=c++11 -Wall -Wextra -g -Isrc
LDFLAGS  :=

TARGET   := taskforge

SRC_DIR   := src
BUILD_DIR := build

SOURCES := main.cpp $(wildcard $(SRC_DIR)/*.cpp)
OBJECTS := $(patsubst %.cpp,$(BUILD_DIR)/%.o,$(notdir $(SOURCES)))

vpath %.cpp . $(SRC_DIR)

.PHONY: all clean run debug memcheck

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: %.cpp | $(BUILD_DIR)
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

run: $(TARGET)
	./$(TARGET)

debug: $(TARGET)
	gdb ./$(TARGET)

memcheck: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all ./$(TARGET)

clean:
	rm -rf $(BUILD_DIR) $(TARGET)