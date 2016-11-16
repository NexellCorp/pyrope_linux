ifndef	JOBS
JOBJS	:= 8
endif

DIR		:= 
######################################################################
# Build options

# Multi Media Libraries
DIR += src
DIR += apps

######################################################################
# Build
all:
	@for dir in $(DIR); do			\
	make -C $$dir || exit $?;		\
	make -C $$dir install; 			\
	done

clean:
	@for dir in $(DIR); do			\
	make -C $$dir clean || exit $?;	\
	done

install:
	@for dir in $(DIR); do			\
	make -C $$dir install || exit $?;	\
	done

distclean:
	@for dir in $(DIR); do			\
	make -C $$dir distclean || exit $?;	\
	done
