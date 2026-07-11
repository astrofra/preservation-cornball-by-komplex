DESTDIR = ../../../doc/html/$(DOCNAME)
LATEX2HTMLDIR = /usr/lib/latex2html
LATEX2HTML = latex2html
LATEX2HTMLOPTS = -init_file ../common/latex2html-init -dir $(DESTDIR)
LATEX = latex
M4 = m4
M4OPTS = -I../common
DVIPS = dvips
DVIPSOPTS = -t a4
RM = rm
RMDIR = rmdir
CP = cp

export TEXINPUTS := ../common:$(LATEX2HTMLDIR)/texinputs:$(TEXINPUTS)

.PRECIOUS:	%.aux %.toc %.tex

# Cancel implicit rules that might bother us:
%.dvi : %.tex

%.txt :		%.m4
		python ../../tools/docconv.py t $* $< 

%.pdf :		%.ps
		echo | gs -dNOPAUSE -sDEVICE=pdfwrite -sOutputFile=$@ -q $^

%.ps :		%.dvi
		$(DVIPS) $(DVIPSOPTS) -o $@ $*

%-html :	%.tex %.toc
		$(LATEX2HTML) $(LATEX2HTMLOPTS) $*
		echo done > $@

%.dvi :		%.aux %.toc
		$(LATEX) $*

%.aux %.toc :	%.tex
		$(LATEX) $*
		$(RM) $*.dvi

%.tex :		%.m4 $(DOCINCLUDES) ../common/midasdoc.sty
		$(M4) $(M4OPTS) $*.m4 > $@

clean distclean cvsclean :
		-$(RM) *.log *.toc *.aux *.dvi *~ *.bak *-html *.tex *.ps *.pdf *.txt core

install :	$(DOCNAME).dvi $(DOCNAME).tex $(DOCNAME).ps $(DOCNAME).pdf $(DOCNAME).txt
		$(CP) $(DOCNAME).dvi ../../../doc/dvi
		$(CP) $(DOCNAME).tex ../../../doc/latex
		$(CP) $(DOCNAME).ps ../../../doc/ps
		$(CP) $(DOCNAME).pdf ../../../doc/pdf
		$(CP) $(DOCNAME).txt ../../../doc/txt