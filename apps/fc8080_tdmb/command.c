#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "fci_types.h"
#include "monitor.h"
#include "fc8080_regs.h"
#include "fci_dal.h"
#include "fic.h"
#include "fic_api.h"
#include "ficdecoder.h"


#define FIC_SZ (32*12)
#define MSC_SZ (188)

#define CAP_CHANNEL_MAX 6
#define CHANNEL_MAX 21

static int dmb_control;
static int dmb_fic;
static int dmb_video;
static u8 dump_start;
static u8 fic_dump_start;
static u8 sig_start;
static int setfreq;

struct service_info_type freq_tbl[]=
{
	{.chnum = 71, .freq = 175280},
	{72, 177008},
	{73, 178736},
	{81, 181280},
	{82, 183008},
	{83, 184736},
	{91, 187280},
	{92, 189008},
	{93, 190736},
	{101 ,193280},
	{102 ,195008},
	{103 ,196736},
	{111, 199280},
	{112, 201008},
	{113, 202736},
	{121, 205280},
	{122, 207008},
	{123, 208736},
	{131, 211280},
	{132, 213008},
	{133, 214736}
};

struct service_info_type cap_freq_tbl[] =
{
	{81, 181280},
	{82, 183008},
	{83, 184736},
	{121 ,205280},
	{122 ,207008},
	{123 ,208736}
};

void Compose_service_list(u32 tbl_idx, u32 cap)
{
	struct service_info_type *composer;
	struct ch_info_type *chInfo;
	u32 i, j, service_cnt, composer_cnt = 0;
	u8 *label;

	if (cap)
		composer = (struct service_info_type *)&cap_freq_tbl[tbl_idx];
	else
		composer = (struct service_info_type *)&freq_tbl[tbl_idx];
	label = dmb_drv_get_ensemble_label();

	if (label != NULL)
		memcpy(composer->ensLabel, label, LABEL_MAX_LEN);

	label = NULL;
	composer->dmb_subch_cnt = dmb_drv_get_dmb_sub_ch_cnt();
	composer->dab_subch_cnt = dmb_drv_get_dab_sub_ch_cnt();
	composer->dat_subch_cnt = dmb_drv_get_dat_sub_ch_cnt();
	composer->tot_subch_cnt = composer->dmb_subch_cnt +
							composer->dab_subch_cnt +
							composer->dat_subch_cnt;

	for (i = 0; i < 3; i++) {
		if (i == 0)
			service_cnt = composer->dmb_subch_cnt;
		else if (i == 1)
			service_cnt = composer->dab_subch_cnt;
		else if (i == 2)
			service_cnt = composer->dat_subch_cnt;

		for (j = 0; j < service_cnt; j++) {
			if (i == 0)
				chInfo = (struct ch_info_type *)dmb_drv_get_fic_dmb(j);
			else if (i == 1)
				chInfo = (struct ch_info_type *)dmb_drv_get_fic_dab(j);
			else if (i == 2)
				chInfo = (struct ch_info_type *)dmb_drv_get_fic_dat(j);

			composer->chinfo[composer_cnt].uiEnsembleID = chInfo->uiEnsembleID;
			composer->chinfo[composer_cnt].ucSubchID = chInfo->ucSubchID;
			composer->chinfo[composer_cnt].uiStartAddress = chInfo->uiStartAddress;
			composer->chinfo[composer_cnt].ucTMId = chInfo->ucTMId;
			composer->chinfo[composer_cnt].ucServiceType = chInfo->ucServiceType;
			composer->chinfo[composer_cnt].ulServiceID = chInfo->ulServiceID;

			if (i == 0)
				label = dmb_drv_get_sub_ch_dmb_label(j);
			else if (i == 1)
				label = dmb_drv_get_sub_ch_dab_label(j);
			else if (i == 2)
				label = dmb_drv_get_sub_ch_dat_label(j);

			if (label != NULL)
				memcpy(composer->chinfo[composer_cnt].label, label, LABEL_MAX_LEN);

			composer_cnt++;
		}
	}
	return;
}

