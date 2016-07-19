#ifndef __FIC_API_H__
#define __FIC_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "fic.h"

#define USER_APPL_NUM_MAX		12
#define USER_APPL_DATA_SIZE_MAX	24
#define LABEL_MAX_LEN				32

struct ch_info_type {
	unsigned short uiEnsembleID;
	unsigned char ucSubchID;
	unsigned short uiStartAddress;
	unsigned char ucTMId;
	unsigned char ucServiceType;
	unsigned long ulServiceID;
	unsigned char num_of_user_appl;
	unsigned short user_appl_type[USER_APPL_NUM_MAX];
	unsigned char user_appl_length[USER_APPL_NUM_MAX];
	unsigned char user_appl_data
		[USER_APPL_NUM_MAX][USER_APPL_DATA_SIZE_MAX];
	unsigned char scids;
	unsigned char ecc;
};

struct subch_info_type {
	unsigned short uiEnsembleID;
	unsigned char ucSubchID;
	unsigned short uiStartAddress;
	unsigned char ucTMId;
	unsigned char ucServiceType;
	unsigned long ulServiceID;
	unsigned char label[LABEL_MAX_LEN];
};

struct service_info_type {
	unsigned int chnum;
	unsigned int freq;
	unsigned char ensLabel[LABEL_MAX_LEN];
	unsigned int tot_subch_cnt;
	unsigned int dmb_subch_cnt;
	unsigned int dab_subch_cnt;
	unsigned int dat_subch_cnt;
	struct subch_info_type chinfo[32];
	unsigned int scan;
};


extern int dmb_drv_get_dmb_sub_ch_cnt();
extern int dmb_drv_get_dab_sub_ch_cnt();
extern int dmb_drv_get_dat_sub_ch_cnt(void);
extern char *dmb_drv_get_ensemble_label();
extern char *dmb_drv_get_sub_ch_dmb_label(int subchannel_count);
extern char *dmb_drv_get_sub_ch_dab_label(int subchannel_count);
extern char *dmb_drv_get_sub_ch_dat_label(int subchannel_count);
extern struct ch_info_type *dmb_drv_get_fic_dmb(int subchannel_count);
extern struct ch_info_type *dmb_drv_get_fic_dab(int subchannel_count);
extern struct ch_info_type *dmb_drv_get_fic_dat(int subchannel_count);

#ifdef __cplusplus
}
#endif

#endif /* __FIC_API_H__ */

