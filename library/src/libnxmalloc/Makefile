########################################################################
# Get Linux Build Enviornment:
include ../../../build.env

LIB_INSTALL	:=	../../lib
INC_INSTALL	:= 	../../include

######################################################################
# Build options
INCLUDE += -I./ -I../libion -I../../include -I$(KERNDIR)/include

LIBRARY += -L../../lib/

######################################################################
# Target
COBJS  	:= 
COBJS  	+= nx_alloc_mem_ion.o
CPPOBJS	:=  

CFLAGS	+= -g

LIBNAME	:= libnxvmem

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(LIBNAME)

$(LIBNAME):	depend $(OBJS)
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJS)

install:
	install -m 755 -d $(LIB_INSTALL)
	install -m 644 $(LIBNAME).a $(LIB_INSTALL)

clean:
	rm -f *.o *.so *.a .depend

distclean: clean
	rm -f $(LIB_INSTALL)/lib/$(LIBNAME).a


#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