void display_service_list(u32 cap)
{
	struct service_info_type *composer;
	struct service_info_type *composer_child;
	struct ch_info_type *chInfo;
	struct subch_info_type *subchInfo;
	u32 i, j, k, service_cnt, composer_cnt = 0, subch_cnt = 0, subch_listcnt;
	u8 *label;

	if (cap) {
		composer = cap_freq_tbl;
		composer_cnt = CAP_CHANNEL_MAX;
	} else {
		composer = freq_tbl;
		composer_cnt = CHANNEL_MAX;
	}

	for (i = 0; i < composer_cnt; i++) {
		if (!composer[i].scan)
			continue;

		composer_child = (struct service_info_type *)&composer[i];

		print_log(0, "\n=====================================================\n");
		print_log(0, "Frequency : %10d, Ensemble_Name : %s\n",
			composer_child->freq, composer_child->ensLabel);
		subch_listcnt = 0;
		for (j = 0; j < 3; j++) {
			if (j == 0) {
				subch_cnt = composer_child->dmb_subch_cnt;
				print_log(0, "DMB  ");
			}
			else if (j == 1) {
				subch_cnt = composer_child->dab_subch_cnt;
				print_log(0, "DAB  ");
			}
			else if (j == 2) {
				subch_cnt = composer_child->dat_subch_cnt;
				print_log(0, "DAT  ");
			}
			print_log(0, "\n");

			for (k = 0; k < subch_cnt; k++) {
				subchInfo = (struct subch_info_type *)&composer_child->chinfo[subch_listcnt];

				print_log(0, "%16s, EId : 0x%x, SubChId : 0x%x, TMId : 0x%x, SVCType : 0x%x, SVCId : 0x%x",
						subchInfo->label, subchInfo->uiEnsembleID, subchInfo->ucSubchID,
						subchInfo->ucTMId, subchInfo->ucServiceType, subchInfo->ucSubchID);
				subch_listcnt++;
				print_log(0, "\n");
			}
		}
		print_log(0, "=====================================================\n");
	}
}

int GetEPG() {
	char* service_type;
	struct esbinfo_t *pEsbInfo;
	struct service_info_t *pSvcInfo;
	int	i, ret = BBM_OK;

	print_log(NULL, "\n======================================== FIC...Info\n");

	pEsbInfo = fic_decoder_get_ensemble_info(0);
	if(pEsbInfo == NULL) {
		print_log(NULL, "ESBINFO NULL ...\n");
		return ret;
	}

	if (strlen((char*) pEsbInfo->label) > 0) {
		print_log(NULL, "Ensemble: %s\n", pEsbInfo->label);
	} else {
		print_log(NULL, "Ensemble: NOT FOUND ENSEMBLE LABEL\n");
		ret = 1;
		//return ret;
	}

	for (i = 0; i < MAX_SVC_NUM; i++) {
		pSvcInfo = fic_decoder_get_service_info_list(i);

		if ((pSvcInfo->flag & 0x07) == 0x07) {
			switch(pSvcInfo->addrType) {
				case 0:
					pSvcInfo->label[16] = '\0';
					print_log(NULL, "%16s sid:     %#x ", pSvcInfo->label, pSvcInfo->sid);
					print_log(NULL, "subchid: 0x%x ", pSvcInfo->sub_channel_id);
					print_log(NULL, "tmid: 0x%x ", pSvcInfo->tmid);
					break;
				case 1:
					pSvcInfo->label[16] = '\0';
					print_log(NULL, "%16s sid: %#x ", pSvcInfo->label, pSvcInfo->sid);
					print_log(NULL, "subchid: 0x%x ", pSvcInfo->sub_channel_id);
					print_log(NULL, "tmid: 0x%x ", pSvcInfo->tmid);
					break;
				default:
					break;
			}

			print_log(NULL, " UAtype = 0x%X", pSvcInfo->user_appl_type[i]);
			print_log(NULL, "\n");
		}
	}

	print_log(NULL, "======================================== FIC...Info end\n");

	return ret;
}

void mmi_bbm_read1(u16 addr, u8 *data)
{
	BBM_READ(dmb_control, addr, data);
}

void mmi_bbm_write1(u16 addr, u8 data)
{
	BBM_WRITE(dmb_control, addr, data);
}

void mmi_bbm_wread1(u16 addr, u16 *data)
{
	BBM_WORD_READ(dmb_control, addr, data);
}

void mmi_bbm_wwrite1(u16 addr, u16 data)
{
	BBM_WORD_WRITE(dmb_control, addr, data);
}

void mmi_bbm_lread1(u16 addr, u32 *data)
{
	BBM_LONG_READ(dmb_control, addr, data);
}

void mmi_bbm_lwrite1(u16 addr, u32 data)
{
	BBM_LONG_WRITE(dmb_control, addr, data);
}

void mmi_bbm_wread_1(void)
{
	u16  length;
	u16 dest_addr, target_addr;
	u16 data;
	int i;

	if(1) {
		dest_addr = 0x18;
		BBM_WORD_READ(dmb_control, dest_addr, &data);
		print_log(NULL, "[0x%04X] : 0x%04X\n", dest_addr, data);
	}
}

