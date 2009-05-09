#!/usr/bin/make -f

report.pdf: report.tex bio.bib
	pdflatex report
	pdflatex report
	bibtex   report
	pdflatex report

clean:
	rm -f *.log *.aux *.pdf

.PHONY: clean
