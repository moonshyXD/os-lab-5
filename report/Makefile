LATEX ?= xelatex

.PHONY: all clean xelatex
all: report.pdf

report.pdf: report.tex preamble.tex titlepage.tex Makefile $(wildcard inc/*.txt ../src/*.c ../src/*.h) ../Makefile
	$(LATEX) -interaction=nonstopmode -halt-on-error report.tex
	$(LATEX) -interaction=nonstopmode -halt-on-error report.tex

xelatex:
	$(MAKE) LATEX=xelatex

clean:
	rm -f report.aux report.log report.out report.lol report.toc
