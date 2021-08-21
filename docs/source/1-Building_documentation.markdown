Building the documentation locally
==================================
[TOC]

The documentation is available in pdf and html, and it is generated in the `docs` folder.

## Dependencies

- [Doxygen](doxygen.nl) 1.9.1+
- Make
- Dot tool, part of the graphviz package
- git
- sed
- pdflatex (only to generate documentation in pdf format.)
- A working internet connection
- [Doxygen Awesome](https://github.com/jothepro/doxygen-awesome-css) (will be set up automatically)

## Build targets

There are several make target that correspond to different documentation formats:
- `html`: Will generate a static website under the ``docs/html`` folder. For ease of use, the site home page will be linked in ``docs/Documentation.html``.
- `pdf`: Will generate a pdf file optimized for being read from a screen.
- ``print``: Will generate a pdf file optimized for being printed in A4 format.
- `clean`: Will remove the generated documentation files.

To use other targets, `make` must be invoked from the `docs` directory, the default target is `html`.

## Updating documentation on github pages
To update documentation on github pages the following git hooks are used:

- pre-push:
~~~{.sh}
#update local documentation
cd docs
#if we have the gh-pages branch as subdirectory
if [ -d "html" ] && [ $(git branch --show-current) == "main" ]; then
	cd html
	if [ $(git branch --show-current) == "gh-pages" ]; then
		#we revert the gh-pages branch to the initial commit
		git reset --hard `git rev-list --max-parents=0 --abbrev-commit HEAD`
		cd ..
		#rebuild the documentation
		make html
		# we make changes on the gh-pages branch only if the documentation was built successfully
		if [ $? == 0 ] && [ -f "html/index.html" ]; then
			#commit changes on the gh-pages branch
			cd html
			git add .
			git commit -m "Update documentation on github pages"
			git push --force origin gh-pages
		else
			echo "generation of the html documentation failed, aborting remote documentation update"
		fi
	else
		echo "documentation for github pages misconfigured, aborting remote documentation update."
	fi
	cd ..
fi
cd ..
~~~


The `setup` target of the makefile will clone the `gh-pages` branch of the repository (via SSH) into `docs/html`. Doxygen is configured to use that folder when generating html documentation.

This ensures that when a commit is pushed on the `main` branch, the documentation on github pages is updated accordingly when there are no errors.
