#general targets
all:
	@echo see README and documentation for instructions.

docs: setup-docs
	make -C ${CURDIR}/docs

clean: clean-vision clean-cortex clean-isolbench clean-tacle clean-docs

setup: setup-docs setup-tacle

#setup targets
setup-docs:
	make -C ${CURDIR}/docs setup

setup-tacle:
	git submodule init rt-tacle-bench
	git submodule update rt-tacle-bench
	cd rt-tacle-bench && bash ../utils/md2dox.sh README

#compilation targets
compile-isolbench:
	make -C ${CURDIR}/IsolBench/

compile-tacle: setup-tacle
	make -C ${CURDIR}/rt-tacle-bench/

compile-vision:
	make -C ${CURDIR}/vision/ compile

#clean targets
clean-tacle:
	make -C ${CURDIR}/rt-tacle-bench/ clean

clean-vision:
	make -C ${CURDIR}/vision/ clean

clean-isolbench:
	make -C ${CURDIR}/IsolBench/ clean

clean-docs:
	make -C ${CURDIR}/docs clean

# benchmark suite groups

# WCET group
setup-group-WCET: setup-tacle

clean-group-WCET: clean-tacle

compile-group-WCET: setup-bmarks-WCET compile-tacle

# vision group
setup-group-vision:

clean-group-vision: clean-vision

compile-group-vision: compile-vision

# vision group
setup-group-interf:

clean-group-interf: clean-isolbench

compile-group-interf: compile-isolbench