void mmi_bbm_read(int argc, char *argv[])
{
	u16 addr;
	u16  length;
	u8  data;
	int i;

	if(argc == 2) {
		addr = htoi(argv[1]);
		BBM_READ(dmb_control, addr, &data);

		print_log(NULL, "[0x%04X] : 0x%02X\n", addr, data);
	} else if(argc == 3) {
		addr = htoi(argv[1]);
		length    = htoi(argv[2]);

		for(i=0; i<length; i++)
		{
			if((i % 8) == 0) print_log(NULL, "\n[0x%04X] : ", addr+i);
			BBM_READ(dmb_control, addr+i, &data);
			print_log(NULL, "%02X ", data & 0xFF);
		}
		print_log(NULL, "\n");
	} else {
		print_log(NULL, "usage : %s [start addr] [length] ; byte read command\n", (int)argv[0]);
	}
}

void mmi_bbm_write(int argc, char *argv[])
{
	u16  dest_addr;
	u8   dest_data;
	u8   length;
	int  i;


	if(argc == 3) {
		dest_addr = htoi(argv[1]);
		dest_data = htoi(argv[2]);

		BBM_WRITE(dmb_control, dest_addr, dest_data);
	} else if(argc == 4) {
		dest_addr = htoi(argv[1]);
		dest_data = htoi(argv[2]);
		length    = htoi(argv[3]);

		for(i=0; i<length; i++)
			BBM_WRITE(dmb_control, dest_addr+i, dest_data);
	} else {
		print_log(NULL, "usage : %s [start addr] [data] [length] ; byte write command\n", (int)argv[0]);
	}
}

void mmi_bbm_wread(int argc, char *argv[])
{
	u16  length;
	u16 dest_addr, target_addr;
	u16 data;
	int i;

	if(argc == 2) {
		dest_addr = htoi(argv[1]);
		BBM_WORD_READ(dmb_control, dest_addr, &data);
		print_log(NULL, "[0x%04X] : 0x%04X\n", dest_addr, data);
	} else if(argc == 3) {
		dest_addr = htoi(argv[1]);
		length    = htoi(argv[2]);

		for(i=0; i<length; i+=2)
		{
			target_addr = dest_addr + i;
			if((i % 4) == 0) print_log(NULL, "\n[0x%04X] : ", target_addr);
			BBM_WORD_READ(dmb_control, target_addr, &data);
			print_log(NULL, "%04X\n", data);
		}
		print_log(NULL, "\n");
	} else {
		print_log(NULL, "usage : %s [start addr] [length] ; word read command\n", argv[0]);
	}
}

void mmi_bbm_wwrite(int argc, char *argv[])
{
	u16  dest_addr;
	u16   dest_data;
	u16   length;
	int  i;

	if(argc == 3) {
		dest_addr = -htoi(argv[1])/100;
		dest_data = htoi(argv[2]);

		BBM_WORD_WRITE(dmb_control, dest_addr, dest_data);
	} else if(argc == 4) {
		dest_addr = htoi(argv[1]);
		dest_data = htoi(argv[2]);
		length    = htoi(argv[3]);

		for(i=0; i<length; i+=2) {
			BBM_WORD_WRITE(dmb_control, dest_addr+i, dest_data);
		}
	} else {
		print_log(NULL, "usage : %s [start addr] [data] [length] ; word write command\n", (int)argv[0]);
	}
}

void mmi_bbm_lread(int argc, char *argv[])
{
	u16  length;
	u16 dest_addr, target_addr;
	u32 data;
	int i;

	if(argc == 2) {
		dest_addr = htoi(argv[1]);
		BBM_LONG_READ(dmb_control, dest_addr, &data);
		print_log(NULL, "[0x%04X] : 0x%08X\n", dest_addr, data);
	} else if(argc == 3) {
		dest_addr = htoi(argv[1]);
		length    = htoi(argv[2]);

		for(i=0; i<length; i+=4) {
			target_addr = dest_addr + i;
			if((i % 2) == 0) print_log(NULL, "\n[0x%04X] : ", target_addr);
			BBM_LONG_READ(dmb_control, target_addr, &data);
			print_log(NULL, "%08X\n", data);
		}
		print_log(NULL, "\n");
	} else {
		print_log(NULL, "usage : %s [start addr] [length] ; long read command\n", argv[0]);
	}
}

