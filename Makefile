PROGRAM ?= runtime
TRACE = strace -f -tt -T -yy -s 128

all:
	$(MAKE) -C src

run: all
	./build/$(PROGRAM)

strace: all
	$(TRACE) -o build/$(PROGRAM).trace ./build/$(PROGRAM)

report:
	$(MAKE) -C report

clean:
	$(MAKE) -C src clean
	$(MAKE) -C report clean

.PHONY: all run strace report clean
