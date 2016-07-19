#include <stdio.h>
#include <string.h>
#include "fci_types.h"

#define PROMPT	  "DMB"

char *help_msg[] = {
     "-----------------------------------------------------------",
     	 "? 	    : help display [mem/peri]",
     "-----------------------------------------------------------",
	 "brd       : byte read command	    [start addr] [length]",
	 "bwr       : byte write command    [start addr] [data] [length]",
	 "wrd       : word read command     [start addr] [length]",
	 "wwr       : word write command    [start addr] [data] [length]",
	 "lrd       : long read command     [start addr] [length]",
	 "lwr       : long write command    [start addr] [data] [length]",
     "-----------------------------------------------------------",
	 "verify    : [address] [retry]",
	 "iverify   : [address] [retry]",
     "-----------------------------------------------------------",
	 "tuner     : [frequency]",
     "-----------------------------------------------------------",
	 "reset     : ",
	 "init      : ",
	 "deinit    : ",
	 "-----------------------------------------------------------",
	 "ird		: [start addr] [length]",
	 "iwr		: [start addr] [data]",
	 "pwron 	: ",
	 "pwroff	: ",
	 "-----------------------------------------------------------",
	 "ficload	: [filename]",
	 "mscload	: [filename]",
	 "fic_dump	: ",
	 "msc_dump	: [subChId][svcId]",
	 "-----------------------------------------------------------",
	 "channel	: [subChId][svcId]",
	 "dechannel	: [svcId]",
	 "-----------------------------------------------------------",
	 "dm		: ",
	 "sig		: ",
	 "rssi		: ",
	 "scan		: ",
     	 "exit      : fccon reset",
     ""
};

char *argv[10];
int  argc;
char hist_buf[10][100];
char buf[100];
int  cnt;


/* white-space is ' ' ',' '\f' '\n' '\r' '\t' '\v' */

int isspace2(char c)
{
	if (c==' ' || c==',' || c=='\f' || c=='\n' || c=='\r' || c=='\t' || c=='\v') return 1;
	else return 0;
}

int make_argv(char *s, int argvsz, char *argv[])
{
        int argc = 0;

        /* split into argv */
        while (argc < argvsz - 1) {

                /* skip any white space */
                while ((*s == ' ') || (*s == '\t') || (*s == 0x0a))
                        ++s;

                if (*s == '\0')         /* end of s, no more args       */
                        break;

                argv[argc++] = s;       /* begin of argument string     */

                /* find end of string */
                while (*s && (*s != ' ') && (*s != '\t') && (*s != 0x0a))
                        ++s;

                if (*s == '\0')         /* end of s, no more args       */
                        break;

                *s++ = '\0';            /* terminate current arg         */
        }
        argv[argc] = NULL;

        return argc;
}

void version_display(void)
{
	print_log(NULL, "\n");
	print_log(NULL, "\t*********************************************\n");
	print_log(NULL, "\t*      FCI   DMB RECEIVER MODEL             *\n");
	print_log(NULL, "\t*                                           *\n");
	print_log(NULL, "\t*         %s  %s             *\n", __DATE__,  __TIME__);
	print_log(NULL, "\t*                                           *\n");
	print_log(NULL, "\t*       Digital Multimedia Broadcast        *\n");
	print_log(NULL, "\t*                                           *\n");
	print_log(NULL, "\t*            http://www.fci.co.kr           *\n");
	print_log(NULL, "\t*********************************************\n");

	print_log(NULL, "\tVersion Information\n");
	print_log(NULL, "\tFunctions:\n");
	print_log(NULL, "\n");
}

void Mon_help_command(void)
{
	int  line;
	char **ptr;
	line = 0;

	ptr = help_msg;


	while(1)
	{
		if(*ptr[0] == 0) break;

		print_log(NULL, "%s\n", (int)*ptr++);
		if(++line % 30 == 0)
		{
			print_log(NULL, "\nPress return key when ready...");
			getchar();
		}
	}

}