void mmi_bbm_lwrite(int argc, char *argv[])
{
	u16  dest_addr;
	u32   dest_data;
	u16   length;
	int  i;

	if(argc == 3) {
		dest_addr = htoi(argv[1]);
		dest_data = htoi(argv[2]);

		BBM_LONG_WRITE(dmb_control, dest_addr, dest_data);
	} else if(argc == 4) {
		dest_addr = htoi(argv[1]);
		dest_data = htoi(argv[2]);
		length    = htoi(argv[3]);

		for(i=0; i<length; i+=4) {
			BBM_LONG_WRITE(dmb_control, dest_addr+i, dest_data);
		}
	} else {
		print_log(NULL, "usage : %s [start addr] [data] [length] ; long write command\n", (int)argv[0]);
	}
}

void mmi_bbm_verify(int argc, char *argv[])
{
	u8  dest_addr;
	u8  dest_data, temp_data;
	int  i;
	int  retry;

	switch(argc) {
		case 3:
			dest_data = 0x0;
			dest_addr = htoi(argv[1]);
			retry     = dtoi(argv[2]);

			print_log(NULL, "%s [0x%X] [%d]\n", argv[0], dest_addr, retry);

			for(i=0; i < retry; i++) {
				BBM_WRITE(dmb_control, dest_addr, dest_data);

				BBM_READ(dmb_control, dest_addr, &temp_data);
				if(dest_data != temp_data) {
					print_log(NULL, "\n 0x%xth Mismatch Data ;  addr[0x%X] write [0x%X] : read [0x%X]\n",
							i, dest_addr, dest_data, temp_data);
				}
				dest_data++;
				print_log(NULL, ".");
			}
			print_log(NULL, "\n");
			break;
		default:
			print_log(NULL, "Usage : %s [address] [retry]\n", argv[0]);
			break;
	}
}

void mmi_bbm_i2c_verify(int argc, char *argv[])
{
	u8  dest_addr;
	u8  dest_data, temp_data;
	int  retry, delay = 1;
	int  i;

	switch(argc) {
		case 3:
			dest_addr = htoi(argv[1]);
			retry     = dtoi(argv[2]);

			dest_data = 0x0;
			for(i=0; i < retry; i++) {
				BBM_TUNER_WRITE(dmb_control, dest_addr, 1, &dest_data, 1);

				BBM_TUNER_READ(dmb_control, dest_addr, 1, &temp_data, 1);

				if(dest_data != temp_data) {
					print_log(NULL, "\n0x%Xth Mismatch Data ;  addr[0x%X] write [0x%X] : read [0x%X]\n",
							i, dest_addr, dest_data, temp_data);
				}
				dest_data++;
				print_log(NULL, ".");
			}
			print_log(NULL, "\n");
			break;
		default:
			print_log(NULL, "Usage : %s [address] [retry]\n", argv[0]);
			break;
	}
}


/*================================================================================
 00 | 175280KHz |  f1e00000 |        1 |    00 | Pixtree 
 01 | 181280KHz |  f1e00453 |        3 |    03 | U1-BWS           
 02 | 181280KHz |  f1e00455 |        1 |    04 | Home&Shopping    
 03 | 181280KHz |  f1e00451 |        1 |    00 | U1(MBN)          
 04 | 181280KHz |  f1e00454 |        1 |    02 | WOW-TV           
 05 | 184736KHz |  f1e00412 |        1 |    02 |  
 06 | 184736KHz |  f1e00413 |        1 |    03 | GS SHOP          
 07 | 184736KHz |  f1e00414 |        1 |    05 | HD TEST          
 08 | 184736KHz |  f1e00415 |        3 |    04 | QBS Data         
 09 | 184736KHz |  f1e00411 |        1 |    01 |  
 10 | 205280KHz |  f1e00424 |        1 |    03 |  
 11 | 205280KHz |  f1e00421 |        3 |    05 | MBC TPEG         
 12 | 205280KHz |  f1e00420 |        1 |    01 | my MBC           
 13 | 205280KHz |  f1e00422 |        3 |    06 | MBC BWS          
 14 | 205280KHz |  f1e00423 |        1 |    02 | MBC RADIO        
 15 | 205280KHz |  f1e00425 |        2 |    58 |  
 16 | 207008KHz |  f1e00443 |        1 |    04 | KBS MUSIC        
 17 | 207008KHz |  f1e00441 |        1 |    00 | KBS STAR         
 18 | 207008KHz |  f1e00442 |        1 |    11 | KBS HEART        
 19 | 207008KHz |  f1e00448 |        3 |    08 | KBS TPEG         
 20 | 207008KHz |  f1e00445 |        3 |    06 | UKBS-TTI         
 21 | 207008KHz |  f1e77044 |        2 |    58 | KBS AEAS         
 22 | 208736KHz |  f1e00435 |        1 |    06 | Arirang Radio    
 23 | 208736KHz |  f1e77043 |        2 |    58 | SBS AEAS         
 24 | 208736KHz |  f1e00436 |        1 |    07 | HYUNDAI Home     
 25 | 208736KHz |  f1e00432 |        1 |    02 | SBS V-Radio      
 26 | 208736KHz |  f1e00434 |        3 |    08 | DGPS             
 27 | 208736KHz |  f1e00431 |        1 |    00 | SBS u TV         
 28 | 208736KHz |  f1e00433 |        3 |    03 | SBS ROADi 
*/

