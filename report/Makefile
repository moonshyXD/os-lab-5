TEX = xelatex

.PHONY: all clean
all: report.pdf

report.pdf: report.tex preamble.tex titlepage.tex $(wildcard inc/*.txt) ../src/runtime.c ../src/contracts.h
	$(TEX) -interaction=nonstopmode -halt-on-error report.tex
	$(TEX) -interaction=nonstopmode -halt-on-error report.tex

clean:
	rm -f report.aux report.log report.out
