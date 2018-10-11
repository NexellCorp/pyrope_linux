###############################################################################
# Build options
NX_MAKE_FILE :=
NX_MAKE_FILE += Makefile.server
NX_MAKE_FILE += Makefile.client

################################################################################
# Build
all:
	@for file in $(NX_MAKE_FILE); do	\
	make -f $$file || exit $?;			\
	done

clean:
	@for file in $(NX_MAKE_FILE); do	\
	make -f $$file clean || exit $?;	\
	done

install:
	@for file in $(NX_MAKE_FILE); do	\
	make -f $$file install || exit $?;	\
	done

distclean:
	@for file in $(NX_MAKE_FILE); do	\
	make -f $$file distclean || exit $?;\
	done
