/*****************************************************************************
	Copyright(c) 2013 FCI Inc. All Rights Reserved

	File name : bbm.c

	Description : API source file of dmb baseband module

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the Free Software
	Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

	History :
	----------------------------------------------------------------------
*******************************************************************************/

#include "../include/dmbdrv_wrap_fc8080.h"
#include "../include/fci_types.h"
#include "../include/bbm.h"
#include "../include/fci_oal.h"
#include "../include/fc8080_regs.h"
#include "../include/fic.h"
#include "../include/fci_tun.h"
/*#include "tdmb.h"*/
#define DPRINTK(x, args...) print_log(NULL, x, args)
struct sub_channel_info_type dmb_subchannel_info;
struct sub_channel_info_type dab_subchannel_info;


static u32 saved_ber = 3000;
static u32 dmb_initialize;

unsigned char current_service_type = 0x18;
unsigned char current_subchannel_id;

int tdmb_interrupt_fic_callback(u32 userdata, u8 *data, int length)
{
	fic_decoder_put((struct fic *)data, length);

	return 0;
}

#ifdef FEATURE_FC8080_DEBUG
#define FC8080_DMB 0x01
#define FC8080_DATA 0x08
#define FC8080_DAB 0x04

u16 dmb_mode = FC8080_DMB;
#endif

int tdmb_interrupt_msc_callback(
u32 userdata, u8 subchannel_id, u8 *data, int length)
{
	/*tdmb_store_data(&data[0], length);*/

	return 0;
}

static int viterbi_rt_ber_read(unsigned int *ber)
{
	u32 dmp_ber_rxd_bits;
	u32 dmp_ber_err_bits;

	bbm_com_write(NULL, 0xe01, 0x0f);

	bbm_com_long_read(NULL, 0xe40, &dmp_ber_rxd_bits);
	bbm_com_long_read(NULL, 0xe44, &dmp_ber_err_bits);

	if (dmp_ber_rxd_bits)
		*ber = (dmp_ber_err_bits * 10000) /	 dmp_ber_rxd_bits;
	else
		*ber = 10000;

	print_log(NULL, "BER : %d \n", *ber);
	return BBM_OK;
}


static int get_signal_level(u32 ber, u8 *level)
{
	if (ber >= 900)
		*level = 0;
	else if ((ber >= 800) && (ber < 900))
		*level = 1;
	else if ((ber >= 700) && (ber < 800))
		*level = 2;
	else if ((ber >= 600) && (ber < 700))
		*level = 3;
	else if ((ber >= 500) && (ber < 600))
		*level = 4;
	else if ((ber >= 400) && (ber < 500))
		*level = 5;
	else if (ber < 400)
		*level = 6;

	return BBM_OK;
}

void dmb_drv_isr()
{
	bbm_com_isr(NULL);
}

#define FEATURE_INTERFACE_TEST_MODE 1
#define CONFIG_TDMB_SPI             1

unsigned char dmb_drv_init(void)
{
#ifdef FEATURE_INTERFACE_TEST_MODE
	int i;
	u8 data;
	u16 wdata;
	u32 ldata;
	u8 temp = 0x1e;
#endif

	printk(KERN_ERR "## \e[31m PJSMSG \e[0m [%s():%s:%d\t] tpeg \n", __FUNCTION__, strrchr(__FILE__, '/')+1, __LINE__);

#ifdef CONFIG_TDMB_SPI
	if (bbm_com_hostif_select(NULL, BBM_SPI))
		return TDMB_FAIL;
#elif defined(CONFIG_TDMB_EBI)
	if (bbm_com_hostif_select(NULL, BBM_PPI))
		return TDMB_FAIL;
#endif

	/* check for factory  chip interface test */
	if (bbm_com_probe(NULL) != BBM_OK) {
		DPRINTK("%s : BBM_PROBE fail\n", __func__);
		return TDMB_FAIL;
	}

	bbm_com_fic_callback_register(0, tdmb_interrupt_fic_callback);
	bbm_com_msc_callback_register(0, tdmb_interrupt_msc_callback);

	bbm_com_init(NULL);
	bbm_com_tuner_select(NULL, FC8080_TUNER, BAND3_TYPE);

#ifdef FEATURE_INTERFACE_TEST_MODE
	for (i = 0; i < 1000; i++) {
		bbm_com_write(NULL, 0xa4, i & 0xff);
		bbm_com_read(NULL, 0xa4, &data);
		if ((i & 0xff) != data)
			DPRINTK("FC8080 byte test (0x%x,0x%x)\r\n"
			, i & 0xff, data);
	}
	for (i = 0; i < 1000; i++) {
		bbm_com_word_write(NULL, 0xa4, i & 0xffff);
		bbm_com_word_read(NULL, 0xa4, &wdata);
		if ((i & 0xffff) != wdata)
			DPRINTK("FC8080 word test (0x%x,0x%x)\r\n"
			, i & 0xffff, wdata);
	}
	for (i = 0; i < 1000; i++) {
		bbm_com_long_write(NULL, 0xa4, i & 0xffffffff);
		bbm_com_long_read(NULL, 0xa4, &ldata);
		if ((i & 0xffffffff) != ldata)
			DPRINTK("FC8080 long test (0x%x,0x%x)\r\n"
			, i & 0xffffffff, ldata);
	}
	for (i = 0; i < 1000; i++) {
		temp = i&0xff;
		bbm_com_tuner_write(NULL, 0x13, 0x01, &temp, 0x01);
		bbm_com_tuner_read(NULL, 0x13, 0x01, &data, 0x01);
		if ((i & 0xff) != data)
			DPRINTK("FC8080 tuner test (0x%x,0x%x)\r\n"
			, i & 0xff, data);
	}
#endif

	saved_ber = 3000;
	dmb_initialize = 1;

	return TDMB_SUCCESS;
}

