NAME=presentation
TARGET=$(NAME).pdf
SOURCE=$(NAME).tex

JUNK=.aux .bbl .blg .dvi .log .nav .out .ps .pdf .snm .tex.backup .tex.bak .toc Notes.bib .spl -blx.bib .run.xml .bcf

.PHONY: clean


$(TARGET): $(SOURCE) *tex figures/*tex
	@pdflatex $(SOURCE)
	@bibtex $(NAME)
	@pdflatex $(SOURCE)
	@pdflatex $(SOURCE)

clean:
	@for ext in $(JUNK); do \
		rm -v $(NAME)$$ext; \
		rm -v $(NAME)$$ext; \
	done
