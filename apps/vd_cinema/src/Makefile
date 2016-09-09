################################################################################
# Get Linux Build Enviornment:
include ../build.env

################################################################################
# Build options
SUB_DIRS	:= gdc
JOBS		:= 8

################################################################################
# Build
all: compile_subdirs 

compile_subdirs:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir -j$(JOBS) || exit $?;	\
	done

install:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir install;		\
	done

clean:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir clean;		\
	done

distclean:
	@for dir in $(SUB_DIRS); do \
	make -C $$dir distclean;	\
	done
