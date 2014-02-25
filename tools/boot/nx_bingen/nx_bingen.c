/*
   1. 2ndboot

       -- first stage --

	NSIH FILE, BIN FILE
	     |
	     |  merge
		 |
    PHASE1_FILE (16KB):  [ parsed NSIH + SecondBoot.bin ]
		 |
         |  ecc gen
		 |
    PHASE2_FILE       :  [ ecc generated PHASE1_FILE    ]



       -- second stage --

	PHASE2_FILE
		 |
	     |  spacing - 0xff
		 |
    RESULT (argv[1])  :  [ spaced        PHASE2_FILE    ]
  



   2. u-boot

       -- first stage --

	NSIH FILE, BIN FILE
		 |
	     |  merge
		 |
    PHASE1_FILE       :  [ parsed NSIH(512) + 0xff(512) + u-boot.bin ]
		 |
	     |  ecc gen
		 |
    PHASE2_FILE       :  [ ecc generated PHASE1_FILE    ]
         | 
    RESULT (argv[1])

 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <math.h>

#include "NSIH.h"
#include "GEN_NANDBOOTEC.h"

#define	VERSION_STR	"0.9.2"


/* PRINT MACRO */
#ifdef DEBUG
#define pr_debug(msg...)	do { printf("[DEBUG] " msg); } while (0)
#else
#define pr_debug(msg...)	do {} while (0)
#endif

#define pr_error(msg...)	do { printf("[ERROR] " msg); } while (0)


//
//
#define DEFAULT_LOADADDR	(0x40100000)
#define DEFAULT_LAUNCHADDR	(0x40100000)

#define	SECOND_BOOT_SIZE	(16*1024)


#define PHASE1_FILE			"PHASE1_FILE"
#define PHASE2_FILE			"PHASE2_FILE"

struct NSIH_INFO {
	uint32_t loadaddr;
	uint32_t launchaddr;
	char *NSIH_NAME;
};


//////////////////////////////////////////////////////////////////////////////
//
//	Uitility functions
//
unsigned int String2Hex ( const char *pstr )
{
	char ch;
	unsigned int value;

	value = 0;

	while ( *pstr != '\0' )
	{
		ch = *pstr++;

		if ( ch >= '0' && ch <= '9' )
		{
			value = value * 16 + ch - '0';
		}
		else if ( ch >= 'a' && ch <= 'f' )
		{
			value = value * 16 + ch - 'a' + 10;
		}
		else if ( ch >= 'A' && ch <= 'F' )
		{
			value = value * 16 + ch - 'A' + 10;
		}
	}

	return value;
}

unsigned int String2Dec ( const char *pstr )
{
	char ch;
	unsigned int value;

	value = 0;

	while ( *pstr != '\0' )
	{
		ch = *pstr++;

		if ( ch >= '0' && ch <= '9' )
		{
			value = value * 10 + ch - '0';
		}
	}

	return value;
}

void Hex2StringByte(unsigned char data, unsigned char *string)
{
	unsigned char hex;

	hex = data >> 4;
	if (hex >= 10)
		hex += 'A' - 10;
	else
		hex += '0';
	*string++ = hex;

	hex = data & 0xF;
	if (hex >= 10)
		hex += 'A' - 10;
	else
		hex += '0';
	*string++ = hex;

	*string = ' ';
}

static size_t get_file_size( FILE *fd )
{
	size_t fileSize;
	long curPos;

	curPos = ftell( fd );

	fseek(fd, 0, SEEK_END);
	fileSize = ftell( fd );
	fseek(fd, curPos, SEEK_SET );

	return fileSize;
}

//
//	end of utilities functions
//
//////////////////////////////////////////////////////////////////////////////


