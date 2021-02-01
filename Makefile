# Top level makefile for CortexSuite (BETA)
# UCSD
#
#

subdirs = lda liblinear motion-estimation pca rbm sphinx srr svd3 clustering


#
# Note the SD-VBS subcomponent of CortexSuite is located inside
# the vision directory and has a separate makefile infrastructure.
#
# this makefile only drives the other components of CortexSuite, in the cortex directory.
#

all:
	@echo see README for instructions. Or read the Makefile. "=)"

run-cortex: run-cortex-small run-cortex-medium run-cortex-large


run-vision:
	make -C ${CURDIR}/vision/ c-run


compile-cortex:
	for dir in $(subdirs); do\
		$(MAKE) -C cortex/$$dir compile;\
		done

run-cortex-small:
	for dir in $(subdirs); do\
		$(MAKE) -C cortex/$$dir run-small;\
		done

run-cortex-medium:
	for dir in $(subdirs); do\
		$(MAKE) -C cortex/$$dir run-medium;\
		done

run-cortex-large:
	for dir in $(subdirs); do\
		$(MAKE) -C cortex/$$dir run-large;\
		done

cycles-cortex:
	@echo ---------------------
	@echo Small Datasets Cycle
	@echo ---------------------
	@echo
	-grep Cycles cortex/results/*.small
	@echo
	@echo
	@echo ---------------------
	@echo Medium  Datasets Cycle
	@echo ---------------------
	@echo
	-grep Cycles cortex/results/*.medium
	@echo
	@echo
	@echo ---------------------
	@echo Large Datasets Cycle
	@echo ---------------------
	@echo
	-grep Cycles cortex/results/*.large
	@echo
	@echo

clean-cortex:
	for dir in $(subdirs); do\
		$(MAKE) -C cortex/$$dir clean;\
		done
