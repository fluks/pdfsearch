bin := pdfsearch
CXX := g++
pkg_config_cflags := $(shell pkg-config --cflags poppler-cpp sqlite3)
src_dir := src/
override CFLAGS += -std=c++11 -Wall -Wextra -pedantic -I$(src_dir) \
	$(pkg_config_cflags)
# getopt_long()
override CFLAGS += -D_GNU_SOURCE
# Update dependencies on compile.
override CFLAGS += -MP -MMD
DEBUG ?= no
ifeq ($(DEBUG), yes)
	override CFLAGS += -O0 -g3
else
	override CFLAGS += -O2 -DNDEBUG
endif
LDLIBS := $(shell pkg-config --libs-only-l poppler-cpp sqlite3) -lboost_regex \
	-lboost_system -lboost_filesystem
LDFLAGS := $(shell pkg-config --libs-only-L poppler-cpp sqlite3)
# Can't find new version of libpoppler-cpp at runtime without this.
LDFLAGS += -Wl,--rpath,/usr/local/lib

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
	override CFLAGS += $(shell pkg-config --cflags check)
endif

.PHONY: all ctags clean check clean_check cppcheck doc

all: $(objects)
	$(CXX) $(CFLAGS) -o $(bin) $(objects) $(LDFLAGS) $(LDLIBS)

# Include dependencies. Needs to be after default goal.
-include $(sources:.cpp=.d)

%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

clean: clean_check
	-$(RM) $(src_dir)/*.o $(bin) $(src_dir)/*.d

ctags:
	ctags -a -o tags -R --c++-kinds=+p --fields=+iaS --extra=+q \
		$(src_dir) $(subst -I,, $(pkg_config_cflags)) /usr/include/sqlite3.h \
		/usr/include/boost/filesystem/ /usr/include/boost/regex/ /usr/include/boost/system/

check: $(objects) $(test_objects)
	$(CXX) $(CFLAGS) -o $(test_bin) $(test_objects) \
		$(filter-out %main.o, $(objects)) $(LDLIBS)
	$(test_bin)

$(test_dir)%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

clean_check:
	-$(RM) $(test_dir)/*.o $(test_bin)

cppcheck:
	cppcheck --enable=warning,information --suppress=missingIncludeSystem $(src_dir)

doc:
	doxygen
