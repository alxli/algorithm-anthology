.PHONY: chapters pdf clean

chapters:
	python3 gen_chapters.py

pdf: chapters
	latexmk -pdf -interaction=nonstopmode -halt-on-error a3c5.tex

clean:
	latexmk -C a3c5.tex