void mmi_tuner_set_1(void)
{
	u16  intMask;
	int i;
	u32 freq = 0;
	struct esbinfo_t *esb;

	freq = 184736;
	if(freq)
	{
		if(tdmb_set_channel(dmb_control, freq))
		{
			print_log(NULL, "Tuner is not setting %s!!!\n", freq);
			return;
		}

		fic_decoder_subchannel_info_clean();

		if(tdmb_scan_status(dmb_control)) {
			print_log(NULL, "LOCK NOK \n");
			return;
		}

		get_fic_data(dmb_fic);

		/* wait 1.2 sec for gathering fic information */
		ms_wait(1200);
		stop_get_fic();

		esb = fic_decoder_get_ensemble_info(0);
		if (esb->flag != 99) {
			print_log(NULL, "ESB ERROR \n");
			fic_decoder_subchannel_info_clean();
			return;
		}

		if (strnlen(esb->label, sizeof(esb->label)) <= 0) {
			fic_decoder_subchannel_info_clean();
			print_log(NULL, "label ERROR \n");
			return;
		}
		GetEPG();
		setfreq = freq;
	}
	else
	{
		print_log(NULL, "Invailed freq ...\n");
		setfreq = 0;
		return;
	}
}

void mmi_tuner_set(int argc, char *argv[])
{
	u16  intMask;
	int i;
	u32 freq = 0;
	struct esbinfo_t *esb;

	switch (argc)
	{
		case 2:
			freq = dtoi(argv[1]);
			if(freq)
			{
				if(tdmb_set_channel(dmb_control, freq))
				{
					print_log(NULL, "Tuner is not setting %s!!!\n", freq);
					return;
				}

				fic_decoder_subchannel_info_clean();

				if(tdmb_scan_status(dmb_control)) {
					print_log(NULL, "LOCK NOK \n");
					return;
				}

				get_fic_data(dmb_fic);

				/* wait 1.2 sec for gathering fic information */
				ms_wait(1200);
				stop_get_fic();

				esb = fic_decoder_get_ensemble_info(0);
				if (esb->flag != 99) {
					print_log(NULL, "ESB ERROR \n");
					fic_decoder_subchannel_info_clean();
					return;
				}

				if (strnlen(esb->label, sizeof(esb->label)) <= 0) {
					fic_decoder_subchannel_info_clean();
					print_log(NULL, "label ERROR \n");
					return;
				}
				GetEPG();
				setfreq = freq;
			}
			else
			{
				print_log(NULL, "Invailed freq ...\n");
				setfreq = 0;
				return;
			}
			break;

		default:
			print_log(NULL, "Usage : %s [frequency]\n", argv[0]);
			break;
	}
}

void mmi_bbm_reset_cmd(int argc, char *argv[])
{
	switch(argc) {
		case 1:
			BBM_RESET(dmb_control);
#if 0 //nexell 20160512 						
			msc_dump_stop();
			stop_get_fic();			
#endif			
			break;
		default:
			print_log(NULL, "Usage : %s\n", argv[0]);
			break;
	}
}

void mmi_bbm_init_cmd(int argc, char *argv[])
{
	int res = BBM_NOK;
	float test;

	if (!dmb_control)
		res = tdmb_drv_open(&dmb_control);
	if (!dmb_fic)
		res |= tdmb_drv_open(&dmb_fic);
	if (!dmb_video)
		res |= tdmb_drv_open(&dmb_video);


	//tdmb_power_on(dmb_control);
	res |= tdmb_init(dmb_control);

	if(res)
		print_log(NULL, "DMB init fail : %d\n", res);
}

void mmi_bbm_deinit_cmd(int argc, char *argv[])
{
	tdmb_video_deselect(dmb_video);
	tdmb_audio_deselect(dmb_video);
	tdmb_data_deselect(dmb_video);
	tdmb_power_off(dmb_control);
	tdmb_drv_close(dmb_fic);
	tdmb_drv_close(dmb_video);
	tdmb_drv_close(dmb_control);
	dmb_fic = dmb_video = dmb_control = 0;
}

