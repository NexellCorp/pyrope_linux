ifndef	JOBS
JOBJS	:= 8
endif

DIR		:= 
######################################################################
# Build options

# Multi Media Libraries
DIR += sap_marriage
DIR += sap_slink_client

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

distclean:
	@for dir in $(DIR); do			\
	make -C $$dir distclean || exit $?;	\
	done

install:
	@for dir in $(DIR); do			\
	make -C $$dir install || exit $?;	\
	done
