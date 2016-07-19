#include <linux/ioctl.h>
#include <string.h>
#include "fci_dal.h"

#define DEVICE_FILENAME	"/dev/dmb"

int BBM_RESET(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_RESET);

	return BBM_OK;
}

int BBM_PROBE(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_PROBE);

	return res;
}

int BBM_INIT(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_INIT);

	return res;
}

int BBM_DEINIT(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_DEINIT);

	return res;
}

int BBM_READ(HANDLE hDevice, u16 addr, u8 *data)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;

	res = ioctl(hDevice, IOCTL_DMB_BYTE_READ, &info);

	*data = info.buff[1];

	return res;
}

int BBM_BYTE_READ(HANDLE hDevice, u16 addr, u8 *data)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;

	res = ioctl(hDevice, IOCTL_DMB_BYTE_READ, &info);

	*data = info.buff[1];

	return res;
}

int BBM_WORD_READ(HANDLE hDevice, u16 addr, u16 *data)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;

	res = ioctl(hDevice, IOCTL_DMB_WORD_READ, &info);

	*data = (u16)info.buff[1];

	return res;
}

int BBM_LONG_READ(HANDLE hDevice, u16 addr, u32 *data)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;

	res = ioctl(hDevice, IOCTL_DMB_LONG_READ, &info);

	*data = info.buff[1];

	return res;
}

int BBM_BULK_READ(HANDLE hDevice, u16 addr, u8 *data, u16 size)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;
	info.buff[1] = size;

	res = ioctl(hDevice, IOCTL_DMB_BULK_READ, &info);

	memcpy(data, (void*)&info.buff[2], size);

	return res;
}

int BBM_WRITE(HANDLE hDevice, u16 addr, u8 data)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;
	info.buff[1] = data;

	res = ioctl(hDevice, IOCTL_DMB_BYTE_WRITE, &info);

	return res;
}

int BBM_BYTE_WRITE(HANDLE hDevice, u16 addr, u8 data)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;
	info.buff[1] = data;

	res = ioctl(hDevice, IOCTL_DMB_BYTE_WRITE, &info);

	return res;
}

int BBM_WORD_WRITE(HANDLE hDevice, u16 addr, u16 data)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;
	info.buff[1] = data;

	res = ioctl(hDevice, IOCTL_DMB_WORD_WRITE, &info);

	return res;
}

int BBM_LONG_WRITE(HANDLE hDevice, u16 addr, u32 data)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;
	info.buff[1] = data;

	res = ioctl(hDevice, IOCTL_DMB_LONG_WRITE, &info);

	return res;
}

int BBM_BULK_WRITE(HANDLE hDevice, u16 addr, u8 *data, u16 size)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;
	info.buff[1] = size;
	memcpy(&info.buff[2], data, size);

	res = ioctl(hDevice, IOCTL_DMB_BULK_WRITE, &info);

	return res;
}

int BBM_TUNER_READ(HANDLE hDevice, u8 addr, u8 alen, u8 *buffer, u8 len)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;
	info.buff[1] = alen;
	info.buff[2] = len;

	res = ioctl(hDevice, IOCTL_DMB_TUNER_READ, &info);

	memcpy(buffer, (void*)&info.buff[3], len);

	return res;
}

int BBM_TUNER_WRITE(HANDLE hDevice, u8 addr, u8 alen, u8 *buffer, u8 len)
{
	int res;
	ioctl_info info;

	info.buff[0] = addr;
	info.buff[1] = alen;
	info.buff[2] = len;
	memcpy((void*)&info.buff[3], buffer, len);

	res = ioctl(hDevice, IOCTL_DMB_TUNER_WRITE, &info);

	return res;
}

int BBM_TUNER_SET_FREQ(HANDLE hDevice, u32 freq)
{
	int res;
	ioctl_info info;

	info.buff[0] = freq;

	res = ioctl(hDevice, IOCTL_DMB_TUNER_SET_FREQ, &info);

	return res;
}

int BBM_SCAN_STATUS(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_SCAN_STATUS);

	return res;
}

int BBM_TUNER_SELECT(HANDLE hDevice, u32 product, u32 band)
{
	int res;
	ioctl_info info;

	info.buff[0] = product;
	info.buff[1] = band;

	res = ioctl(hDevice, IOCTL_DMB_TUNER_SELECT, &info);

	return res;
}

int BBM_TYPE_SET(HANDLE hDevice, u8 type)
{
	int res;
	ioctl_info info;
	info.buff[0] = type;
	res = ioctl(hDevice, IOCTL_DMB_TYPE_SET, &info);

	return res;
}

int BBM_FIC_SELECT(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_FIC_SELECT);

	return res;
}

int BBM_VIDEO_SELECT(HANDLE hDevice, u8 subChId)
{
	int res;
	ioctl_info info;

	info.buff[0] = subChId;

	res = ioctl(hDevice, IOCTL_DMB_VIDEO_SELECT, &info);

	return res;
}

int BBM_AUDIO_SELECT(HANDLE hDevice, u8 subChId)
{
	int res;
	ioctl_info info;

	info.buff[0] = subChId;

	res = ioctl(hDevice, IOCTL_DMB_AUDIO_SELECT, &info);

	return res;
}

int BBM_DATA_SELECT(HANDLE hDevice, u8 subChId)
{
	int res;
	ioctl_info info;

	info.buff[0] = subChId;

	res = ioctl(hDevice, IOCTL_DMB_DATA_SELECT, &info);

	return res;
}

int BBM_FIC_DESELECT(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_FIC_DESELECT);

	return res;
}

int BBM_VIDEO_DESELECT(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_VIDEO_DESELECT);

	return res;
}

int BBM_AUDIO_DESELECT(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_AUDIO_DESELECT);

	return res;
}

int BBM_DATA_DESELECT(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_DATA_DESELECT);

	return res;
}


int BBM_TUNER_GET_RSSI(HANDLE hDevice, s32* rssi)
{
	int res;
	ioctl_info info;

	res = ioctl(hDevice, IOCTL_DMB_TUNER_GET_RSSI, &info);

	*rssi = info.buff[0];

	return res;
}


int BBM_POWER_ON(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_POWER_ON);

	return res;
}

int BBM_POWER_OFF(HANDLE hDevice)
{
	int res;

	res = ioctl(hDevice, IOCTL_DMB_POWER_OFF);

	return res;
}