unsigned char dmb_drv_deinit(void)
{
	dmb_initialize = 0;

	bbm_com_video_deselect(NULL, 0, 0, 0);
	bbm_com_audio_deselect(NULL, 0, 2);
	bbm_com_data_deselect(NULL, 0, 3);

	ms_wait(100);

	bbm_com_deinit(NULL);

	bbm_com_fic_callback_deregister(NULL);
	bbm_com_msc_callback_deregister(NULL);

	bbm_com_hostif_deselect(NULL);

	return TDMB_SUCCESS;
}

unsigned char dmb_drv_scan_ch(unsigned long frequency)
{
	struct esbinfo_t *esb;

	if (!dmb_initialize)
		return TDMB_FAIL;

	if (bbm_com_tuner_set_freq(NULL, frequency))
		return TDMB_FAIL;

	fic_decoder_subchannel_info_clean();

	if (bbm_com_scan_status(NULL)) {
		bbm_com_word_write(NULL, BBM_BUF_ENABLE, 0x0000);
		return TDMB_FAIL;
	}

	bbm_com_word_write(NULL, BBM_BUF_ENABLE, 0x0100);

	/* wait 1.2 sec for gathering fic information */
	ms_wait(1200);

	bbm_com_word_write(NULL, BBM_BUF_ENABLE, 0x0000);

	esb = fic_decoder_get_ensemble_info(0);
	if (esb->flag != 99) {
		fic_decoder_subchannel_info_clean();
		return TDMB_FAIL;
	}

	if (strnlen(esb->label, sizeof(esb->label)) <= 0) {
		fic_decoder_subchannel_info_clean();
		return TDMB_FAIL;
	}

	return TDMB_SUCCESS;
}

int dmb_drv_get_dmb_sub_ch_cnt()
{
	struct service_info_t *svc_info;
	int i, n;

	if (!dmb_initialize)
		return 0;

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x01)
				&& (svc_info->dscty == 0x18))
				n++;
		}
	}

	return n;
}

int dmb_drv_get_dab_sub_ch_cnt()
{
	struct service_info_t *svc_info;
	int i, n;

	if (!dmb_initialize)
		return 0;

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x00)
				&& (svc_info->ascty == 0x00))
				n++;
		}
	}

	return n;
}

char *dmb_drv_get_ensemble_label()
{
	struct esbinfo_t *esb;

	if (!dmb_initialize)
		return NULL;

	esb = fic_decoder_get_ensemble_info(0);

	if (esb->flag == 99)
		return (char *)esb->label;

	return NULL;
}

char *dmb_drv_get_sub_ch_dmb_label(int subchannel_count)
{
	int i, n;
	struct service_info_t *svc_info;
	char *label = NULL;

	if (!dmb_initialize)
		return NULL;

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x01)
				&& (svc_info->dscty == 0x18)) {
				if (n == subchannel_count) {
					label = (char *) svc_info->label;
					break;
				}
				n++;
			}
		}
	}

	return label;
}

char *dmb_drv_get_sub_ch_dab_label(int subchannel_count)
{
	int i, n;
	struct service_info_t *svc_info;
	char *label = NULL;

	if (!dmb_initialize)
		return NULL;

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x00)
				&& (svc_info->ascty == 0x00)) {
				if (n == subchannel_count) {
					label = (char *) svc_info->label;
					break;
				}
				n++;
			}
		}
	}

	return label;
}

struct sub_channel_info_type *dmb_drv_get_fic_dmb(int subchannel_count)
{
	int i, n, j;
	struct esbinfo_t *esb;
	struct service_info_t *svc_info;
	u8 num_of_user_appl;

	if (!dmb_initialize)
		return NULL;

