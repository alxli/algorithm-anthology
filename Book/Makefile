.PHONY: chapters pdf clean

all: pdf

chapters:
	python3 gen_chapters.py

pdf: chapters
	pdflatex -jobname=A3C5 -interaction=nonstopmode -halt-on-error A3C5.tex >/dev/null || (cat A3C5.log && false)

clean:
	rm -f A3C5.aux A3C5.fdb_latexmk A3C5.fls A3C5.idx A3C5.ilg A3C5.ind
	rm -f A3C5.listing A3C5.log A3C5.out A3C5.synctex.gz A3C5.toc
