#include <stdio.h>
#include <fcntl.h>

#include "fc8080_regs.h"
#include "fci_types.h"
#include "fci_oal.h"
#include "fci_dal.h"

int tdmb_drv_open(HANDLE *hDevice)
{
	int handle;

	handle = open("/dev/tpeg", O_RDWR | O_NDELAY);

	if(handle < 0)
	{
		print_log(0, "Cannot open handle : %d\n", handle);
		return BBM_NOK;
	}

	*hDevice = handle;

	return BBM_OK;
}

int tdmb_drv_close(HANDLE hDevice)
{
	int res = BBM_NOK;

	res = close(hDevice);

	return BBM_OK;
}

void tdmb_power_on(HANDLE hDevice)
{
	BBM_POWER_ON(hDevice);
}

void tdmb_power_off(HANDLE hDevice)
{
	BBM_POWER_OFF(hDevice);
}

int tdmb_init(HANDLE hDevice)
{
	int res = BBM_NOK;

	res = BBM_INIT(hDevice);
	res = BBM_TUNER_SELECT(hDevice, FC8080_TUNER, BAND3_TYPE);

	return res;
}

int tdmb_set_channel(HANDLE hDevice, u32 freq)
{
	int res = BBM_NOK;

	res = BBM_TUNER_SET_FREQ(hDevice,  freq);

	return  res;
}
int tdmb_scan_status(HANDLE hDevice)
{
	int res = BBM_NOK;

	res = BBM_SCAN_STATUS(hDevice);

	return res;
}

int tdmb_data_read(HANDLE hDevice, u8 *buf, u32 bufSize)
{
	s32 outSize;

	outSize = read(hDevice, buf, bufSize);

	return outSize;
}

int tdmb_type_set(HANDLE hDevice, u8 type)
{
	int res = BBM_NOK;

	res = BBM_TYPE_SET(hDevice, type);

	return res;
}

int tdmb_fic_select(HANDLE hDevice)
{
	int res = BBM_NOK;

	res = BBM_FIC_SELECT(hDevice);

	return  res;
}

int tdmb_video_select(HANDLE hDevice, u8 subchId)
{
	int res = BBM_NOK;

	res = BBM_VIDEO_SELECT(hDevice, subchId);

	return  res;
}

int tdmb_audio_select(HANDLE hDevice, u8 subchId)
{
	int res = BBM_NOK;

	res = BBM_AUDIO_SELECT(hDevice, subchId);

	return  res;
}

int tdmb_data_select(HANDLE hDevice, u8 subchId)
{
	int res = BBM_NOK;

	res = BBM_DATA_SELECT(hDevice, subchId);

	return  res;
}

int tdmb_fic_deselect(HANDLE hDevice)
{
	int res = BBM_NOK;

	res = BBM_FIC_DESELECT(hDevice);

	return  res;
}

int tdmb_video_deselect(HANDLE hDevice)
{
	int res = BBM_NOK;

	res = BBM_VIDEO_DESELECT(hDevice);

	return  res;
}

int tdmb_audio_deselect(HANDLE hDevice)
{
	int res = BBM_NOK;

	res = BBM_AUDIO_DESELECT(hDevice);

	return res;
}


int tdmb_data_deselect(HANDLE hDevice)
{
	int res = BBM_NOK;

	res = BBM_DATA_DESELECT(hDevice);

	return  res;
}

int tdmb_signal_quality_info(HANDLE hDevice,
	u8 *Lock, u32 *ui32VBER, u32 *ui32RBER, s32 *i32RSSI)
{
	u8 sync;
	struct dm_st {
		u32 start;

		u16 vit_ber_rxd_rsps;
		u16 vit_ber_err_rsps;
		u32 vit_ber_err_bits;

		u32 dmp_ber_rxd_bits;
		u32 dmp_ber_err_bits;
	};

	struct dm_st dm;

	BBM_READ(hDevice, BBM_SYNC_STAT, &sync);
	*Lock = (sync & 0x20)?1:0;
	BBM_BULK_READ(hDevice, 0xf000, (u8*) &dm, sizeof(dm));

	dm.vit_ber_err_bits &= 0x00ffffff;
	dm.dmp_ber_err_bits &= 0x00ffffff;

	BBM_TUNER_GET_RSSI(hDevice, i32RSSI);

	if (dm.vit_ber_rxd_rsps)
		*ui32VBER = (dm.vit_ber_err_bits * 10000)
				/ (204 * 8*  dm.vit_ber_rxd_rsps);
	else
		*ui32VBER = 10000;

	if (dm.dmp_ber_rxd_bits)
		*ui32RBER = (dm.dmp_ber_err_bits	* 10000) / dm.dmp_ber_rxd_bits;
	else
		*ui32RBER = 10000;
	//print_log(hDevice, "Lock  0x%x, BER : %d, RSBER : %d RSSI : %d\n", *Lock, *ui32RBER, *ui32VBER, *i32RSSI);
	print_log(hDevice, "TPEG Lock  RSSI : %d\n", *Lock, *i32RSSI);
}


