#include <linux/string.h>

#include "fic.h"
#include "fic_api.h"


struct ch_info_type dmb_info;
struct ch_info_type dab_info;
struct ch_info_type dat_info;

int dmb_drv_get_dmb_sub_ch_cnt()
{
	struct service_info_t *svc_info;
	int i, n;

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x01) && (svc_info->dscty == 0x18))
				n++;
		}
	}

	return n;
}

int dmb_drv_get_dab_sub_ch_cnt()
{
	struct service_info_t *svc_info;
	int i, n;

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x00) && (svc_info->ascty == 0x00))
				n++;
		}
	}

	return n;
}

int dmb_drv_get_dat_sub_ch_cnt(void)
{
	struct service_info_t *svc_info;
	int i, n;

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if (svc_info->tmid == 0x03)
				n++;
		}
	}

	return n;
}


char *dmb_drv_get_ensemble_label()
{
	struct esbinfo_t *esb;

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

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x01) && (svc_info->dscty == 0x18)) {
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

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x00) && (svc_info->ascty == 0x00)) {
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

char *dmb_drv_get_sub_ch_dat_label(int subchannel_count)
{
	int i, n;
	struct service_info_t *svc_info;
	char *label = NULL;

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if (svc_info->tmid == 0x03) {
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

struct ch_info_type *dmb_drv_get_fic_dmb(int subchannel_count)
{
	int i, n, j;
	struct esbinfo_t *esb;
	struct service_info_t *svc_info;
	u8 num_of_user_appl;

	memset((void *)&dmb_info, 0, sizeof(dmb_info));

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x01)
				&& (svc_info->dscty == 0x18)) {
				if (n == subchannel_count) {
					dmb_info.ucSubchID = svc_info->sub_channel_id;
					dmb_info.uiStartAddress = 0;
					dmb_info.ucTMId = svc_info->tmid;
					dmb_info.ucServiceType = svc_info->dscty;
					dmb_info.ulServiceID = svc_info->sid;
					dmb_info.scids = svc_info->scids;

					num_of_user_appl = svc_info->num_of_user_appl;
					dmb_info.num_of_user_appl = num_of_user_appl;
					for (j = 0; j < num_of_user_appl; j++) {
						dmb_info.user_appl_type[j] = svc_info->user_appl_type[j];
						dmb_info.user_appl_length[j] = svc_info->user_appl_length[j];
						memcpy(&dmb_info.user_appl_data[j][0], &svc_info->user_appl_data[j][0], dmb_info.user_appl_length[j]);
					}

					esb = fic_decoder_get_ensemble_info(0);
					if (esb->flag == 99)
						dmb_info.uiEnsembleID = esb->eid;
					else
						dmb_info.uiEnsembleID = 0;
					dmb_info.ecc	= esb->ecc;

					break;
				}
				n++;
			}
		}
	}

	return &dmb_info;
}

struct ch_info_type *dmb_drv_get_fic_dab(int subchannel_count)
{
	int i, n;
	struct esbinfo_t *esb;
	struct service_info_t *svc_info;

	memset((void *)&dab_info, 0, sizeof(dab_info));

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if ((svc_info->tmid == 0x00)
				&& (svc_info->ascty == 0x00)) {
				if (n == subchannel_count) {
					dab_info.ucSubchID = svc_info->sub_channel_id;
					dab_info.uiStartAddress = 0;
					dab_info.ucTMId = svc_info->tmid;
					dab_info.ucServiceType = svc_info->ascty;
					dab_info.ulServiceID = svc_info->sid;
					dab_info.scids = svc_info->scids;

					esb = fic_decoder_get_ensemble_info(0);
					if (esb->flag == 99)
						dmb_info.uiEnsembleID = esb->eid;
					else
						dmb_info.uiEnsembleID = 0;
					dab_info.ecc = esb->ecc;

					break;
				}
				n++;
			}
		}
	}

	return &dab_info;
}

struct ch_info_type *dmb_drv_get_fic_dat(int subchannel_count)
{
	int i, n, j;
	struct esbinfo_t *esb;
	struct service_info_t *svc_info;
	u8 num_of_user_appl;
	struct scInfo_t  *pScInfo;

	memset((void *)&dat_info, 0, sizeof(dat_info));

	n = 0;
	for (i = 0; i < MAX_SVC_NUM; i++) {
		svc_info = fic_decoder_get_service_info_list(i);

		if ((svc_info->flag & 0x07) == 0x07) {
			if (svc_info->tmid == 0x03) {
				if (n == subchannel_count) {
					dat_info.ucSubchID =
						svc_info->sub_channel_id;
					dat_info.uiStartAddress = 0;
					dat_info.ucTMId = svc_info->tmid;
					pScInfo = get_sc_info(svc_info->scid);
					dat_info.ucServiceType = pScInfo->dscty;
					dat_info.ulServiceID = svc_info->sid;
					dat_info.scids = svc_info->scids;

					num_of_user_appl = svc_info->num_of_user_appl;
					dat_info.num_of_user_appl = num_of_user_appl;
					for (j = 0; j < num_of_user_appl; j++) {
						dat_info.user_appl_type[j] = svc_info->user_appl_type[j];
						dat_info.user_appl_length[j] = svc_info->user_appl_length[j];
						memcpy(&dat_info.user_appl_data[j][0], &svc_info->user_appl_data[j][0], dat_info.user_appl_length[j]);
					}

					esb = fic_decoder_get_ensemble_info(0);
					if (esb->flag == 99)
						dat_info.uiEnsembleID = esb->eid;
					else
						dat_info.uiEnsembleID = 0;
					dat_info.ecc = esb->ecc;

					break;
				}
				n++;
			}
		}
	}

	return &dat_info;
}

