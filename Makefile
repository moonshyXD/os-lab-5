CC ?= gcc
CFLAGS ?= -O2
WARNINGS = -Wall -Wextra -Werror -Wformat=2
BUILD = build
LIBS = $(BUILD)/libforward.so $(BUILD)/libcentral.so

.PHONY: all check clean report
all: $(LIBS) $(BUILD)/linked $(BUILD)/runtime

$(BUILD):
	mkdir -p $@

$(BUILD)/lib%.so: src/%.c src/contracts.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=c11 -fPIC -shared $(LDFLAGS) -Wl,-soname,$(@F) -o $@ $< -lm

$(BUILD)/commands.o: src/commands.c src/commands.h src/contracts.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=c11 -Wpedantic -c -o $@ $<

$(BUILD)/linked: src/linked.c $(BUILD)/commands.o $(BUILD)/libforward.so
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=c11 $(LDFLAGS) -o $@ $< $(BUILD)/commands.o -L$(BUILD) -lforward -Wl,-rpath,'$$ORIGIN'

$(BUILD)/runtime: src/runtime.c $(BUILD)/commands.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=c11 $(LDFLAGS) -o $@ $^ -ldl

TRACE = strace -f -tt -T -yy -z -s 128 -e trace=execve,openat,read,mmap,mprotect,close,munmap,write,exit_group

check: all
	printf '1 0 0.5\n2 1 100\n' | $(TRACE) -o $(BUILD)/linked.trace ./$(BUILD)/linked > $(BUILD)/linked.log
	printf '1 0 0.5\n0\n1 0 0.5\n2 1 100\n0\n1 0 0.5\n' | $(TRACE) -o $(BUILD)/runtime.trace ./$(BUILD)/runtime > $(BUILD)/runtime.log
	printf '%s\n' '-0.24483487' '25' | diff -u - $(BUILD)/linked.log
	printf '%s\n' '-0.24483487' 'Active library: ./build/libcentral.so' '0' '25' 'Active library: ./build/libforward.so' '-0.24483487' | diff -u - $(BUILD)/runtime.log

report:
	$(MAKE) -C report

clean:
	rm -rf $(BUILD)
	$(MAKE) -C report clean
