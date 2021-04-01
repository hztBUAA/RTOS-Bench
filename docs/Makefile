.PHONY: setup clean all

all: html

pdf:
	@cd conf && sed 's/GENERATE_LATEX\s*=\s*NO/GENERATE_LATEX=YES/' Doxyfile | doxygen -
	@make -C pdf
	@mv pdf/refman.pdf Documentation.pdf
	@rm -r pdf

print: setup
	@sed -i 's/\\documentclass\[oneside\]/\\documentclass\[twoside\]/' conf/latex-header.tex
	@cd conf && sed 's/GENERATE_LATEX\s*=\s*NO/GENERATE_LATEX=YES/' Doxyfile | doxygen -
	@sed -i 's/\\documentclass\[twoside\]/\\documentclass\[oneside\]/' conf/latex-header.tex
	@make -C pdf
	@mv pdf/refman.pdf Documentation-printable.pdf
	@rm -r pdf

html: setup
	@cd conf && sed 's/GENERATE_HTML\s*=\s*NO/GENERATE_HTML=YES/' Doxyfile | doxygen -
ifeq ("$(wildcard Documentation.html)","")
	@ln -s html/index.html Documentation.html
endif

setup:
ifeq ("$(wildcard conf/doxygen-awesome)", "")
	@echo installing dependencies
	@git clone https://github.com/jothepro/doxygen-awesome-css.git conf/doxygen-awesome
endif

clean:
	@echo removing generated documentation pages...
	@rm -rf html pdf Documentation.html Documentation.pdf Documentation-printable.pdf
	@rm -rf conf/doxygen-awesome

force:
