VERSION ?= v1.0
RELEASE_DATE ?= July 26, 2026
BOOK = A3C5
LATEX_INPUT = \def\BookVersion{$(VERSION)}\def\BookDate{$(RELEASE_DATE)}\input{$(BOOK).tex}
RELEASE_URL = https://github.com/alxli/algorithm-anthology/raw/master/Book/$(BOOK)-$(VERSION).pdf

.PHONY: chapters pdf release update-readme clean

all: pdf

chapters:
	python3 gen_chapters.py

pdf: chapters
	pdflatex -jobname=$(BOOK) -interaction=nonstopmode -halt-on-error "$(LATEX_INPUT)" >/dev/null || (cat $(BOOK).log && false)
	pdflatex -jobname=$(BOOK) -interaction=nonstopmode -halt-on-error "$(LATEX_INPUT)" >/dev/null || (cat $(BOOK).log && false)

release: pdf
	cp $(BOOK).pdf $(BOOK)-$(VERSION).pdf
	$(MAKE) update-readme VERSION=$(VERSION)
	@echo "Generated Book/$(BOOK).pdf and Book/$(BOOK)-$(VERSION).pdf"

update-readme:
	python3 -c 'from pathlib import Path; import re, sys; p = Path("../README.md"); s = p.read_text(); replacement = " [PDF ({0})]({1})".format(*sys.argv[1:]); s, n = re.subn(r"\s*\[PDF \([^]]+\)\]\(https://github\.com/alxli/algorithm-anthology/[^)]+\.pdf\)", replacement, s); assert n == 1, "expected exactly one versioned PDF link in README.md"; p.write_text(s)' "$(VERSION)" "$(RELEASE_URL)"

clean:
	rm -f $(BOOK).aux $(BOOK).fdb_latexmk $(BOOK).fls $(BOOK).idx $(BOOK).ilg $(BOOK).ind
	rm -f $(BOOK).listing $(BOOK).log $(BOOK).out $(BOOK).synctex.gz $(BOOK).toc
