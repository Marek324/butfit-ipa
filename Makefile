# Makefile for boat_sim project

# Compiler and flags
CXX = g++
CXXFLAGS = -Wall -g  -mavx -msse4 # -Wall for more warnings, -g for debugging symbols
INC_DIR = include
LIBS = -lglut  -lSOIL -lGL -lGLEW -lGLU
LIB_DIRS = /usr/lib /usr/lib/x86_64-linux-gnu

LDFLAGS = $(addprefix -L, $(LIB_DIRS))

# Directories
SRC_DIR = src
BUILD_DIR = build

# Source files - includes .cpp and .s from src and src_project
SOURCES = $(wildcard $(SRC_DIR)/*.cpp) $(wildcard $(SRC_DIR)/src_project/*.cpp) $(wildcard $(SRC_DIR)/*.s) $(wildcard $(SRC_DIR)/src_project/*.s)

# Object files in build directory - creates corresponding object file paths in build dir
OBJECTS = $(patsubst $(SRC_DIR)/%.cpp,$(BUILD_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(SOURCES)))
OBJECTS += $(patsubst $(SRC_DIR)/%.s,$(BUILD_DIR)/%.o,$(filter $(SRC_DIR)/%.s,$(SOURCES)))


EXECUTABLE = boat_sim
EXECUTABLE_PATH = $(EXECUTABLE)

# Default target
all: $(EXECUTABLE_PATH)

# Rule to create the executable in build dir
$(EXECUTABLE_PATH): $(OBJECTS)
	$(CXX) -o $@ $^ $(LDFLAGS) $(LIBS)

# General rule to compile/assemble source files to object files in build dir
# For .cpp files in src directory
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INC_DIR) -c $< -o $@



# For .s files in src directory - USING 'as' directly
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.s
	$(CXX) -g -c $< -o $@ 



# Clean target
clean:
	rm  $(BUILD_DIR)/*
	rm  $(EXECUTABLE_PATH)

# Debug build target
debug: CXXFLAGS += -DDEBUG -g

# Run target
run: all
	./$(EXECUTABLE_PATH)

# Run debug target
rundebug: debug
	gdb ./$(EXECUTABLE_PATH)

.PHONY: all clean debug run rundebug
