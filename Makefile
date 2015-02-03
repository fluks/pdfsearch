bin := pdfsearch
CXX := g++
pkg_config_cflags := $(shell pkg-config --cflags poppler-cpp sqlite3)
src_dir := src/
CFLAGS := -g -std=c++11 -Wall -Wextra -pedantic -I$(src_dir) \
	$(pkg_config_cflags)
# getopt_long()
CFLAGS += -D_GNU_SOURCE
DEBUG ?= no
ifeq ($(DEBUG), yes)
	CFLAGS += -O0
else
	CFLAGS += -O2 -DNDEBUG
endif
LDLIBS := $(shell pkg-config --libs poppler-cpp sqlite3) -lboost_regex \
	-lboost_system -lboost_filesystem

sources := $(wildcard $(src_dir)*.cpp)
objects := $(sources:.cpp=.o)

PREFIX := $(abspath build)
prefix_file := prefix

test_dir := tests/
test_bin := $(test_dir)test
test_sources := $(wildcard $(test_dir)*.cpp)
test_objects := $(test_sources:.cpp=.o)

ifeq ($(MAKECMDGOALS), check)
	LDLIBS += $(shell pkg-config --libs check)
	CFLAGS += $(shell pkg-config --cflags check)
endif

.PHONY: all ctags clean check clean_check

all: $(objects)
	$(CXX) $(CFLAGS) -o $(bin) $(objects) $(LDLIBS)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $< 

clean: clean_check
	-$(RM) $(src_dir)/*.o $(bin)

ctags:
	ctags -a -o tags -R --c++-kinds=+p --fields=+iaS --extra=+q \
		$(src_dir) $(subst -I,, $(pkg_config_cflags)) /usr/include/sqlite3.h

check: $(objects) $(test_objects)
	$(CXX) $(CFLAGS) -o $(test_bin) $(test_objects) \
		$(filter-out %main.o, $(objects)) $(LDLIBS)
	$(test_bin)

$(test_dir)%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

clean_check:
	-$(RM) $(test_dir)/*.o $(test_bin)
