ifndef	JOBS
JOBJS	:= 8
endif

DIR		:= 
######################################################################
# Build options

DIR += reboot_update
DIR += adc_test
DIR += audio_test
DIR += cec_test
DIR += fb_test
DIR += gpio_test
DIR += jpeg_test
DIR += nmea_test
DIR += spi_test
DIR += transcoding_example
DIR += v4l2_test
DIR += vip_test
DIR += vpu_test2


######################################################################
# Build
all:
	@for dir in $(DIR); do			\
	make -C $$dir || exit $?;		\
	make -C $$dir install; 			\
	done

clean:
	@for dir in $(DIR); do			\
	make -C $$dir clean || exit $?;	\
	done