	memset((void *)&dmb_subchannel_info, 0, sizeof(dmb_subchannel_info));

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x01)
				&& (svc_info->dscty == 0x18)) {
				if (n == subchannel_count) {
					dmb_subchannel_info.ucSubchID
						= svc_info->sub_channel_id;
					dmb_subchannel_info.uiStartAddress
						= 0;
					dmb_subchannel_info.ucTMId
						= svc_info->tmid;
					dmb_subchannel_info.ucServiceType
						= svc_info->dscty;
					dmb_subchannel_info.ulServiceID
						= svc_info->sid;
					dmb_subchannel_info.scids
						= svc_info->scids;

					num_of_user_appl =
						svc_info->num_of_user_appl;
					dmb_subchannel_info.num_of_user_appl
						= num_of_user_appl;
					for (j = 0; j < num_of_user_appl; j++) {
						dmb_subchannel_info.
							user_appl_type[j]
						= svc_info->user_appl_type[j];
						dmb_subchannel_info.
							user_appl_length[j]
						= svc_info->user_appl_length[j];
						memcpy(
						&dmb_subchannel_info.
						user_appl_data[j][0]
						, &svc_info->
							user_appl_data[j][0]
						, dmb_subchannel_info.
						user_appl_length[j]);
					}

					esb = fic_decoder_get_ensemble_info(0);
					if (esb->flag == 99)
						dmb_subchannel_info.uiEnsembleID
						= esb->eid;
					else
						dmb_subchannel_info.uiEnsembleID
						= 0;
					dmb_subchannel_info.ecc	= esb->ecc;

					break;
				}
				n++;
			}
		}
	}

	return &dmb_subchannel_info;
}

struct sub_channel_info_type *dmb_drv_get_fic_dab(int subchannel_count)
{
	int i, n;
	struct esbinfo_t *esb;
	struct service_info_t *svc_info;

	if (!dmb_initialize)
		return NULL;

	memset((void *)&dab_subchannel_info, 0, sizeof(dab_subchannel_info));

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x00)
				&& (svc_info->ascty == 0x00)) {
				if (n == subchannel_count) {
					dab_subchannel_info.ucSubchID =
						svc_info->sub_channel_id;
					dab_subchannel_info.uiStartAddress = 0;
					dab_subchannel_info.ucTMId
						= svc_info->tmid;
					dab_subchannel_info.ucServiceType =
						svc_info->ascty;
					dab_subchannel_info.ulServiceID =
						svc_info->sid;
					dab_subchannel_info.scids =
						svc_info->scids;

					esb = fic_decoder_get_ensemble_info(0);
					if (esb->flag == 99)
						dmb_subchannel_info.uiEnsembleID
						= esb->eid;
					else
						dmb_subchannel_info.uiEnsembleID
						= 0;
					dab_subchannel_info.ecc	= esb->ecc;

					break;
				}
				n++;
			}
		}
	}

	return &dab_subchannel_info;
}

#ifdef FEATURE_FC8080_DEBUG
void dmb_drv_check_overrun(u8 reset)
{
	u16 overrun;
	u16 temp = 0;

	bbm_com_word_read(NULL, BBM_BUF_OVERRUN, &overrun);

	if (overrun & dmb_mode) {
		/* overrun clear */
		bbm_com_word_write(NULL, BBM_BUF_OVERRUN, overrun);

		if (reset) {
			/* buffer restore */
			bbm_com_word_read(NULL, BBM_BUF_ENABLE, &temp);
			temp &= ~dmb_mode;
			bbm_com_word_write(NULL, BBM_BUF_ENABLE, temp);
			temp |= dmb_mode;
			bbm_com_word_write(NULL, BBM_BUF_ENABLE, temp);
		}

		DPRINTK("fc8080 Overrun occured\n");
	}

}
#endif

u8 dmb_drv_set_ch(u32 frequency, 
                  u8 subchannel, 
                  u8 sevice_type)
{
	if (!dmb_initialize)
		return TDMB_FAIL;

	current_service_type = sevice_type;
	current_subchannel_id = subchannel;

	bbm_com_video_deselect(NULL, 0, 0, 0);
	bbm_com_audio_deselect(NULL, 0, 2);
	bbm_com_data_deselect(NULL, 0, 3);

	if (bbm_com_tuner_set_freq(NULL, frequency) != BBM_OK)
		return TDMB_FAIL;

	if (sevice_type == 0x18)
		bbm_com_video_select(NULL, subchannel, 0, 0);
	else if (sevice_type == 0x00)
		bbm_com_audio_select(NULL, subchannel, 2);
	else
		bbm_com_data_select(NULL, subchannel, 3);

#ifdef FEATURE_FC8080_DEBUG
	if (sevice_type == 0x18)
		dmb_mode = FC8080_DMB;
	else if (sevice_type == 0x00)
		dmb_mode = FC8080_DAB;
	else
		dmb_mode = FC8080_DATA;
#endif

	return TDMB_SUCCESS;
}

unsigned short dmb_drv_get_ber()
{
	return saved_ber;
}

unsigned char dmb_drv_get_ant(void)
{
	u8 level = 0;
	unsigned int ber;

	if (!dmb_initialize) {
		saved_ber = 3000;
		return 0;
	}

	if (viterbi_rt_ber_read(&ber)) {
		saved_ber = 3000;
		return 0;
	}

	if (ber <= 20)
		ber = 0;

	saved_ber = ber;
	if (get_signal_level(ber, &level))
		return 0;

#ifdef FEATURE_FC8080_DEBUG
	dmb_drv_check_overrun(1);
#endif

	return level;
}

signed short dmb_drv_get_rssi()
{
	s32 rssi;

	if (!dmb_initialize) {
		rssi = -110;
		return rssi;
	}

	bbm_com_tuner_get_rssi(NULL, &rssi);

	return (signed short)rssi;
}
