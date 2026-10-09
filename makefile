CXX = g++
CXXFLAGS = -std=c++17 -Iinclude $(shell root-config --cflags)
DEPFLAGS = -MMD -MP
LDFLAGS = $(shell root-config --libs)
SRCS = $(wildcard src/*.cpp) main.cpp
OBJS = $(SRCS:.cpp=.o)
DEPS = $(OBJS:.o=.d)
TARGET = build/smart_canvas

all: $(TARGET)

$(TARGET): $(OBJS)
	mkdir -p build
	$(CXX) -o $@ $^ $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

clean:
	rm -f src/*.o *.o src/*.d *.d $(TARGET)