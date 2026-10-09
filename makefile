CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Iinclude $(shell root-config --cflags)
DEPFLAGS = -MMD -MP
LDLIBS = $(shell root-config --libs)
SRCS = $(wildcard src/*.cpp) main.cpp
OBJDIR = build/obj
OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
TARGET = build/smart_canvas

.PHONY: all run clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	./$(TARGET)

clean:
	rm -rf build
