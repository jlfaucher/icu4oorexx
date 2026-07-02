#######################################
# GNUmakefile for make (Macos, Linux) #
#######################################

# Keep this name!
# Under Macos and Linux, make will use in priority GNUmakefile.
# Under Windows, nmake will ignore GNUmakefile and use Makefile.

# ooRexx scripts in the current directory
scriptsFiles = charnames

help:
	@echo "make all"
	@echo "---------- scriptsFiles"
	@for f in $(scriptsFiles); do echo "make" $$f; done
	@echo "----------"
	@echo "make clean"

all:
	@for f in $(scriptsFiles); do make $$f; done
	@echo

$(scriptsFiles):
	bash -i -c 'rexx $@.rex  --testRegression > $@-out.txt 2>&1'
	diff -q $@-out-ref.txt $@-out.txt

clean:
	@for f in $(scriptsFiles); do rm -f $$f-out.txt; done
