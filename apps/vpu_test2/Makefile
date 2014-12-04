########################################################################
# Get Linux Build Enviornment:
include ../../build.env

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I../../library/include
LIBRARY	+= -L../../library/lib -L../../library/lib/ratecontrol 
LIBRARY += -lnxvpu -lnxdsp -lnxvip -lnxv4l2 -lnxvmem -lnxvidrc
LIBRARY += -L./theoraparser -ltheoraparser
LIBRARY	+= -lavcodec -lavformat -lstdc++ -lm

######################################################################
# Target
CPPOBJS	:= \
	MediaExtractor.o	\
	CodecInfo.o			\
	NX_Queue.o			\
	NX_Semaphore.o		\
	Util.o				\
	VpuDecTest.o		\
	VpuEncTest.o		\
	VpuJpgTest.o		\
	main.o

CFLAGS	+= -g

THEORAPARSER := libtheoraparser

TARGET:= codec_tests

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all:	\
	$(THEORAPARSER) \
	$(TARGET)

$(THEORAPARSER):
	make -C theoraparser

$(TARGET):	depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $@ $(LIBRARY)

clean:
	make -C theoraparser clean
	rm -f $(OBJS) $(TARGET) .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
