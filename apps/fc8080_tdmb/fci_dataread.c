#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include "fc8080_regs.h"
#include "fci_types.h"
#include "fic.h"
#include "fci_oal.h"
#include "fci_dal.h"

#define FIC_BUF_SIZE 1024
#define MSC_BUF_SIZE 8096
#define DUMP_PATH "/data/"

#define DT_FIC 0x02
#define DT_MSC 0x04

static pthread_t p_thr_msc;
static pthread_t p_thr_get_fic;
static pthread_t p_thr_sig;

int msc_dump_flag = 0;
int fic_dump_flag = 0;
int sig_mon_flag = 0;

typedef struct _msc_dump_param{
	int num;
	int dev;
	int subch;
} msc_dump_param;

u8 data[MSC_BUF_SIZE];
u8 fic_data[FIC_BUF_SIZE];

void msc_dump_thread(void *param)
{
	msc_dump_param *msc = (msc_dump_param *)param;
	int hDevice = msc->dev;
	int num = msc->num;
	int subch = msc->subch;
	int index = 0;
	int read_size = 0;
	int tot_read_size = 0;

	if (num == SVC_VIDEO)
		tdmb_video_select(hDevice, subch);
	else if (num == SVC_AUDIO)
		tdmb_audio_select(hDevice, subch);
	else
		tdmb_data_select(hDevice, subch);

	fic_dump_flag = 0;
	{
		FILE *f;
		char f_name[128];

		if (num == 0x18)
			sprintf(f_name, DUMP_PATH"dmb_video_%04d.ts", index++);
		else if (num == 0x00)
			sprintf(f_name, DUMP_PATH"dmb_audio_%04d.ts", index++);
		else
			sprintf(f_name, DUMP_PATH"dmb_data_%04d.ts", index++);

		f = fopen(f_name, "wb");

		print_log(0, "DMB start msc dump %s	%s SUBCH(0x%x)\n", f_name
			, (num == 0x18)?"VIDEO":(num == 0x00)?"AUDIO":"DATA", subch);
		create_tspacket_anal();

		while(1)
		{
 			int size = tdmb_data_read(hDevice, data, MSC_BUF_SIZE);

			if (size > 0) {
				if (num == 0x18)
					put_ts_packet(0, data, size);
//nexell 20160512
				//fwrite(data, 1, size, f);
				read_size += size;
				tot_read_size += size;

				//print_log(0, ".");
			}

			if ((size > 0) && (read_size > (size * 100))) {

//nexell 20160512

#if 0
				print_log(0, "*");
#else

				u32 vBER, rBER;
				s32 rssi;
				u8 lock;
				
				//print_pkt_log();
				tdmb_signal_quality_info(hDevice, &lock, &vBER, &rBER, &rssi);
				read_size = 0;
				print_log(0, "DMB Total Read size ... %d Bytes %d\n", tot_read_size, size);
#endif
			}

			if (msc_dump_flag == 0)
				break;

		}
		fclose(f);

		print_log(0, "end msc dump\n");
		index %= 10;
	}
	if(num == SVC_VIDEO)
		tdmb_video_deselect(hDevice);
	else if (num == SVC_AUDIO)
		tdmb_audio_deselect(hDevice);
	else
		tdmb_data_deselect(hDevice, subch);
}


int msc_dump_start(int hDevice, u8 subChId, u8 svcid)
{
    int thr_id;
	static msc_dump_param param;
	param.dev = hDevice;
	param.num = svcid;
	param.subch = subChId;

	msc_dump_flag = 1;
	thr_id = pthread_create(&p_thr_msc, NULL, (void*)&msc_dump_thread, (void*)&param);
	if(thr_id <0)
 	{
		msc_dump_flag = 0;
    	print_log(0, "msc read thread create error.\n");
    	return -1;
	}
	pthread_detach(p_thr_msc);

    return 0;
}

int msc_dump_stop()
{
	msc_dump_flag = 0;
}

void get_fic_thread(void *param)
{
	msc_dump_param *msc = (msc_dump_param *)param;
	int hDevice = msc->dev;
	int num = msc->num;
	int ret;
	int size;
	u16 buf_en;

	FILE *f;
	char f_name[128];
	sprintf(f_name, DUMP_PATH"fic_%d.ts", num);

	f = fopen(f_name, "wb");

	tdmb_fic_select(hDevice);
	BBM_WORD_READ(hDevice, BBM_BUF_ENABLE, &buf_en);
	BBM_WORD_WRITE(hDevice, BBM_BUF_ENABLE, (buf_en | 0x0100));

	while(fic_dump_flag)
	{
		size = tdmb_data_read(hDevice, fic_data, MSC_BUF_SIZE);

		if (!(size % 384)) {
			ret = fic_decoder_put((struct fic*)fic_data, size);

			if (ret) {
				fprintf(stderr,"X");
				print_log(0, "ret = %d, size = %d\n", ret, size);
			} else
				fprintf(stderr,"_");
			fwrite(fic_data, 1, size, f);
		} else
			ms_wait(10);
	}
	fclose(f);
	BBM_WORD_WRITE(hDevice, BBM_BUF_ENABLE, buf_en);
	tdmb_fic_deselect(hDevice);
}

int get_fic_data(int hFic, int freq)
{
	int thr_id;

	static msc_dump_param param;
	param.dev = hFic;
	param.num = freq;

	fic_dump_flag = 1;

	thr_id = pthread_create(&p_thr_get_fic, NULL, (void*)&get_fic_thread, (void*)&param);
	if(thr_id <0)
	{
		print_log(0, "fic thread create error.\n");
		return -1;
	}

	pthread_detach(p_thr_get_fic);

	return 0;
}

int stop_get_fic()
{
	fic_dump_flag = 0;
}

void sig_thread(void *param)
{
	int hDevice = (int)param;

	while(sig_mon_flag)
	{
		u32 vBER, rBER;
		s32 rssi;
		u8 lock;

		tdmb_signal_quality_info(hDevice, &lock, &vBER, &rBER, &rssi);
		ms_wait(1000);
	}
	print_log(0, "end sig thread\n");
}

int sig_thread_start(int handle)
{
	int thr_id;

	sig_mon_flag= 1;

	thr_id = pthread_create(&p_thr_sig, NULL, (void *)&sig_thread, (void *)handle);
	if(thr_id <0)
	{
		print_log(0, "sig nonitor thread create error.\n");
		return -1;
	}

	pthread_detach(p_thr_sig);

	return 0;
}

int sig_thread_stop()
{
	sig_mon_flag = 0;
}

