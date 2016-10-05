########################################################################
# Get Linux Build Enviornment:
include ../../build.env

######################################################################
# Build options
#LIBRARY += -lstdc++

######################################################################
# Target
COBJS   := reboot_update.o
CFLAGS  += -g
TARGET := reboot_update

######################################################################
# Build
OBJS    := $(COBJS)

all: $(TARGET)

$(TARGET):  depend $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

clean:
	rm -f $(TARGET) *.o .depend

#########################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c)
depend dep:
	$(CC) -M $(CFLAGS) $(SRCS) > .depend
