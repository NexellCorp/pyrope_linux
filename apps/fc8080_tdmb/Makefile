########################################################################
# Get Linux Build Enviornment:
include ../../build.env

LFLAGS  = -static -pthread

OBJS = main.o monitor.o command.o clib.o fci_oal.o fc8080_api.o fci_dal.o fci_dataread.o ficdecoder.o fic.o fci_tspacket_test.o fc8080_demux.o fic_api.o
SRCS = $(OBJS:.o=.c)

.SUFFIXES : .c .o

fccon: $(OBJS)
	$(CC) $(LFLAGS) -o fccon $(OBJS)

clean:
	rm -rf $(OBJS) fccon