void mmi_bbm_power_on(int argc, char *argv[])
{
	tdmb_power_on(dmb_control);
}

void mmi_bbm_power_off(int argc, char *argv[])
{
	tdmb_power_off(dmb_control);
}

void mmi_i2c_read_cmd(int argc, char *argv[])
{
	u8  addr;
	u8  data[256], length;
	u8  tmp;
	int i;

	if(argc == 2) {
		data[0] = 0;
		addr = htoi(argv[1]);

		BBM_TUNER_READ(dmb_control, addr, 1, &data[0], 1);

		print_log(NULL, "[0x%08X] : 0x%02X\n", addr, data[0]);
	} else if(argc == 3) {

		addr = htoi(argv[1]);
		length    = htoi(argv[2]);

		for(i=0; i<length; i++) {
			tmp = addr+i;
			BBM_TUNER_READ(dmb_control, tmp, 1, &data[i], 1);
		}

		for(i=0; i<length; i++) {
			tmp = addr+i;
			if((i % 8) == 0) print_log(NULL, "\n[0x%08X] : ", tmp);
			print_log(NULL, "%02X ", data[i] & 0xFF);
		}
		print_log(NULL, "\n");
	} else {
		print_log(NULL, "Usage : %s [start addr] [length]\n", argv[0]);
	}
}

void mmi_i2c_write_cmd(int argc, char *argv[])
{
	u8  dest_addr;
	u8   dest_data, length;
	int  i;

	if(argc == 3) {
		dest_addr = htoi(argv[1]);
		dest_data = htoi(argv[2]);

		BBM_TUNER_WRITE(dmb_control, dest_addr, 1, &dest_data, 1);
	} else if(argc == 4) {
		dest_addr = htoi(argv[1]);
		dest_data = htoi(argv[2]);
		length    = htoi(argv[3]);

		if(dest_data == 0x1234) {
			dest_data = 0;
			for(i=0; i<=length; i++)
				BBM_TUNER_WRITE(dmb_control, dest_addr+i, 1, &dest_data, 1);
		} else {
			for(i=0; i<length; i++)
				BBM_TUNER_WRITE(dmb_control, dest_addr+i, 1, &dest_data, 1);
		}
	} else {
		print_log(NULL, "Usage : %s [start addr] [data] [length]\n", argv[0]);
	}
}

void mmi_ficload_cmd(int argc, char *argv[])
{
	if(argc == 2)
	{
		FILE	*fp;

		fp = fopen(argv[1], "rb");
		if(fp)
		{
			char fic_dat[FIC_SZ];
			int i, n;

			fic_decoder_subchannel_info_clean();
			while(!feof(fp))
			{
				n = fread(fic_dat, 1, FIC_SZ, fp);
				if(n != FIC_SZ)
					break;
				fic_decoder_put((struct fic*) fic_dat, FIC_SZ);
			}

			fclose(fp);

			GetEPG();
		} else {
			print_log(NULL, "File not found: %s\n", argv[1]);
		}
	}
}

void mmi_mscload_cmd(int argc, char *argv[])
{
	int res;
	int	count = 0;

	if(argc == 2)
	{
		FILE	*fp;
		create_tspacket_anal();
		fp = fopen(argv[1], "rb");
		if(fp)
		{
			char msc[MSC_SZ];
			int i, n;

			while(!feof(fp))
			{
				n = fread(msc, 1, MSC_SZ, fp);
				if(n != MSC_SZ)
					break;
				res = put_ts_packet(0, msc, n);

				if (res) {
					print_log(NULL, "PKT NO : %d count : %d mod : %d \n", count, count/32, count%32);
				}
				count++;
			}

			fclose(fp);
		} else {
			print_log(NULL, "File not found: %s\n", argv[1]);
		}
		print_pkt_log();
	}
}

void mmi_fic_dump_cmd(int argc, char *argv[])
{
	if (fic_dump_start) {
		fic_dump_start = 0;
		stop_get_fic();
		print_log(NULL, "Stop Dump fic\n");
	} else {
		fic_dump_start = 1;
		get_fic_data(dmb_fic, setfreq);
		print_log(NULL, "Start Dump fic\n");
	}
}

void mmi_msc_dump_cmd_1(void)
{
	u8 subChId = 0;
	u8 svcId = 0;

	subChId = 5;
	svcId = 0x02;
	dump_start = 1;
	msc_dump_start(dmb_video, subChId, svcId);

	print_log(NULL, "msc_dump_start subch : 0x%x, svcId : 0x%x\n", subChId, svcId);

}

