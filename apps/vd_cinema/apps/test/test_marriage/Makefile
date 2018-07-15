################################################################################
# Get Linux Build Enviornment:
include ../../../build.env

################################################################################
# Build options
INCLUDE += -I./ -I../../../src/include

LIBRARY	+= -L../../../lib
LIBRARY += -lnxcinema_linux
LIBRARY	+= -lstdc++ -lpthread

################################################################################
# Target
COBJS	:=
CXXOBJS	:= main.o

TARGET	:= test_marriage

################################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(CFLAGS) $(CXXFLAGS) $(AFLGAS) $(OBJS) -o $(TARGET) $(LIBRARY) -g

clean:
	rm -f $(TARGET) *.o .depend

install:

################################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CXXOBJS:.o=.cpp) $(AOBJS:.o=.s)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(CXXFLAGS) $(AFLGAS) $(INCS) $(SRCS) > .depend
