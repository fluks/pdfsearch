bin := pdfsearch
CXX := g++
pkg_config_cflags := $(shell pkg-config --cflags poppler-cpp)
src_dir := src/
CFLAGS := -g -std=c++11 -Wall -Wextra -pedantic $(pkg_config_cflags) -I$(src_dir)
DEBUG ?= no
ifeq ($(DEBUG), yes)
	CFLAGS += -O0
else
	CFLAGS += -O2 -DNDEBUG
endif
LDLIBS := $(shell pkg-config --libs poppler-cpp)

sources := $(wildcard $(src_dir)*.cpp)
objects := $(sources:.cpp=.o)

PREFIX := $(abspath build)
prefix_file := prefix

.PHONY: all ctags clean

all: $(objects)
	$(CXX) $(CFLAGS) -o $(bin) $(objects) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -o $@ $< $(LDLIBS)

clean:
	-$(RM) *.o $(bin)

ctags:
	ctags -o tags -R --c++-kinds=+p --fields=+iaS --extra=+q $(src_dir) $(pkg_config_cflags)