void Monitor(void)
{
	char  tmp_buf[100];
	int   index = 0;
	int   i;
	int minute, prev_min, sec, prev_sec;

	u8 data;
	u16 wdata;
	u32 ldata;
	u8 temp = 0x1e;

	minute = prev_min = sec = prev_sec = 0;

	print_log(NULL, "\nTask TDMB Start...\n");

	for(cnt=0; cnt<10; cnt++) hist_buf[cnt][0] = '\0';
	cnt = 0;

#if 1

	mmi_bbm_init_cmd(NULL, NULL);
	//ms_wait(1);
	mmi_bbm_wread_1();
 
#if 1
#if 1// PJSIN 20160714 add-- [ 1
	mmi_bbm_write1(0x00a4, 0xAA); 
#if 0// PJSIN 20160714 add-- [ 1 
	mmi_bbm_lread1(0x00a4, &ldata);
	print_log(NULL, "FC8080_TDMB byte test lread (0x%x,0x%x)\r\n", 0xAA, ldata);
	mmi_bbm_read1(0xa4, &ldata);
	print_log(NULL, "FC8080_TDMB byte test read (0x%x,0x%x)\r\n", 0xAA, ldata);


	mmi_bbm_lwrite1(0x00a4, 0xAA); 
	mmi_bbm_lread1(0x00a4, &ldata);
	print_log(NULL, "FC8080_TDMB byte test lread (0x%x,0x%x)\r\n", 0xAA, ldata);
	mmi_bbm_read1(0xa4, &ldata);
	print_log(NULL, "FC8080_TDMB byte test read (0x%x,0x%x)\r\n", 0xAA, ldata);
#endif// ]-- end 


#else
	while ( 1 )
	{
		ms_wait(1000);

		print_log(NULL, "index %d \r\n", index++);
#if 1// PJSIN 20160713 add-- [ 1 
		print_log(NULL, "FC8080_TDMB byte test \r\n");
		for (i = 0; i < 100; i++) {
			mmi_bbm_write1(0xa4, i & 0xff);
			mmi_bbm_read1(0xa4, &data);
			if ((i & 0xff) != data){
				print_log(NULL, "FC8080_TDMB byte test read error!(0x%x,0x%x)\r\n", i & 0xff, data);
				return; 
			}
		}

		print_log(NULL, "FC8080_TDMB word test \r\n");
		for (i = 0; i < 100; i++) {
			mmi_bbm_wwrite1(0xa4, i & 0xffff);
			mmi_bbm_wread1(0xa4, &wdata);
			if ((i & 0xffff) != wdata)
				print_log(NULL, "FC8080_TDMB word test read error!(0x%x,0x%x)\r\n", i & 0xffff, wdata);
		}
#endif// ]-- end 

		print_log(NULL, "FC8080_TDMB long test \r\n");
		for (i = 0; i < 100; i++) {
			mmi_bbm_lwrite1(0xa4, i & 0xffffffff);
			mmi_bbm_lread1(0xa4, &ldata);
			if ((i & 0xffffffff) != ldata)
				print_log(NULL, "FC8080_TDMB long test read error!(0x%x,0x%x)\r\n", i & 0xffffffff, ldata);
		}

		print_log(NULL, "\r\n");
	}
#endif// ]-- end 
#endif


	//print_log(NULL, "call mmi_tuner_set_1. \n");
	//mmi_tuner_set_1();

#if 0// PJSIN 20160711 add-- [ 1 

	ms_wait(10);
	mmi_tuner_set_1();

	ms_wait(3000);
	mmi_msc_dump_cmd_1();

	while(1);
#endif// ]-- end 

	print_log(NULL, "\nTask TDMB Exit...\n");

#else

	while ( 1 )
	{
		print_log(NULL, "[%s] ", PROMPT);
		fgets(buf, 100, stdin );


		if(buf[0] == '.')	/* Previous command */
		{
			strcpy(buf, "!!");
		}

		if(buf[0] == '!')	/* History command */
		{
			if(buf[1] == '!' || (buf[1] >='0' && buf[1] <='9'))
			{
				if(buf[1] == '!')  index = (cnt+9) % 10;
				else	       index = buf[1] - '0';

				if(buf[2] == '\0' || buf[3] == '\0')
				{
					strcpy(buf, hist_buf[index]);
				}
				else
				{
					sprintf(tmp_buf, "%s %s", hist_buf[index], &buf[3]);
					strcpy(buf, tmp_buf);
				}
				print_log(NULL, "\n%s", (int)buf);
			}
			else
			{
				for(i=0 ; i<10 ; i++)
				{
					print_log(NULL, "%d %s\n", i, hist_buf[i]);
				}
				continue;
			}
		}

		strcpy(tmp_buf, buf);
		argc = make_argv(buf, sizeof(argv)/sizeof(argv[0]), argv);

		if ( argc != 0 )
		{
			anal_command(argc, argv);
			strcpy(hist_buf[cnt], tmp_buf);
			cnt++;
			if(cnt == 10) cnt = 0;
		}
	}
#endif
}