int merge_nsih_secondboot(const char*nsih_file, const char *in_file, const char *out_file, const struct NSIH_INFO *NSIH_INFO, int is_2ndboot)
{
	int ret = 0;
	FILE *nish_fd = NULL;
	FILE *in_fd = NULL;
	FILE *out_fd = NULL;
	unsigned char *out_buf = NULL;

	int out_buf_size = SECOND_BOOT_SIZE;
	int in_file_size = 0;
	size_t read_size = 0;
	size_t write_size = 0;

	struct NX_SecondBootInfo *bootinfo;

	nish_fd = fopen(nsih_file, "rb");
	in_fd = fopen(in_file, "rb");
	out_fd = fopen(out_file, "wb");

	if( !nish_fd || !in_fd || !out_fd )
	{
		ret = -1;
		pr_error("Cannot open file!!!\n");
		goto ERROR_EXIT;
	}

	in_file_size = get_file_size( in_fd );
	if( !is_2ndboot )
	{
		out_buf_size = in_file_size + 512;
	}
	if( in_file_size + NSIH_BIN_SIZE > out_buf_size )
	{
		ret = -1;
		pr_error("Input file too large(%d)!!!\n", in_file_size);
		goto ERROR_EXIT;
	}

	out_buf = (unsigned char*)malloc( out_buf_size );
	if( NULL == out_buf )
	{
		ret = -1;
		pr_error("Out buffer allocation failed!!(%d)\n", out_buf_size);
		goto ERROR_EXIT;
	}
	memset( out_buf, 0xff, out_buf_size );

	//--------------------------------------------------------------------------
	// NSIH parsing
	ret = ProcessNSIH (nsih_file, out_buf);
	if (ret != NSIH_BIN_SIZE)
	{
		pr_error ("NSIH Parsing failed.\n");
		ret = -1;
		goto ERROR_EXIT;
	}
	ret = 0;

	bootinfo = (struct NX_SecondBootInfo *)out_buf;
	bootinfo->LOADSIZE = in_file_size;
	bootinfo->LOADADDR = NSIH_INFO->loadaddr;
	bootinfo->LAUNCHADDR = NSIH_INFO->launchaddr;
	bootinfo->SIGNATURE = HEADER_ID;

	read_size = fread( out_buf + NSIH_BIN_SIZE, 1, in_file_size, in_fd );
	if( read_size != in_file_size )
	{
		pr_error ("File read error.\n");
		ret = -1;
		goto ERROR_EXIT;
	}

	write_size = fwrite( out_buf, 1, out_buf_size, out_fd );
	if( write_size != out_buf_size )
	{
		pr_error ("Out file write failed.\n");
		ret = -1;
		goto ERROR_EXIT;
	}

ERROR_EXIT:
	if( nish_fd )	fclose( nish_fd );
	if( in_fd )		fclose( in_fd );
	if( out_fd )	fclose( out_fd );
	if( out_buf )	free( out_buf );

	if( ret == 0 )
	{
		printf("Image Generating Success!!!\n\n");
	}
	else
	{
		printf("Image Generating Failed!!!\n\n");
	}
	return ret;
}


