CXX = g++
# CXXFLAGS = -std=c++14 -g -Wall -Wshadow -Wextra -fsanitize=address -fsanitize=undefined
CXXFLAGS = -std=c++14 -O3

C_FILES = $(wildcard src/*.cpp)
H_FILES = $(wildcard src/*.h)
O_FILES = $(C_FILES:src/%.cpp=build/%.o)

TARGET_APP = solver4

.PHONY = all clean
.DEFAULT = all

all: $(TARGET_APP)

build:
	@mkdir -p build

build/%.o: src/%.cpp $(H_FILES) | build
	$(CXX) $(CXXFLAGS) -c $< -o $@

$(TARGET_APP): $(O_FILES)
	$(CXX) $(CXXFLAGS) -o $@ $^	

clean:
	-rm -f $(TARGET_APP) 
	-rm -rf build
