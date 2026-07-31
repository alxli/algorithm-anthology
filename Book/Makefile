VERSION = v1.0
RELEASE_DATE = July 31, 2026
BOOK = A3C5
BUILD_JOB = $(BOOK)-build
VERSIONED_PDF = $(BOOK)-$(VERSION).pdf
LATEX_INPUT = \def\BookVersion{$(VERSION)}\def\BookDate{$(RELEASE_DATE)}\input{$(BOOK).tex}
RELEASE_URL = https://github.com/alxli/algorithm-anthology/raw/master/Book/$(VERSIONED_PDF)
BUILD_AUX_FILES = $(BUILD_JOB).aux $(BUILD_JOB).fdb_latexmk $(BUILD_JOB).fls $(BUILD_JOB).idx
BUILD_AUX_FILES += $(BUILD_JOB).ilg $(BUILD_JOB).ind $(BUILD_JOB).listing $(BUILD_JOB).log
BUILD_AUX_FILES += $(BUILD_JOB).out $(BUILD_JOB).synctex.gz $(BUILD_JOB).toc
LEGACY_AUX_FILES = $(BOOK).aux $(BOOK).fdb_latexmk $(BOOK).fls $(BOOK).idx $(BOOK).ilg $(BOOK).ind
LEGACY_AUX_FILES += $(BOOK).listing $(BOOK).log $(BOOK).out $(BOOK).synctex.gz $(BOOK).toc

.PHONY: chapters pdf release update-readme clean

all: pdf

chapters:
	python3 gen_chapters.py

pdf: chapters
	rm -f $(BUILD_AUX_FILES) $(BUILD_JOB).pdf
	pdflatex -draftmode -jobname=$(BUILD_JOB) -interaction=nonstopmode -halt-on-error "$(LATEX_INPUT)" >/dev/null || (tail -100 $(BUILD_JOB).log && false)
	pdflatex -jobname=$(BUILD_JOB) -interaction=nonstopmode -halt-on-error "$(LATEX_INPUT)" >/dev/null || (tail -100 $(BUILD_JOB).log && false)
	cp $(BUILD_JOB).pdf $(BOOK).pdf
	cp $(BUILD_JOB).pdf $(VERSIONED_PDF)
	rm -f $(BUILD_JOB).pdf
	@echo "Generated Book/$(BOOK).pdf and Book/$(VERSIONED_PDF)"

release: pdf
	$(MAKE) update-readme VERSION=$(VERSION)

update-readme:
	@if grep -Fq '[PDF ($(VERSION))]($(RELEASE_URL))' ../README.md; then \
		echo "README.md already links to $(VERSION)"; \
	else \
		python3 -c 'from pathlib import Path; import re, sys; p = Path("../README.md"); s = p.read_text(); replacement = "[PDF ({0})]({1})".format(*sys.argv[1:]); s, n = re.subn(r"\[PDF \([^]]+\)\]\(https://github\.com/alxli/algorithm-anthology/[^)]+\.pdf\)", replacement, s); assert n == 1, "expected exactly one versioned PDF link in README.md"; p.write_text(s)' "$(VERSION)" "$(RELEASE_URL)"; \
	fi

clean:
	rm -f $(BUILD_AUX_FILES) $(LEGACY_AUX_FILES) $(BUILD_JOB).pdf
