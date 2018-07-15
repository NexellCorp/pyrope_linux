################################################################################
# Get Linux Build Enviornment:
include ../../build.env

################################################################################
# Build options

# Prevent: "warning: creating a DT_TEXTREL in object"
CFLAGS	+= -fpic -DARM_11
CXXFLAGS+= -fpic -DARM_11

INCLUDE +=		\
	-I../include\
	-I../utils	\
	-I./

LIBRARY	+= -L../../lib
LIBRARY += -lssl -lcrypto
LIBRARY	+= -lstdc++ -lpthread

################################################################################
# Target
COBJS	:=
CXXOBJS	:=

CXXOBJS +=	\
	gdc_protocol.o		\
	CNX_OpenSSL.o		\
	NX_GDCServer.o		\
	NX_GDCClient.o

CXXOBJS +=	\
	../utils/NX_DbgMsg.o\
	../utils/SockUtils.o

LIBNAME	:= libnxmarriage
TARGET	:= $(LIBNAME).so

################################################################################
# Build
OBJS	:= $(COBJS) $(CXXOBJS)

all: $(TARGET)

$(TARGET): depend $(OBJS)
	$(CC) $(LDFLAGS) -shared -Wl,-soname,$(SO_NAME) -o $@ $(OBJS) $(LIBRARY)

clean:
	rm -f $(TARGET) $(OBJS) .depend

install:
	cp $(TARGET) ../../lib

distclean: clean
	rm ../../lib/$(TARGET)

################################################################################
# Dependency
ifeq (.depend,$(wildcard .depend))
include .depend
endif

SRCS := $(COBJS:.o=.c) $(CXXOBJS:.o=.cpp)
INCS := $(INCLUDE)
depend dep:
	$(CC) -M $(CFLAGS) $(CXXFLAGS) $(AFLGAS) $(INCS) $(SRCS) > .depend
