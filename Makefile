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

PREFIX := /usr/local/
prefix_file := prefix
# Read and use the prefix, defined in install, on uninstall.
ifeq (uninstall, $(MAKECMDGOALS))
	PREFIX := $(if $(wildcard $(prefix_file)), \
		$(shell read -r line < $(prefix_file) && echo $$line), \
		$(PREFIX))
endif

test_dir := tests/
test_bin := $(test_dir)test
test_sources := $(wildcard $(test_dir)*.cpp)
test_objects := $(test_sources:.cpp=.o)

ifeq ($(MAKECMDGOALS), check)
	override CFLAGS += -I$(test_dir)
endif

.PHONY: all ctags clean check check_compile_only clean_check cppcheck doc man \
	install uninstall reset

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
	$(CXX) $(CFLAGS) -o $(test_bin) $(filter-out %main.o, $(objects)) \
		$(test_objects) $(LDFLAGS) $(LDLIBS)
	if [ "$(run_check)" != no ]; then \
		$(test_bin); \
	fi

check_compile_only: run_check := no
check_compile_only: check

$(test_dir)%.o: %.cpp
	$(CXX) $(CFLAGS) -c -o $@ $<

clean_check:
	-$(RM) $(test_dir)/*.o $(test_dir)/*.d $(test_bin)

cppcheck:
	cppcheck --enable=warning,information --suppress=missingIncludeSystem $(src_dir)

doc:
	@doxygen

man:
	@pod2man --release --center 'General Commands Manual' doc/pdfsearch.pod doc/pdfsearch.1

install:
	# Save original config.h.
	[ ! -e "$(src_dir)/config.h.orig" ] && cp $(src_dir)/config.h $(src_dir)/config.h.orig
	sed -i \
		-e 's/\(CONFIG_FILE =\).*/\1 "$(subst /,\/,$(HOME))\/.$(bin)\/$(bin).conf";/' \
		-e 's/\(DATABASE_FILE =\).*/\1 "$(subst /,\/,$(HOME))\/.$(bin)\/$(bin).sqlite";/' \
			$(src_dir)/config.h
	make
	echo $(PREFIX) > $(prefix_file)
	mkdir -p $(PREFIX)/bin
	cp $(bin) $(PREFIX)/bin
	-mkdir $(HOME)/.$(bin)
	cp $(src_dir)/dummy.conf $(HOME)/.$(bin)/$(bin).conf
	-mkdir -p $(PREFIX)/man/man1
	-cp doc/$(bin).1 $(PREFIX)/man/man1/

uninstall:
	$(RM) $(PREFIX)/bin/$(bin)
	$(RM) $(PREFIX)/man/man1/$(bin).1

reset:
	[ -e "$(src_dir)/config.h.orig" ] && mv $(src_dir)/config.h.orig $(src_dir)/config.h