//////////////////////////////////////////////////////////////////////////////
//
//	-- first stage --
//
int first_stage (const char *OUT_BINNAME, const char *IN_BINNAME, const struct NSIH_INFO *NSIH_INFO, int is_2ndboot)
{
	FILE	*bin_fp;
	FILE	*phase1_fp, *phase2_fp;
	unsigned int BinFileSize, SrcLeft, SrcReadSize, DstSize, DstTotalSize;
	static unsigned int pdwSrcData[7*NX_BCH_SECTOR_MAX/4], pdwDstData[8*NX_BCH_SECTOR_MAX/4];
	size_t sz;

	unsigned char *outbuf = NULL;
	struct NX_SecondBootInfo *bootinfo;
	int outbuf_sz = 0;
	int bin_offset = iSectorSize;

	int ret = 0;


	bin_fp = fopen (IN_BINNAME, "rb");
	if (!bin_fp)
	{
		printf ("ERROR : Failed to open %s file.\n", IN_BINNAME);
		return -1;
	}

	BinFileSize = get_file_size (bin_fp);
	pr_debug ("BinFileSize 1: %d\n", BinFileSize);


	// calulation buffer size
	outbuf_sz = ALIGN(BinFileSize, BUFSIZE);
	pr_debug ("outbuf_sz: %d\n", outbuf_sz);

	if (is_2ndboot && (iSectorSize + BinFileSize) > outbuf_sz)
	{
		pr_error ("BIN is out of range.\n");
		fclose (bin_fp);
		return -1;
	}

	// buffer allocation
	outbuf = malloc(outbuf_sz);
	if (!outbuf)
	{
		pr_error ("Not enough memory.");
		fclose (bin_fp);
		return -1;
	}
	memset (outbuf, 0xff, outbuf_sz);


	//--------------------------------------------------------------------------
	// NSIH parsing
	ret = ProcessNSIH (NSIH_INFO->NSIH_NAME, outbuf);
	if (ret != NSIH_BIN_SIZE)
	{
		pr_error ("NSIH Parsing failed.\n");
		fclose (bin_fp);
		return -1;
	}

	bootinfo = (struct NX_SecondBootInfo *)outbuf;
	bootinfo->LOADSIZE = BinFileSize;
	bootinfo->LOADADDR = NSIH_INFO->loadaddr;
	bootinfo->LAUNCHADDR = NSIH_INFO->launchaddr;
	bootinfo->SIGNATURE = HEADER_ID;


	//--------------------------------------------------------------------------
	// BIN processing
	if (is_2ndboot)
		bin_offset = 512;
	else
		bin_offset = 1024;

	sz = fread (outbuf + bin_offset, 1, BinFileSize, bin_fp);
	fclose (bin_fp);


	pr_debug ("bootinfo sz: %d loadaddr: 0x%x, launchaddr 0x%x\n",
		bootinfo->LOADSIZE, bootinfo->LOADADDR, bootinfo->LAUNCHADDR);


	phase1_fp = fopen (PHASE1_FILE, "wb");
	if (!phase1_fp)
	{
		pr_error ("failed to create %s file.\n", PHASE1_FILE);
		return -1;
	}
	fwrite (outbuf, 1, outbuf_sz, phase1_fp);

	fclose (phase1_fp);


	phase1_fp = fopen (PHASE1_FILE, "rb");

	BinFileSize = get_file_size (phase1_fp);
	pr_debug ("BinFileSize 2: %d\n", BinFileSize);



	if (is_2ndboot && BinFileSize > BUFSIZE)
	{
		printf ("WARNING : %s is too big for Second boot image, only will used first %d bytes.\n", IN_BINNAME, BUFSIZE);
		BinFileSize = BUFSIZE;
	}

	pr_debug ("iSectorSize: %d\n", iSectorSize);

	iNX_BCH_VAR_K		=	(iSectorSize * 8);

	iNX_BCH_VAR_N		=	(((1<<iNX_BCH_VAR_M)-1));
	iNX_BCH_VAR_R		=	(iNX_BCH_VAR_M * iNX_BCH_VAR_T);

	iNX_BCH_VAR_TMAX	=	(60);
	iNX_BCH_VAR_RMAX	=	(iNX_BCH_VAR_M * iNX_BCH_VAR_TMAX);

	iNX_BCH_VAR_R32		=	((iNX_BCH_VAR_R   +31)/32);
	iNX_BCH_VAR_RMAX32	=	((iNX_BCH_VAR_RMAX+31)/32);

	iNX_BCH_OFFSET		= 8;

	SrcLeft = BinFileSize;

	//--------------------------------------------------------------------------
	// Encoding - ECC Generation
	printf ("\n");

	DstTotalSize = 0;

	// generate the Galois Field GF(2**mm)
	generate_gf ();
	// Compute the generator polynomial and lookahead matrix for BCH code
	gen_poly ();



	phase2_fp = fopen (OUT_BINNAME, "wb");
	if (!phase2_fp)
	{
		printf ("ERROR : Failed to create %s file.\n", OUT_BINNAME);
		fclose (phase1_fp);
		return -1;
	}

	while (SrcLeft > 0)
	{
		size_t sz;
		SrcReadSize = (SrcLeft > (iSectorSize*7)) ? (iSectorSize*7) : SrcLeft;
		sz = fread (pdwSrcData, 1, SrcReadSize, phase1_fp);
		SrcLeft -= SrcReadSize;
		pr_debug ("SrcLeft: %x, SrcReadSize: %x\n", SrcLeft, SrcReadSize);
		DstSize = MakeECCChunk (pdwSrcData, pdwDstData, SrcReadSize);
		fwrite (pdwDstData, 1, DstSize, phase2_fp);
		
		DstTotalSize += DstSize;
	}

	//--------------------------------------------------------------------------
	printf ("\n");
	printf ("%d bytes(%d sector) generated.\n", DstTotalSize, (DstTotalSize+iSectorSize-1)/iSectorSize);

	fclose (phase1_fp);
	fclose (phase2_fp);


	return 0;
}

