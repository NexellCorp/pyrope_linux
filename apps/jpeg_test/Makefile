########################################################################
# Get Linux Build Enviornment:
include ../../build.env

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I../../library/include 
LIBRARY	+= -L../../library/lib -L../../library/lib/ratecontrol
LIBRARY += -lnxjpeg -lturbojpeg -lnxvip -lnxdsp -lnxv4l2 -lnxvmem -lnxscaler -lnxvpu -lnxvidrc
LIBRARY	+= -lstdc++ -lpthread

######################################################################
# Target
DEC_OBJS	:= jpeg_dec.o
ENC_OBJS	:= jpeg_enc.o

COBJS		:= $(DEC_OBJS) $(ENC_OBJS)
CPPOBJS		:=  

CFLAGS		+= -g

DEC_APP		:= jpeg_dec
ENC_APP		:= jpeg_enc

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all:			\
	$(DEC_APP)	\
	$(ENC_APP)	\

$(DEC_APP):	depend $(OBJS)
	$(CC) $(CFLAGS) $(DEC_OBJS) -o $@ $(LIBRARY)

$(ENC_APP):	depend $(OBJS)
	$(CC) $(CFLAGS) $(ENC_OBJS) -o $@ $(LIBRARY)

clean:
	rm -f $(DEC_APP) $(ENC_APP) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
