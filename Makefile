CC ?= gcc
CFLAGS ?= -O2
WARNINGS = -Wall -Wextra -Werror -Wformat=2
BUILD = build
LIBS = $(BUILD)/libforward.so $(BUILD)/libcentral.so

PROGRAM ?= runtime
TRACE = strace -f -tt -T -yy -s 128

all: $(LIBS) $(BUILD)/linked $(BUILD)/runtime

run: all
	./$(BUILD)/$(PROGRAM)

strace: all
	$(TRACE) -o $(BUILD)/$(PROGRAM).trace ./$(BUILD)/$(PROGRAM)

report:
	$(MAKE) -C report

clean:
	rm -rf $(BUILD)
	$(MAKE) -C report clean

.PHONY: all run strace report clean

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
