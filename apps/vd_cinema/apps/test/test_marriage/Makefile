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
CPPOBJS	:= main.o

TARGET	:= test_marriage

################################################################################
# Build
OBJS	:= $(COBJS) $(CPPOBJS)

all: $(TARGET) $(TARGET2)

$(TARGET): depend $(OBJS)
	$(CC) $(CPPFLAGS) $(OBJS) -o $(TARGET) $(LIBRARY) -g

clean:
	rm -f $(TARGET) $(TARGET2) *.o .depend

install:

################################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CPPOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(INCS) $(SRCS) > .depend
