########################################################################
# Get Linux Build Enviornment:
include ../../build.env

######################################################################
# Build options
INCLUDE += -I./
INCLUDE += -I../../library/include
LIBRARY	+= -L../../library/lib
LIBRARY += -lnxnmeaparser
LIBRARY	+= -lstdc++

######################################################################
# Target
COBJS	:= nmea_test.o
COBJS_6818 := nmea_test_6818.o
CPPOBJS	:=  

CFLAGS	+= -g

TARGET := nmea_test
TARGET_6818 := nmea_test_6818

######################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)
OBJS_6818	:= $(COBJS_6818)

all: $(TARGET) $(TARGET_6818)

$(TARGET):	depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LIBRARY)

$(TARGET_6818):	depend $(OBJS_6818)
	$(CC) $(CFLAGS) $(OBJS_6818) -o $@ $(LIBRARY)

clean:
	rm -f $(TARGET) $(TARGET_6818) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp) $(APPOBJS:.o=.c) $(COBJS_6818:.o=.c)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
