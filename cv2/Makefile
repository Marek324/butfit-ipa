CXX = g++
#Option #1: set path for opencv

#INCLUDE_DIR = /usr/include/opencv4  # <---- ADJUST THIS PATH (VERIFY!)
#LIB_DIR = /usr/lib/x86_64-linux-gnu  # <---- ADJUST THIS PATH (VERIFY!)  (pkg-config should handle this now)
#LIBS = -lopencv_core -lopencv_imgproc -lopencv_highgui
#LDFLAGS = -L$(LIB_DIR) $(LIBS)
#CXXFLAGS = -std=c++17 -O2 -mavx2 -I$(INCLUDE_DIR)


#Option #2: set path for opencv
LDFLAGS = `pkg-config --libs opencv4`
CXXFLAGS = -std=c++17 -O2 -mavx2 `pkg-config --cflags opencv4`

TARGET = cv2
SRC = cv2_main.cpp cv2_student.s  # Added cv2_student.s to SRC
OBJ = build/cv2_main.o build/cv2_student.o  # Added cv2_student.o to OBJ

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) -o $@ $^ $(LDFLAGS)


# Rule to compile C++ source files (.cpp)
build/cv2_main.o: cv2_main.cpp
	$(CXX) $(CXXFLAGS) -g -c $< -o $@

# Rule to assemble assembly source files (.s)
build/cv2_student.o: cv2_student.s
	$(CXX) -g -c $< -o $@  # No CXXFLAGS needed for assembly typically

clean:
	rm -f $(TARGET) $(OBJ)

run: $(TARGET)
	./$(TARGET)

.PHONY: all clean run