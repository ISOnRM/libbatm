# tools
CC ?= cc
AR ?= ar

# flags 
CFLAGS         ?= -std=c99 -Wall -Wextra -Wpedantic -fPIC

MODE ?= release
ifeq ($(MODE),debug)
	OPTFLAGS := -g -O0 -DDEBUG
else
	OPTFLAGS :=    -O2 -DNDEBUG
endif

CPPFLAGS := -Iinclude


#lists

BUILD ?= build

SRCS := $(wildcard src/*.c)
OBJS := $(SRCS:src/%.c=$(BUILD)/%.o)
DEPS := $(OBJS:.o=.d)

# install stuff
PREFIX     ?= /usr/local
#BINDIR     ?= $(PREFIX)/bin
LIBDIR     ?= $(PREFIX)/lib
INCLUDEDIR ?= $(PREFIX)/include

.PHONY: all clean

all: $(BUILD)/libbatm.a $(BUILD)/libbatm.so.0.1.0

clean:
	rm -rf build

install: all
#	install -d $(DESTDIR)$(BINDIR)
#   install -m 755 $(BUILD)/batm $(DESTDIR)$(BINDIR)/

	install -d $(DESTDIR)$(LIBDIR)
	install -m 644 $(BUILD)/libbatm.a            $(DESTDIR)$(LIBDIR)/
	install -m 755 $(BUILD)/libbatm.so.0.1.0     $(DESTDIR)$(LIBDIR)/
	cd $(DESTDIR)$(LIBDIR) && ln -sf libbatm.so.0.1.0 libbatm.so.0
	cd $(DESTDIR)$(LIBDIR) && ln -sf libbatm.so.0     libbatm.so

	install -d $(DESTDIR)$(INCLUDEDIR)/batm
	install -m 644 include/batm/batm.h $(DESTDIR)$(INCLUDEDIR)/batm/
	@if [ -z "$(DESTDIR)" ]; then ldconfig; fi


$(BUILD):
	mkdir -p $@

# objects

$(BUILD)/%.o: src/%.c | $(BUILD)
	$(CC) $(CFLAGS) $(CPPFLAGS) $(OPTFLAGS)  -MMD -MP -MF $(@:.o=.d) -MT $@ -c $< -o $@

-include $(DEPS)

# static
$(BUILD)/libbatm.a: $(OBJS) | $(BUILD)
	$(AR) rcs $@ $^

# so
$(BUILD)/libbatm.so.0.1.0: $(OBJS) | $(BUILD)
	$(CC) -shared -Wl,-soname,libbatm.so.0 $^ -o $@
	cd $(BUILD) && ln -sf libbatm.so.0.1.0 libbatm.so.0
	cd $(BUILD) && ln -sf libbatm.so.0 libbatm.so
