VERSION = v1.0
RELEASE_DATE = August 20, 2026
BOOK = A3C5
BUILD_DIR = build
VERSIONED_PDF = $(BOOK)-$(VERSION).pdf
LATEX_INPUT = \def\BookVersion{$(VERSION)}\def\BookDate{$(RELEASE_DATE)}\input{$(BOOK).tex}
RELEASE_URL = https://github.com/alxli/algorithm-anthology/raw/master/Book/$(VERSIONED_PDF)
# Every auxiliary file lands in $(BUILD_DIR), so only sources and the two published PDFs sit beside
# them. The explicit -jobname is required because the input is a command line rather than a file
# name, which would otherwise make TeX call everything "texput".
PDFLATEX = pdflatex -output-directory=$(BUILD_DIR) -jobname=$(BOOK) -interaction=nonstopmode -halt-on-error
BUILD_LOG = $(BUILD_DIR)/$(BOOK).log
# Auxiliary files left beside the sources by an older in-place build, or by an editor that compiles
# $(BOOK).tex on its own. Removed by `make clean` so the directory settles into the new layout.
STRAY_FILES = $(BOOK).aux $(BOOK).fdb_latexmk $(BOOK).fls $(BOOK).idx $(BOOK).ilg $(BOOK).ind
STRAY_FILES += $(BOOK).listing $(BOOK).log $(BOOK).out $(BOOK).synctex.gz $(BOOK).toc
STRAY_FILES += "$(BOOK).synctex(busy)" $(BOOK)-build.*

.PHONY: all chapters pdf release update-readme clean

all: pdf

chapters:
	python3 gen_chapters.py

pdf: chapters
	mkdir -p $(BUILD_DIR)
	$(PDFLATEX) -draftmode "$(LATEX_INPUT)" >/dev/null || (tail -100 $(BUILD_LOG) && false)
	$(PDFLATEX) "$(LATEX_INPUT)" >/dev/null || (tail -100 $(BUILD_LOG) && false)
	cp $(BUILD_DIR)/$(BOOK).pdf $(BUILD_DIR)/publish.tmp && mv -f $(BUILD_DIR)/publish.tmp $(BOOK).pdf
	cp $(BUILD_DIR)/$(BOOK).pdf $(BUILD_DIR)/publish.tmp && mv -f $(BUILD_DIR)/publish.tmp $(VERSIONED_PDF)
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
	rm -rf $(BUILD_DIR)
	rm -f $(STRAY_FILES)
