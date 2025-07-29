CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Iinclude

SRC = src/main.cpp src/storage.cpp src/schema.cpp src/table_manager.cpp
OBJ = $(SRC:.cpp=.o)
TARGET = nova

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^

clean:
	rm -f $(OBJ) $(TARGET)
