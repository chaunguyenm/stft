NAME=stfft_Nguyen-Ponce
TARGET=$(NAME).pdf
SOURCE=$(NAME).tex

DATE=`date +"%Y.%m%d"`
TARBALL=$(NAME)-$(DATE).tar.gz
JUNK=.aux .bbl .blg .dvi .log .nav .out .ps .pdf .snm .tex.backup .tex.bak .toc Notes.bib .spl

.PHONY: clean


$(TARGET): $(SOURCE) *tex refs.bib 
	@pdflatex $(SOURCE)
	@bibtex $(NAME)
	@pdflatex $(SOURCE)
	@pdflatex $(SOURCE)

distrib:
	tar -cvzf $(TARBALL)  $(SOURCE) refs.bib Makefile figs/

all: $(TARGET) $(TARGETa)

abs: $(TARGETa)

full: $(TARGET)

clean:
	@for ext in $(JUNK); do \
		rm -v $(NAME)$$ext; \
		rm -v $(NAMEa)$$ext; \
	done
