.PHONY: setup clean all

all: html

pdf: clean-pdf
	@mkdir -p pdf/attachments
	@cp -r source/attachments pdf/
	@cd conf && sed 's/GENERATE_LATEX\s*=\s*NO/GENERATE_LATEX=YES/' Doxyfile | doxygen -
	@make -C latex
	@mv latex/refman.pdf pdf/Documentation.pdf
	@rm -r latex


print: clean-pdf
	@sed -i 's/\\documentclass\[oneside\]/\\documentclass\[twoside\]/' conf/latex-header.tex
	@cd conf && sed 's/GENERATE_LATEX\s*=\s*NO/GENERATE_LATEX=YES/' Doxyfile | doxygen -
	@sed -i 's/\\documentclass\[twoside\]/\\documentclass\[oneside\]/' conf/latex-header.tex
	@mkdir -p pdf/attachments
	@cp -r source/attachments pdf/
	@make -C latex
	@mv latex/refman.pdf pdf/Documentation.pdf
	@rm -r latex

html: setup
	@# copy the pdf folder into the html documentation, so that i will be included in the commit & push
	@mkdir -p html/attachments
	@cp -r source/attachments html/
	@cd conf && sed 's/GENERATE_HTML\s*=\s*NO/GENERATE_HTML=YES/' Doxyfile | doxygen -
	@cd html && touch .nojekyll
ifeq ("$(wildcard Documentation.html)","")
	@ln -s html/index.html Documentation.html
endif

setup:
ifeq ("$(wildcard conf/doxygen-awesome/doxygen-awesome.css)", "")
	@git submodule update --init --recursive conf/doxygen-awesome
endif

clean: clean-pdf clean-html

clean-html:
	@echo removing generated documentation pages...
	@rm -rf html Documentation.html

clean-pdf:
	@rm -rf latex pdf

