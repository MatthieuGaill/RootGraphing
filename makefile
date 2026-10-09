CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -fPIC -Iinclude $(shell root-config --cflags)
DEPFLAGS = -MMD -MP
LDLIBS = $(shell root-config --libs)

LIB_SRCS = $(wildcard src/*.cpp)
OBJDIR = build/obj
LIB_OBJS = $(patsubst %.cpp,$(OBJDIR)/%.o,$(LIB_SRCS))
MAIN_OBJ = $(OBJDIR)/main.o
DEPS = $(LIB_OBJS:.o=.d) $(MAIN_OBJ:.o=.d)

TARGET = build/smart_canvas
STATIC_LIB = build/lib/libSmartCanvas.a
SHARED_LIB = build/lib/libSmartCanvas.so
# Where "make install" copies the headers and libraries
PREFIX ?= $(HOME)/SmartCanvas

.PHONY: all lib run install clean

all: $(TARGET) lib

lib: $(STATIC_LIB) $(SHARED_LIB)

$(TARGET): $(MAIN_OBJ) $(STATIC_LIB)
	$(CXX) -o $@ $^ $(LDLIBS)

$(STATIC_LIB): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	ar rcs $@ $^

$(SHARED_LIB): $(LIB_OBJS)
	@mkdir -p $(dir $@)
	$(CXX) -shared -o $@ $^ $(LDLIBS)

$(OBJDIR)/%.o: %.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(DEPFLAGS) -c $< -o $@

-include $(DEPS)

run: $(TARGET)
	./$(TARGET)

# Headers go in a SmartCanvas/ subdirectory to avoid name clashes: #include "SmartCanvas/SmartCanvas.hh"
install: lib
	mkdir -p $(PREFIX)/include/SmartCanvas $(PREFIX)/lib
	cp include/*.hh $(PREFIX)/include/SmartCanvas/
	cp $(STATIC_LIB) $(SHARED_LIB) $(PREFIX)/lib/

clean:
	rm -rf build
