TARGET			= fc8080_tpeg

KDIR			:= ../../../../kernel/kernel-3.4.39
#STRIP_CMD		= arm-cortex_a9-linux-gnueabi-strip

obj-m			:= $(TARGET).o

$(TARGET)-objs	+=

######################################################################
# Source Cods
$(TARGET)-objs	+=

# Driver Main
$(TARGET)-objs	+= src/bbm.o \
	src/fc8080.o \
	src/fc8080_bb.o \
	src/fc8080_i2c.o \
	src/fc8080_isr.o \
	src/fc8080_ppi.o \
	src/fc8080_spi.o \
	src/fc8080_tun.o \
	src/fci_hal.o \
	src/fci_hpi.o \
	src/fci_oal.o \
	src/fci_ringbuffer.o \
	src/fci_tun.o \
	src/fic.o \
	src/ficdecoder.o \
	src/platform.o

# OS APIS
#$(TARGET)-objs	+= osapi/drv_osapi_linux.o

# VPU Control Sources
#$(TARGET)-objs	+= src/vpu_hw_interface.o
#$(TARGET)-objs	+= src/nx_jpu_api.o
#$(TARGET)-objs	+= src/nx_vpu_api.o
#$(TARGET)-objs	+= src/nx_vpu_gdi.o


################################################################################
#	Build options for board type
################################################################################
PWD     := $(shell pwd)
#KBUILD_CFLAGS += -I./include
#KBUILD_CFLAGS += -I./firmware
#KBUILD_CFLAGS += -I../libnxvmalloc

EXTRA_CFLAGS += -I./include

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@find . -type f \( -name '*.ko' -o -name '*.mod.*'		\
					-o -name '*.o' -o -name '.*.cmd'		\
					-o -name '*.order' -o -name '*.symvers'	\
					-o -name '*.' \) -print | xargs rm -f
	rm -rf .tmp_versions

