CC ?= gcc
CFLAGS ?= -O2
WARNINGS = -Wall -Wextra -Werror -Wformat=2
BUILD = ../build
LIBS = $(BUILD)/libforward.so $(BUILD)/libcentral.so

.PHONY: all clean
all: $(LIBS) $(BUILD)/linked $(BUILD)/runtime

$(BUILD):
	mkdir -p $@

$(BUILD)/lib%.so: %.c contracts.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=c11 -fPIC -shared $(LDFLAGS) -Wl,-soname,$(@F) -o $@ $< -lm

$(BUILD)/commands.o: commands.c commands.h contracts.h | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=c11 -Wpedantic -c -o $@ $<

$(BUILD)/linked: linked.c $(BUILD)/commands.o $(BUILD)/libforward.so
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=c11 $(LDFLAGS) -o $@ $< $(BUILD)/commands.o -L$(BUILD) -lforward -Wl,-rpath,'$$ORIGIN'

$(BUILD)/runtime: runtime.c $(BUILD)/commands.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $(WARNINGS) -std=c11 $(LDFLAGS) -o $@ $^ -ldl

clean:
	rm -rf $(BUILD)
