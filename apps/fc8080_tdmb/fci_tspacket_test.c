#include <stdio.h>
#include <string.h>

/*
 * Max Demux Count
 */
#define MAX_DEMUX           2

/*
 * Sync Byte 0xb8
 */
#define SYNC_BYTE_INVERSION

struct pid_info {
	unsigned long count;
	unsigned long discontinuity;
	unsigned long continuity;
};

struct demux_info {
	struct pid_info  pids[8192];

	unsigned long    ts_packet_c;
	unsigned long    malformed_packet_c;
	unsigned long    tot_scraped_sz;
	unsigned long    packet_no;
	unsigned long    sync_err;
	unsigned long 	 sync_err_set;
};

static int is_sync(unsigned char* p) {
	int syncword = p[0];
#ifdef SYNC_BYTE_INVERSION
	if(0x47 == syncword || 0xb8 == syncword)
		return 1;
#else
	if(0x47 == syncword)
		return 1;
#endif
	return 0;
}
static struct demux_info demux[MAX_DEMUX];

int err;
int print_pkt_log()
{
	unsigned long i=0;

	print_log(0, "\n DMB PKT_TOT : %d, SYNC_ERR : %d, SYNC_ERR_BIT : %d, ERR_PKT : %d err : %d\n", demux[0].ts_packet_c, demux[0].sync_err, demux[0].sync_err_set, demux[0].malformed_packet_c, err);

#if 1
	for(i=0;i<8192;i++)
	{
		if(demux[0].pids[i].count>0)
			print_log(0, "DMB PID : 0x%x, TOT_PKT : %d, DISCONTINUITY : %d \n", i, demux[0].pids[i].count, demux[0].pids[i].discontinuity);
	}
#endif	
}

int put_ts_packet(int no, unsigned char* packet, int sz) {
	unsigned char* p;
	int transport_error_indicator, pid, payload_unit_start_indicator, continuity_counter, last_continuity_counter;
	int i;
	if ((sz % 188)) {
		print_log(0, "L : %d\n", sz);
	} else {
		for(i = 0; i < sz; i += 188) {
			p = packet + i;

			pid = ((p[1] & 0x1f) << 8) + p[2];

			demux[no].ts_packet_c++;
			if(!is_sync(packet + i)) {
				print_log(0, "DMB SYNC ERROR/n");
				if(0x80!=(p[1] & 0x80))
					demux[no].sync_err_set++;
				else
					demux[no].sync_err++;
				continue;
			}

			// Error Indicator가 설정되면 Packet을 버림
			transport_error_indicator = (p[1] & 0x80) >> 7;
			if(1 == transport_error_indicator) {
				demux[no].malformed_packet_c++;
				continue;
			}

			payload_unit_start_indicator = (p[1] & 0x40) >> 6;

			demux[no].pids[pid].count++;

			// Continuity Counter Check
			continuity_counter = p[3] & 0x0f;

			if(demux[no].pids[pid].continuity == -1) {
				demux[no].pids[pid].continuity = continuity_counter;
			} else {
				last_continuity_counter = demux[no].pids[pid].continuity;

				demux[no].pids[pid].continuity = continuity_counter;

				if(((last_continuity_counter + 1) & 0x0f) != continuity_counter)
					demux[no].pids[pid].discontinuity++;
			}
		}
	}
	return 0;
}

void create_tspacket_anal() {
	int n, i;

	for(n = 0; n < MAX_DEMUX; n++) {
		memset((void*)&demux[n], 0, sizeof(demux[n]));

		for(i = 0; i < 8192; i++) {
			demux[n].pids[i].continuity = -1;
		}
	}
}

