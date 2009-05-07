#!/usr/bin/make -f

proposal.pdf: proposal.tex bio.bib
	pdflatex proposal
	pdflatex proposal
	bibtex   proposal
	pdflatex proposal

clean:
	rm -f *.log *.aux *.pdf

.PHONY: clean