//
//	-- end of first stage --
//
//////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////////
//
//	-- second stage --
//
int second_stage(const char *OUT_BINNAME, const char *IN_BINNAME, const int page_size)
{
	FILE *in_fp;
	FILE *out_fp;

	char *data = NULL;

	int data_in_page = page_size;
	int BinFileSize;
	int Cnt, i;
	int ret = 0;


	/* prepare buffer */
	data = malloc (page_size);
	if (!data)
	{
		ret = -10;
		goto err;
	}


	in_fp = fopen (IN_BINNAME, "rb");
	if (!in_fp) {
		ret = -20;
		goto err_in_fp;
	}
	out_fp = fopen (OUT_BINNAME, "wb");
	if (!out_fp) {
		ret = -30;
		goto err_out_fp;
	}


	/* writing size calc. */
	BinFileSize = get_file_size (in_fp);
	pr_debug ("BinFileSize 3: %d\n", BinFileSize);

	data_in_page = MIN (data_in_page, MAXPAGE);
	Cnt = DIV_ROUND_UP (BinFileSize, data_in_page);
	pr_debug ("Cnt: %d\n", Cnt);


	/* writing */
	for (i = 0; i < Cnt; i++)
	{
		size_t sz;

		memset (data, 0xff, page_size);

		sz = fread (data, 1, data_in_page, in_fp);
		fwrite (data, 1, page_size, out_fp);
	}

	fclose (in_fp);
	fclose (out_fp);

	return 0;

err_out_fp:
	fclose (out_fp);
err_in_fp:
	printf ("err:%d\n", ret);
err:
	return ret;
}
//
//		--- end of second stage ---
//
//////////////////////////////////////////////////////////////////////////////


void print_usage(char *argv[])
{
	//                 1         2         3         4         5         6         7         8
	//        12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("\n==============================================================================\n");
	printf("Description : Nexell Binary Generator with ECC Generator\n");
	printf("version     : %s, %s, %s\n", VERSION_STR, __DATE__, __TIME__);
	printf("\n");
	printf("usage: options\n");
	printf(" -h help                                                       \n");
	printf(" -t image type, 2ndboot|bootloader (mandatory)                 \n");
	printf(" -d device type, nand|other        (mandatory)                 \n");
	printf(" -i input file name                (mandatory)                 \n");
	printf(" -o output file name               (mandatory)                 \n");
	printf(" -n nsih file name                 (mandatory)                 \n");
	printf(" -p page size  (KiB)               (optional, nand only, default %d )    \n", MAXPAGE);
	printf(" -l load address                   (optional, default 0x%08x )\n", DEFAULT_LOADADDR);
	printf(" -e launch address                 (optional, default 0x%08x )\n", DEFAULT_LAUNCHADDR);
	printf("\nExample: nand image\n");
	printf(" %s -t 2ndboot -d nand -o nand_2ndboot.bin -i pyrope_2ndboot_NAND.bin -n NSIH.txt -p 4096 -l 0x40100000 -e 0x40100000 \n", argv[0]);
	printf(" %s -t bootloader -d nand -o nand_bootloader.bin -i u-boot.bin -n NSIH.txt -p 4096 -l 0x40100000 -e 0x40100000 \n", argv[0]);
	printf("\nExample: normal image\n");
	printf(" %s -t 2ndboot -d other -o 2ndboot.bin -i pyrope_2ndboot_USB.bin -n NSIH.txt -l 0x40100000 -e 0x40100000 \n", argv[0]);
	printf(" %s -t bootloader -d other -o bootloader.bin -i u-boot.bin -n NSIH.txt -l 0x40100000 -e 0x40100000 \n", argv[0]);
	printf("==============================================================================\n");
	printf("\n");
}


//------------------------------------------------------------------------------

#define BUILD_NONE					0x0
#define BUILD_2NDBOOT				0x1
#define BUILD_BOOTLOADER			0x2

#define	DEVICE_NONE					0x0
#define	DEVICE_NAND					0x1
#define	DEVICE_OTHER				0x2

