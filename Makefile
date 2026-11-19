CFLAGS ?= -O2 -std=c11 -Wall -Wextra -Werror -Wformat=2
PROGRAM ?= runtime

all: build/libforward.so build/libcentral.so build/linked build/runtime
run: all
	./build/$(PROGRAM)
strace: all
	strace -f -tt -T -yy -s 128 -o build/$(PROGRAM).trace ./build/$(PROGRAM)
report:
	$(MAKE) -C report
clean:
	rm -rf build
	$(MAKE) -C report clean
.PHONY: all run strace report clean

build:
	mkdir -p build

build/libforward.so: src/forward.c src/contracts.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -shared $(LDFLAGS) -Wl,-soname,libforward.so -o build/libforward.so src/forward.c -lm

build/libcentral.so: src/central.c src/contracts.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -shared $(LDFLAGS) -Wl,-soname,libcentral.so -o build/libcentral.so src/central.c -lm

build/linked: src/linked.c src/commands.c src/commands.h src/contracts.h build/libforward.so
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o build/linked src/linked.c src/commands.c -Lbuild -lforward -Wl,-rpath,'$$ORIGIN'

build/runtime: src/runtime.c src/commands.c src/commands.h src/contracts.h | build
	$(CC) $(CPPFLAGS) $(CFLAGS) $(LDFLAGS) -o build/runtime src/runtime.c src/commands.c -ldl
