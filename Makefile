#general targets
all:
	@echo see README and documentation for instructions.

docs: setup-docs
	make -C ${CURDIR}/docs

clean: clean-vision clean-isolbench clean-docs clean-tacle clean-image-filters

setup: setup-docs setup-tacle setup-image-filters

#setup targets
setup-docs:
	make -C ${CURDIR}/docs setup

setup-tacle:
	@echo 'Initialization and fetching of the pinned version of the submodule...'
	@git submodule update --init --recursive rt-tacle-bench
	@echo 'Convert the submodule README.md to a .dox file that will be included in the documentation...'
	@cd rt-tacle-bench && bash ../utils/md2dox.sh README

setup-image-filters:
	@echo 'Initialization and fetching of the pinned version of the submodule...'
	@git submodule update --init --recursive image-filters
ifndef DOCS_ONLY
	@echo 'Fetching and converting input images base...'
	@bash ${CURDIR}/image-filters/inputs/init.sh
endif
	@echo 'Convert the submodule README.md to a .dox file that will be included in the documentation...'
	@cd image-filters && bash ../utils/md2dox.sh README

#compilation targets
compile-isolbench:
	@echo 'Compiling IsolBench'
	make -C ${CURDIR}/IsolBench/

compile-tacle: setup-tacle
	@echo 'Compiling TACLeBench'
	make -C ${CURDIR}/rt-tacle-bench/

compile-vision:
	@echo 'Compiling SD-VBS'
	make -C ${CURDIR}/vision/ compile

compile-image-filters:
	@echo 'Compiling image-filters'
	make -C ${CURDIR}/image-filters/

#clean targets
clean-tacle:
	@echo 'Cleaning TACLeBench'
	make -C ${CURDIR}/rt-tacle-bench/ clean

clean-vision:
	@echo 'Cleaning SD-VBS'
	make -C ${CURDIR}/vision/ clean

clean-isolbench:
	@echo 'Cleaning IsolBench'
	make -C ${CURDIR}/IsolBench/ clean

clean-image-filters:
	@echo 'Cleaning image-filters'
	make -C ${CURDIR}/image-filters/ clean

clean-docs:
	@echo 'Cleaning docs'
	make -C ${CURDIR}/docs clean

# benchmark suite groups

# WCET group
setup-group-WCET: setup-tacle

clean-group-WCET: clean-tacle

compile-group-WCET: setup-bmarks-WCET compile-tacle

# vision group
setup-group-vision: setup-image-filters

clean-group-vision: clean-vision clean-image-filters

compile-group-vision: compile-vision compile-image-filters

# interference group
setup-group-interf:

clean-group-interf: clean-isolbench

compile-group-interf: compile-isolbench