void mmi_msc_dump_cmd(int argc, char *argv[])
{
	u8 subChId = 0;
	u8 svcId = 0;

	if (dump_start) {
		dump_start = 0;
		msc_dump_stop();
	} else {
		if (argc == 1) {
			subChId = 0;
			svcId = SVC_VIDEO;
			dump_start = 1;
			msc_dump_start(dmb_video, subChId, svcId);
		} else if (argc == 3) {
			subChId = htoi(argv[1]);
			svcId = htoi(argv[2]);
			dump_start = 1;
			msc_dump_start(dmb_video, subChId, svcId);
		}
		else
			print_log(NULL, "usage : %s [subChId][svcId]\n", argv[0]);
		print_log(NULL, "msc_dump_start subch : 0x%x, svcId : 0x%x\n", subChId, svcId);
	}
}

void mmi_channelselect_cmd(int argc, char *argv[])
{
	u8 svcId = 0;
	u8 subChId = 0;

	tdmb_type_set(dmb_video, MSC_TYPE);

	if (argc == 1) {
		subChId = 0;
		tdmb_video_select(dmb_video, subChId);
	} else if (argc == 3) {
		subChId = htoi(argv[1]);
		svcId = htoi(argv[2]);
		if (svcId == 0x18)
			tdmb_video_select(dmb_video, subChId);
		else if (svcId == 0x00)
			tdmb_audio_select(dmb_video, subChId);
		else
			tdmb_data_select(dmb_video, subChId);

		print_log(0, "tdmb_%s_select subchID : 0x%x\n"
			, (svcId == 0x18)?"video":(svcId == 0x00)?"audio":"data", subChId);
	} else
		print_log(NULL, "usage : %s [svcId][subChId]\n", argv[0]);
}

void mmi_dechannelselect_cmd(int argc, char *argv[])
{
	u8 channel = 0;
	u8 svcId = 0;

	if (argc == 1) {
		tdmb_video_deselect(dmb_video);
		tdmb_audio_deselect(dmb_video);
		tdmb_data_deselect(dmb_video);
		print_log(NULL, "Channel Deselected All\n");
	} else if (argc == 2) {
		svcId = htoi(argv[1]);

		if (svcId == 0x18)
			tdmb_video_deselect(dmb_video);
		else if (svcId == 0x00)
			tdmb_audio_deselect(dmb_video);
		else
			tdmb_data_deselect(dmb_video);

		print_log(0, "tdmb_%s_deselect\n"
			, (svcId == 0x18)?"video":(svcId == 0x00)?"audio":"data");
	}
	else
		print_log(NULL, "usage : %s [svcId]\n", argv[0]);
}

void mmi_exit_cmd(int argc, char *argv[])
{

#if 0 //nexell 20160512
	msc_dump_stop();
	stop_get_fic();
	tdmb_power_off(dmb_control);

	tdmb_drv_close(dmb_control);
	tdmb_drv_close(dmb_fic);
	tdmb_drv_close(dmb_video);
#endif

	exit(0);
}

static void mmi_dm_cmd(int argc, char *argv[])
{
	u32 vBER, rBER;
	s32 rssi;
	u8 lock;

	tdmb_signal_quality_info(dmb_control, &lock, &vBER, &rBER, &rssi);
}

static void mmi_sig_cmd(int argc, char *argv[])
{
	if (sig_start) {
		sig_start = 0;
		sig_thread_stop();
	} else {
		sig_start = 1;
		sig_thread_start(dmb_control);
	}
}


