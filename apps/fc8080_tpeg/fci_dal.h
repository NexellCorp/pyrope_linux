#ifndef __FCI_DAL_H__
#define __FCI_DAL_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fci_types.h"

#define BBM_HPI		0		// EBI2
#define BBM_SPI		1		// SPI

typedef enum {
	BAND3_TYPE = 0,
	LBAND_TYPE
} band_type;

typedef enum {
	FC8080_TUNER = 0,
} product_type;

#define CTL_TYPE 0
#define FIC_TYPE 1
#define MSC_TYPE 2

#define IOCTL_MAGIC     't'

typedef struct {
    u32 buff[4];
} ioctl_info;

#define IOCTL_MAXNR			45

#define IOCTL_DMB_RESET			_IO( IOCTL_MAGIC, 0 )
#define IOCTL_DMB_PROBE			_IO( IOCTL_MAGIC, 1 )
#define IOCTL_DMB_INIT		 	_IO( IOCTL_MAGIC, 2 )
#define IOCTL_DMB_DEINIT	 	_IO( IOCTL_MAGIC, 3 )

#define IOCTL_DMB_BYTE_READ 		_IOWR( IOCTL_MAGIC, 4, ioctl_info )
#define IOCTL_DMB_WORD_READ 		_IOWR( IOCTL_MAGIC, 5, ioctl_info )
#define IOCTL_DMB_LONG_READ 		_IOWR( IOCTL_MAGIC, 6, ioctl_info )
#define IOCTL_DMB_BULK_READ 		_IOWR( IOCTL_MAGIC, 7, ioctl_info )

#define IOCTL_DMB_BYTE_WRITE 		_IOW( IOCTL_MAGIC, 8, ioctl_info )
#define IOCTL_DMB_WORD_WRITE 		_IOW( IOCTL_MAGIC, 9, ioctl_info )
#define IOCTL_DMB_LONG_WRITE 		_IOW( IOCTL_MAGIC, 10, ioctl_info )
#define IOCTL_DMB_BULK_WRITE 		_IOW( IOCTL_MAGIC, 11, ioctl_info )

#define IOCTL_DMB_TUNER_SELECT	 	_IOW( IOCTL_MAGIC, 12, ioctl_info )

#define IOCTL_DMB_TUNER_READ	 	_IOWR( IOCTL_MAGIC, 13, ioctl_info )
#define IOCTL_DMB_TUNER_WRITE	 	_IOW( IOCTL_MAGIC, 14, ioctl_info )

#define IOCTL_DMB_TUNER_SET_FREQ 	_IOW( IOCTL_MAGIC, 15, ioctl_info )
#define IOCTL_DMB_SCAN_STATUS 		_IO( IOCTL_MAGIC, 16 )

#define IOCTL_DMB_TYPE_SET   _IOW( IOCTL_MAGIC, 17, ioctl_info )

#define IOCTL_DMB_CHANNEL_SELECT	_IOW( IOCTL_MAGIC, 18, ioctl_info )
#define IOCTL_DMB_FIC_SELECT 		_IO( IOCTL_MAGIC, 19 )
#define IOCTL_DMB_VIDEO_SELECT		_IOW( IOCTL_MAGIC, 20, ioctl_info )
#define IOCTL_DMB_AUDIO_SELECT		_IOW( IOCTL_MAGIC, 21, ioctl_info )
#define IOCTL_DMB_DATA_SELECT		_IOW( IOCTL_MAGIC, 22, ioctl_info )

#define IOCTL_DMB_CHANNEL_DESELECT	_IOW( IOCTL_MAGIC, 23, ioctl_info )
#define IOCTL_DMB_FIC_DESELECT 		_IO( IOCTL_MAGIC, 24 )
#define IOCTL_DMB_VIDEO_DESELECT	_IO( IOCTL_MAGIC, 25 )
#define IOCTL_DMB_AUDIO_DESELECT	_IO( IOCTL_MAGIC, 26 )
#define IOCTL_DMB_DATA_DESELECT		_IO( IOCTL_MAGIC, 27 )

#define IOCTL_DMB_TUNER_GET_RSSI 	_IOR( IOCTL_MAGIC, 28, ioctl_info )

#define IOCTL_DMB_POWER_ON		_IO( IOCTL_MAGIC, 29 )
#define IOCTL_DMB_POWER_OFF		_IO( IOCTL_MAGIC, 30 )

#define IOCTL_DMB_GET_BER		_IOR(IOCTL_MAGIC, 31, struct ioctl_info)

extern int BBM_RESET(HANDLE hDevice);
extern int BBM_PROBE(HANDLE hDevice);
extern int BBM_INIT(HANDLE hDevice);
extern int BBM_DEINIT(HANDLE hDevice);

extern int BBM_READ(HANDLE hDevice, u16 addr, u8 *data);
extern int BBM_BYTE_READ(HANDLE hDevice, u16 addr, u8 *data);
extern int BBM_WORD_READ(HANDLE hDevice, u16 addr, u16 *data);
extern int BBM_LONG_READ(HANDLE hDevice, u16 addr, u32 *data);
extern int BBM_BULK_READ(HANDLE hDevice, u16 addr, u8 *data, u16 size);

extern int BBM_DATA(HANDLE hDevice, u16 addr,u8 *data, u16 size);

extern int BBM_WRITE(HANDLE hDevice, u16 addr, u8 data);
extern int BBM_BYTE_WRITE(HANDLE hDevice, u16 addr, u8 data);
extern int BBM_WORD_WRITE(HANDLE hDevice, u16 addr, u16 data);
extern int BBM_LONG_WRITE(HANDLE hDevice, u16 addr, u32 data);
extern int BBM_BULK_WRITE(HANDLE hDevice, u16 addr, u8 *data, u16 size);

extern int BBM_TUNER_READ(HANDLE hDevice, u8 addr, u8 alen, u8 *buffer, u8 len);
extern int BBM_TUNER_WRITE(HANDLE hDevice, u8 addr, u8 alen, u8 *buffer, u8 len);

extern int BBM_TUNER_SET_FREQ(HANDLE hDevice, u32 freq);
extern int BBM_TUNER_SELECT(HANDLE hDevice, u32 product, u32 band);
extern int BBM_TUNER_GET_RSSI(HANDLE hDevice, s32 *rssi);
extern int BBM_SCAN_STATUS(HANDLE hDevice);

extern int BBM_TYPE_SET(HANDLE hDevice, u8 type);

extern int BBM_CHANNEL_SELECT(HANDLE hDevice, u8 subChId, u8 svcChId);
extern int BBM_VIDEO_SELECT(HANDLE hDevice, u8 subChId);
extern int BBM_AUDIO_SELECT(HANDLE hDevice, u8 subChId);
extern int BBM_DATA_SELECT(HANDLE hDevice, u8 subChId);
extern int BBM_CHANNEL_DESELECT(HANDLE hDevice, u8 subChId);
extern int BBM_VIDEO_DESELECT(HANDLE hDevice);
extern int BBM_AUDIO_DESELECT(HANDLE hDevice);
extern int BBM_DATA_DESELECT(HANDLE hDevice);

extern int BBM_HOSTIF_SELECT(HANDLE hDevice, u8 hostif);
extern int BBM_HOSTIF_DESELECT(HANDLE hDevice);

#ifdef __cplusplus
}
#endif

#endif /* __FCI_DAL_H__ */

