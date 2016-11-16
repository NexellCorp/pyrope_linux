ifndef	JOBS
JOBJS	:= 8
endif

DIR		:= 
######################################################################
# Build options

DIR += vd_cinema

######################################################################
# Build
all:
	@for dir in $(DIR); do			\
	make -C $$dir || exit $?;		\
	done

clean:
	@for dir in $(DIR); do			\
	make -C $$dir clean || exit $?;	\
	done

