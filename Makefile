all:
	@echo see README and documentation for instructions.

clean: clean-vision clean-cortex clean-isolbench

compile-vision:
	make -C ${CURDIR}/vision/ compile

compile-cortex:
	for dir in $(subdirs); do\
		$(MAKE) -C cortex/$$dir compile;\
		done
clean-cortex:
	for dir in $(subdirs); do\
		$(MAKE) -C cortex/$$dir clean;\
		done

clean-vision:
	make -C ${CURDIR}/vision/ clean

clean-isolbench:
	make -C ${CURDIR}/IsolBench/ clean