static u32 mmi_scan_cmd(int argc, char *argv[])
{
	int res = BBM_OK;
	u32 i, cap;
	struct esbinfo_t *esb;

	if (argc < 2)
		cap = 1;
	else
		cap = dtoi(argv[1]);

	if (cap) {
		for (i = 0; i < CAP_CHANNEL_MAX; i++) {
			fic_decoder_subchannel_info_clean();
			cap_freq_tbl[i].scan = 0;
			tdmb_set_channel(dmb_control, cap_freq_tbl[i].freq);
			res = tdmb_scan_status(dmb_control);

			if (res)
				print_log(NULL, "FREQ %d SCAN FAIL\n", cap_freq_tbl[i].freq);
			else {
				get_fic_data(dmb_fic);

				/* wait 1.2 sec for gathering fic information */
				ms_wait(1200);
				stop_get_fic();
				esb = fic_decoder_get_ensemble_info(0);
				if (esb->flag != 99) {
					print_log(NULL, "ESB ERROR \n");
					fic_decoder_subchannel_info_clean();
					continue;
				}

				if (strnlen(esb->label, sizeof(esb->label)) <= 0) {
					fic_decoder_subchannel_info_clean();
					print_log(NULL, "label ERROR \n");
					continue;
				}
				cap_freq_tbl[i].scan = 1;
				Compose_service_list(i, cap);
				print_log(NULL, "FREQ %d SCAN Success\n", cap_freq_tbl[i].freq);
				//GetEPG();
			}
		}
	} else {
		for (i = 0; i < CHANNEL_MAX; i++) {
			fic_decoder_subchannel_info_clean();
			freq_tbl[i].scan = 0;
			tdmb_set_channel(dmb_control, freq_tbl[i].freq);
			res = tdmb_scan_status(dmb_control);

			if (res)
				print_log(NULL, "FREQ %d SCAN FAIL\n", freq_tbl[i].freq);
			else {
				print_log(NULL, "FREQ %d SCAN Success\n", freq_tbl[i].freq);
				tdmb_fic_select(dmb_control);
				get_fic_data(dmb_fic);
				/* wait 1.2 sec for gathering fic information */
				ms_wait(1200);
				stop_get_fic();

				esb = fic_decoder_get_ensemble_info(0);
				if (esb->flag != 99) {
					print_log(NULL, "ESB ERROR \n");
					fic_decoder_subchannel_info_clean();
					continue;
				}

				if (strnlen(esb->label, sizeof(esb->label)) <= 0) {
					fic_decoder_subchannel_info_clean();
					print_log(NULL, "label ERROR \n");
					continue;
				}
				freq_tbl[i].scan = 1;
				Compose_service_list(i, cap);
				//GetEPG();
			}
		}
	}
	display_service_list(cap);
}

static u32 mmi_rssi_cmd(int argc, char *argv[])
{
	s32 rssi;

	BBM_TUNER_GET_RSSI(dmb_control, &rssi);

	print_log(NULL, "RSSI : %d\n", rssi);
}

void anal_command(int argc, char *argv[])
{
	if(iscmd("?"))             Mon_help_command();

	else if(iscmd("brd"))		mmi_bbm_read(argc, argv);
	else if(iscmd("bwr"))		mmi_bbm_write(argc, argv);
	else if(iscmd("wrd"))		mmi_bbm_wread(argc, argv);
	else if(iscmd("wwr"))		mmi_bbm_wwrite(argc, argv);
	else if(iscmd("lrd"))		mmi_bbm_lread(argc, argv);
	else if(iscmd("lwr"))		mmi_bbm_lwrite(argc, argv);

	else if(iscmd("verify"))	mmi_bbm_verify(argc, argv);
	else if(iscmd("iverify"))	mmi_bbm_i2c_verify(argc, argv);

	else if(iscmd("tuner"))		mmi_tuner_set(argc, argv);

	else if(iscmd("reset"))		mmi_bbm_reset_cmd(argc, argv);
	else if(iscmd("init"))		mmi_bbm_init_cmd(argc, argv);
	else if(iscmd("deinit"))	mmi_bbm_deinit_cmd(argc, argv);

	else if(iscmd("ird"))		mmi_i2c_read_cmd(argc, argv);
	else if(iscmd("iwr"))		mmi_i2c_write_cmd(argc, argv);
	else if(iscmd("pwron"))		mmi_bbm_power_on(argc, argv);
	else if(iscmd("pwroff"))	mmi_bbm_power_off(argc, argv);

	else if(iscmd("ficload"))	mmi_ficload_cmd(argc, argv);
	else if(iscmd("mscload"))	mmi_mscload_cmd(argc, argv);
	else if(iscmd("fic_dump"))	mmi_fic_dump_cmd(argc, argv);
	else if(iscmd("msc_dump"))	mmi_msc_dump_cmd(argc, argv);

	else if(iscmd("channel"))	mmi_channelselect_cmd(argc, argv);
	else if(iscmd("dechannel"))	mmi_dechannelselect_cmd(argc, argv);
	else if(iscmd("dm"))		mmi_dm_cmd(argc, argv);
	else if(iscmd("sig"))		mmi_sig_cmd(argc, argv);
	else if(iscmd("rssi"))		mmi_rssi_cmd(argc, argv);
	else if(iscmd("scan"))		mmi_scan_cmd(argc, argv);
	else if(iscmd("exit"))		mmi_exit_cmd(argc, argv);

	else print_log(NULL, "\nNo such command - type help\n");
}