int main (int argc, char** argv)
{
	int page_size = MAXPAGE;
	uint32_t sector_size = 0;

	struct NSIH_INFO NSIH_INFO;
	uint32_t loadaddr = DEFAULT_LOADADDR;
	uint32_t launchaddr = DEFAULT_LAUNCHADDR;

	char *bin_type = NULL;
	char *dev_type = NULL;
	char *out_file = NULL;
	char *in_file = NULL;
	char *nsih_file = NULL;
	int build_type = BUILD_NONE;
	int device_type = DEVICE_NONE;

	int opt;


	// arguments parsing 
	while (-1 != (opt = getopt(argc, argv, "ht:d:o:i:n:p:l:e:"))) {
		switch(opt) {
			case 't':
				bin_type = optarg;
				break;
			case 'd':
				dev_type = optarg;
				break;
			case 'o':
				out_file = optarg;
				break;
			case 'i':
				in_file = optarg;
				break;
			case 'n':
				nsih_file = optarg;
				break;
			case 'p':
				page_size = strtoul (optarg, NULL, 10);
				break;
			case 'l':
				loadaddr = strtoul (optarg, NULL, 16);
				break;
			case 'e':
				launchaddr = strtoul (optarg, NULL, 16);
				break;
			case 'h':
			default:
				print_usage(argv);
				exit(0);
				break;
		}
	}


	// argument check
	if (!bin_type || !dev_type || !out_file || !in_file || !nsih_file)
	{
		print_usage(argv);
		return -1;
	}

	if (page_size < 512 || !IS_POWER_OF_2 (page_size))
	{
		pr_error ("Page size must bigger than 512 and must power-of-2\n");
		return -1;
	}

	//	parse Board Type
	if (!strcmp(bin_type, "2ndboot"))
		build_type = BUILD_2NDBOOT;
	else if(!strcmp(bin_type, "bootloader"))
		build_type = BUILD_BOOTLOADER;
	pr_debug ("build type : %d\n", build_type);
	if (build_type == BUILD_NONE)
	{
		pr_error ("Enter 2ndboot or bootloader\n");
		return -1;
	}

	//	parse Device Type
	if (!strcmp(dev_type, "nand"))
		device_type = DEVICE_NAND;
	if (!strcmp(dev_type, "other"))
		device_type = DEVICE_OTHER;
	pr_debug ("device type : %d\n", device_type);
	if (device_type == DEVICE_NONE)
	{
		pr_error ("Enter nand or other\n");
		return -1;
	}

	// if( device_type==DEVICE_OTHER && build_type!=BUILD_2NDBOOT )
	// {
	// 	pr_error ("Other device support only second boot mode.\n");
	// 	return -1;
	// }

	if (!strcmp(out_file, PHASE1_FILE) || !strcmp(out_file, PHASE2_FILE)
		|| !strcmp(in_file, PHASE1_FILE) || !strcmp(in_file, PHASE2_FILE)
		|| !strcmp(nsih_file, PHASE1_FILE) || !strcmp(nsih_file, PHASE2_FILE))
	{
		pr_error ("Do not using %s or %s as file name.\n", PHASE1_FILE, PHASE2_FILE);
		return -1;
	}

	if( device_type == DEVICE_NAND )
	{
		// get sector size
		sector_size = (page_size < 1024) ? 512 : 1024;
		if (sector_size == 512)
		{
			iSectorSize			=	512;
			iNX_BCH_VAR_T		=	24;
			iNX_BCH_VAR_M		=	13;
		}
		else if (sector_size == 1024)
		{
			iSectorSize			=	1024;
			iNX_BCH_VAR_T		=	60;
			iNX_BCH_VAR_M		=	14;
		}
		pr_debug("page size: %d, sector size: %d\n", page_size, sector_size);
	}

	NSIH_INFO.loadaddr = loadaddr;
	NSIH_INFO.launchaddr = launchaddr;
	NSIH_INFO.NSIH_NAME = nsih_file;

	//                 1         2         3         4         5         6         7         8
	//        12345678901234567890123456789012345678901234567890123456789012345678901234567890
	printf("\n==============================================================================\n");
	printf(" Type        : %s\n", bin_type);
	printf(" Device      : %s\n", dev_type);
	printf(" Input       : %s\n", in_file);
	printf(" Output      : %s\n", out_file);
	printf(" NSIH        : %s\n", nsih_file);
	printf(" Load Addr   : 0x%x\n", NSIH_INFO.loadaddr);
	printf(" Luanch Addr : 0x%x\n", NSIH_INFO.launchaddr);
	printf(" Signature   : 0x%x\n", HEADER_ID);
	printf("==============================================================================\n");

	//  make image
	if (build_type == BUILD_2NDBOOT)
	{
		if( device_type == DEVICE_NAND )
		{
			first_stage  (PHASE2_FILE, in_file, &NSIH_INFO, 1);
			second_stage (out_file, PHASE2_FILE, page_size);
		}
		else
		{
			merge_nsih_secondboot(nsih_file, in_file, out_file, &NSIH_INFO, 1);
		}
	}
	else	// BUILD_BOOTLOADER
	{
		if( device_type == DEVICE_NAND )
		{
			first_stage  (out_file, in_file, &NSIH_INFO, 0);
		}
		else
		{
			merge_nsih_secondboot(nsih_file, in_file, out_file, &NSIH_INFO, 0);
		}
	}

	return 0;
}

