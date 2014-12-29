#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>

#include "BOOT_BINGEN.h"

#define DEBUG	0

struct NX_SecondBootInfo* pBootInfo;

static U8* cpu_name			= NULL;
static U8* option_name		= NULL;
static U8* boot_mode		= NULL;
static U8* input_name 		= NULL;
static U8* nsih_name 		= NULL;
static U8* output_name		= NULL;

static U8* SwapEnb			= NULL;

static U32 device_addr      = NULL;
static U32 device_portnum	= NULL;

static U32 OutputSize		= NULL;
static U32 InputSize		= NULL;

static U32 loadaddr			= NULL;
static U32 launchaddr		= NULL;

int main(int argc, char **argv)
{

	FILE *InFile_fd		= NULL;
	FILE *OutFile_fd	= NULL;

	U32 param_opt 		= NULL;

	U8* Out_Buffer		= NULL;
	U8* pdwBuffer		= NULL;

	U32	MallocSize		= NULL;

	U32 i = NULL, ret	= TRUE;

	U8* view_option		= NULL;

	U8 inFile_Name[50]  = "./bin/";
	U8 outFile_Name[50] = "./output/";
	U8 nsihFile_Name[50]= "./nsih/";

	CRC uCrc;

	/* Pre-Fix  Default (Name & Value) */
	cpu_name	= "NXP4330";
	option_name	= "2ndboot";
	boot_mode	= ".";

	nsih_name	= "NSIH.txt";
	input_name	= "pyrope_2ndboot_spi.bin";
	output_name	= "2ndboot_spi.bin";

	OutputSize	= 16*1024;

//	loadaddr	= 0x40100000;
//	launchaddr	= 0x40100000;

    if( argc == CTRUE )
    {
        usage();
        return CTRUE;
    }

	while( -1 !=(param_opt = getopt( argc, argv, "h:c:t:n:i:o:l:e:s:b:a:u:v:")))
	{
      	switch(param_opt)
      	{
	      	case 'h':
	      		usage();
	      		return CTRUE;
	      	case 'c':
	      		cpu_name	= strdup(optarg);
	      		break;
			case 't':
				option_name = strdup(optarg);
				break;
	      	case 'n':
	        	nsih_name 	= strdup(optarg);
	        	break;
	      	case 'i':
	      		input_name 	= strdup(optarg);
	        	break;
	      	case 'o':
	      		output_name = strdup(optarg);
	        	break;
			case 'l':
				loadaddr 	= HexAtoInt(optarg);
				break;
	      	case 'e':
	      		launchaddr	= HexAtoInt(optarg);
	        	break;
            // Not Required Option
	        case 's':
				OutputSize 	= atoi(optarg);
				printf("The Size you choose is Binary %dKB.\r\n", OutputSize );
	      	case 'u':
	      		device_portnum = HexAtoInt(optarg);
	        	break;
			case 'b':
				boot_mode 	= strdup(optarg);
				break;
			case 'a':
				device_addr	= HexAtoInt(optarg);
				break;
            // Debug Option
			case 'v':
				view_option	= strdup(optarg);
				break;
			// Unknown Option
			default:
	        	printf("unknown option_num parameter\r\n");
	        	break;
    	}
    }

    if( nsih_name == NULL )
    {
		nsih_name = "NSIH.txt";
		printf("Did not enter the NSIH files.\r\n");
		printf("This has been used as the default NSIH file.\r\n");
    }

	if( input_name == NULL )
	{
		input_name = "pyrope_secondboot.bin";
		printf("Did not enter the Binary files.\r\n");
		printf("This has been used as the default pyrope_secondboot.bin.\r\n");
	}
	//--------------------------------------------------------------------------
#if 0
	strncpy( output_name, cpu_name, 7 );
	strncat( output_name, "_", 1 );
	strncat( output_name, boot_mode, 3 );
	strncat( output_name, "_", 1 );
	strncat( output_name, option_name, 7 );

    InFile_fd	= fopen(input_name , "r");
    OutFile_fd	= fopen(output_name, "w");
//#else
	// Base Folder /bin, /output, /nsih + user file name
	strcat(inFile_Name  , input_name );
	strcat(outFile_Name , output_name);
	strcat(nsihFile_Name, nsih_name  );
	InFile_fd	= fopen(inFile_Name , "r");
    OutFile_fd	= fopen(outFile_Name, "wb");
#endif

    InFile_fd	= fopen(input_name , "r");
    OutFile_fd	= fopen(output_name, "w");

    if((!InFile_fd))
    {
    	printf("Input File open failed!! check filename!!\n");
    	ret = FALSE;
    	goto ERR_END;
    }

    if((!OutFile_fd))
    {
    	printf("Output File open failed!! check filename!!\n");
    	ret = FALSE;
    	goto ERR_END;
    }

	/* input file size check */
	fseek( InFile_fd, 0, SEEK_END );
	InputSize = ftell ( InFile_fd );
	fseek( InFile_fd, 0, SEEK_SET );

	/* Ourput Size Calcurate */
	//--------------------------------------------------------------------------
	if( 0 == strcmp( option_name, "2ndboot" ) )
	{
		if( (0 == strcmp( cpu_name, "NXP4330" )) )
		{
			OutputSize = NXP4330_SRAM_SIZE;
		}
		else if( 0 == strcmp( cpu_name, "NXP5430" ))
		{
			OutputSize = InputSize + NSIHSIZE;
			if( OutputSize >= (NXP5430_SRAM_SIZE - NSIHSIZE) )
			{
				printf("The image is Generated exceeds 64KB. The Creation failed! \r\n");
				printf("Calcurate image Size : %d \r\n", OutputSize );
				printf("Return Error End!!\r\n");
				goto ERR_END;
			}
		}
		else if(0 == strcmp( cpu_name, "S5P4418" ))
		{
			OutputSize = InputSize + NSIHSIZE;
			if( OutputSize >= (S5P4418_SRAM_SIZE - NSIHSIZE) )
			{
				printf("The image is Generated exceeds 28KB. The Creation failed! \r\n");
				printf("Calcurate image Size : %d \r\n", OutputSize );
				printf("Return Error End!!\r\n");
				goto ERR_END;
			}
		}
		else
			printf("CPU name is unknown. \r\n");

	}
	else if( 0 == strcmp( option_name, "3rdboot" ) )
		OutputSize = InputSize + NSIHSIZE;
	//--------------------------------------------------------------------------

	/* File Descript Check & Maximum Size */
	//--------------------------------------------------------------------------
	if( OutputSize == 0 )
	{
		printf("Did not enter the Filesize files.\r\n");
		goto ERR_END;
	}

	if( (OutputSize % 512) != 0 )
		MallocSize = ((OutputSize / BLOCKSIZE) + 1) * BLOCKSIZE;
	else
		MallocSize = OutputSize;
	//--------------------------------------------------------------------------
    Out_Buffer 	= (U8*)malloc( MallocSize );
	memset(Out_Buffer , 0xFF, MallocSize);		// set 0 to rest area

	/* Read to ProcessNSIH */
	//--------------------------------------------------------------------------
 	if( NSIHSIZE != ProcessNSIH( nsih_name, Out_Buffer ) )
	// translate text file to 512B binary
	/* | 512 NSIH | (16KB - 512 - 16) second boot | 4 CRC | 12 dummy |	==> total size is 16KB */
	{
		printf("ERROR : Failed to process NSIH(%s).\n", argv[3] );
		ret = FALSE;
		goto ERR_END;
	}
	fread(Out_Buffer + NSIHSIZE, 1, InputSize, InFile_fd);
	//--------------------------------------------------------------------------

#if 0
	NX_SHELLU_HexDump( (U32*)Out_Buffer + (NSIHSIZE/4), InputSize, 8 );
#endif

	/*  CRC Check for SPI, SD, UART, ETC */
	//---------------------------------------------------------------------------------------
#if SECURE_BOOT
	// Secure Boot <-- Decript Issue ( 16Byte Convert )
	if( ((InputSize % 16) != 0) )
		InputSize = ((InputSize / 16) * 16);
#endif
	//---------------------------------------------------------------------------------------

	/*  2ndboot & 3rdboot Header Modify Information. */
	//---------------------------------------------------------------------------------------
	//if( (0 == strcmp( option_name, "3rdboot" )) || (0 == strcmp( option_name, "2ndboot" )) )
	{
		pBootInfo = (struct NX_SecondBootInfo*)Out_Buffer;

		// Device Dependency
		//------------------------------------------------------------------------//
		// Not Required Option (NSIH Default)
		//-------------------------------------------------------------------------
		if( 0 == strcmp( boot_mode, "SD" ) )
			pBootInfo->DEVICEADDR         = 0x8000;
		else if( 0 == strcmp( boot_mode, "SPI" ) )
		{
			pBootInfo->DEVICEADDR		  = 0x10000;
			pBootInfo->DBI.SPIBI.AddrStep = 3; 			            // (0:8Bit, 1:16Bit, 3:24Bit)
		}
        if( device_portnum != 0 )
        {
    		pBootInfo->DBI.SDMMCBI.PortNumber = (U8)device_portnum;     // Each Device Port
        }
		if( device_addr != 0 )
		{
            pBootInfo->DEVICEADDR = device_addr;                        // Each Device Address
		}
		//------------------------------------------------------------------------
		// Output Size
		if( (0 != strcmp( cpu_name, "NXP4330" )) )
			pBootInfo->LOADSIZE			= InputSize;
		else
			pBootInfo->LOADSIZE			= InputSize;  //OutputSize;

		//------------------------------------------------------------------------//
        // NSIH Default ( Load Address, Launch Address )
        if( loadaddr != 0 )
    		pBootInfo->LOADADDR			= loadaddr;
        if( launchaddr != 0 )
	    	pBootInfo->LAUNCHADDR		= launchaddr;

		pBootInfo->SIGNATURE		= HEADER_ID;		// Signature (NSIH)
		//------------------------------------------------------------------------//
        pBootInfo->DBI.SDMMCBI.CRC32 = __calc_crc((void*)(Out_Buffer + NSIHSIZE), (InputSize) );
		/* CRC - 2ndboot (16KB-16  ) */
		// NXP4330 && Secondboot
		if( (InputSize + NSIHSIZE) <= (NXP4330_SRAM_SIZE-16) )
		{
			uCrc.iCrc = __calc_crc((void*)(Out_Buffer), (OutputSize-16) );
			for(i = 0; i < CRCSIZE; i++)
			{
				Out_Buffer[OutputSize-16+i] = uCrc.chCrc[i];
				//printf("(BASEADDR + 0x%X) - CRC[%d]: %X(%X) \r\n",
				//(OutputSize-16+i), i, Out_Buffer[OutputSize-16+i], uCrc.chCrc[i]);
			}
		}
		//------------------------------------------------------------------------//

	#ifdef BOOT_DEBUG
		NX_DEBUG_MSG("LOADSIZE 	: %8X \r\n", pBootInfo->LOADSIZE   );
		NX_DEBUG_MSG("LOADADDR 	: %8X \r\n", pBootInfo->LOADADDR   );
		NX_DEBUG_MSG("LAUNCHADDR 	: %8X \r\n", pBootInfo->LAUNCHADDR );
		NX_DEBUG_MSG("SIGNATURE 	: %8X \r\n", pBootInfo->SIGNATURE  );
		NX_DEBUG_MSG("CRC32 		: %8X \r\n", pBootInfo->DBI.SDMMCBI.CRC32 );
	#endif
	}
	//---------------------------------------------------------------------------------------

	fwrite(Out_Buffer, 1, OutputSize, OutFile_fd);				// Write image
	print_bingen_info();										// Bingen Debug Mesage (Infomation desk)

#if 0
	if( (0 == strcmp( view_option, "viewer" )) ){
		//print_hexdump( (U32*)Out_Buffer + (NSIHSIZE/4), InputSize );
		NX_SHELLU_HexDump( (U32*)Out_Buffer, OutputSize, 8 ); 	}
#endif

	fclose(InFile_fd);
	fclose(OutFile_fd);

	free(Out_Buffer);

NOMAL_END:
    printf("%s: return %d\n", __func__, ret);
	return 0;

ERR_END:
	fclose(InFile_fd);
	fclose(OutFile_fd);

	free(Out_Buffer);

	return -1;
}

void print_hexdump( U32* pdwBuffer, U32 Size )
{
	register int i = 0;

	for(i = 0; i < Size/4; i++)
	{
		printf("%8X  ", pdwBuffer[i] );
		if( ((i+1) % 8) == 0 )
			printf("\r\n");
	}
	printf("\r\n");

}
#if 0
//------------------------------------------------------------------------------
// NX_SHELLU_HexDump
//------------------------------------------------------------------------------
CBOOL	NX_SHELLU_HexDump	( U32 SrcAddr, U32 PrintDataSize, U32 PrintDataWidth )
{
    U32 i, j;
    U32 dwMemAddr;
    U32 Buffer[16/4];
    U8 	*pBuffer8;
    U16 *pBuffer16;
    U32 *pBuffer32;

    U32 Cnt = 0;

    pBuffer8 	= (U8  *)Buffer;
    pBuffer16 	= (U16 *)Buffer;
    pBuffer32 	= (U32 *)Buffer;

    // checke data width size
         if( PrintDataWidth == 16 ) dwMemAddr = (U32)(SrcAddr & ~0x1);
    else if( PrintDataWidth == 32 ) dwMemAddr = (U32)(SrcAddr & ~0x3);
    else 							dwMemAddr = (U32)(SrcAddr & ~0x0);

    // Print data
	for( i=0 ; i < (PrintDataSize/16) ; i++ )
	{
		//printf("0x%08X", dwMemAddr );
		//printf( " : " );

        // 16bit expressiont case
		if( PrintDataWidth == 16 )
		{
			pBuffer16[0] = *(((U16 *)dwMemAddr)+0);
			pBuffer16[1] = *(((U16 *)dwMemAddr)+1);
			pBuffer16[2] = *(((U16 *)dwMemAddr)+2);
			pBuffer16[3] = *(((U16 *)dwMemAddr)+3);
			pBuffer16[4] = *(((U16 *)dwMemAddr)+4);
			pBuffer16[5] = *(((U16 *)dwMemAddr)+5);
			pBuffer16[6] = *(((U16 *)dwMemAddr)+6);
			pBuffer16[7] = *(((U16 *)dwMemAddr)+7);

        	printf("%04X  "   , pBuffer16[0] );
        	printf("%04X  "   , pBuffer16[1] );
        	printf("%04X  "   , pBuffer16[2] );
        	printf("%04X  :  ", pBuffer16[3] );
        	printf("%04X  "   , pBuffer16[4] );
        	printf("%04X  "   , pBuffer16[5] );
        	printf("%04X  "   , pBuffer16[6] );
        	printf("%04X "    , pBuffer16[7] );
        }
	    // 32bit expressiont case
		else if( PrintDataWidth == 32 )
		{
			pBuffer32[0] = *(((U32 *)dwMemAddr)+0);
			pBuffer32[1] = *(((U32 *)dwMemAddr)+1);
			pBuffer32[2] = *(((U32 *)dwMemAddr)+2);
			pBuffer32[3] = *(((U32 *)dwMemAddr)+3);

        	printf("%08X    "     , pBuffer32[0] );
        	printf("%08X    :    ", pBuffer32[1] );
        	printf("%08X    "     , pBuffer32[2] );
        	printf("%08X "        , pBuffer32[3] );
        }
        // 8bit expressiont case
		else //if( PrintDataWidth == 8 )
		{
			pBuffer8[ 0] = *(((U8 *)dwMemAddr)+ 0);
			pBuffer8[ 1] = *(((U8 *)dwMemAddr)+ 1);
			pBuffer8[ 2] = *(((U8 *)dwMemAddr)+ 2);
			pBuffer8[ 3] = *(((U8 *)dwMemAddr)+ 3);
			pBuffer8[ 4] = *(((U8 *)dwMemAddr)+ 4);
			pBuffer8[ 5] = *(((U8 *)dwMemAddr)+ 5);
			pBuffer8[ 6] = *(((U8 *)dwMemAddr)+ 6);
			pBuffer8[ 7] = *(((U8 *)dwMemAddr)+ 7);
			pBuffer8[ 8] = *(((U8 *)dwMemAddr)+ 8);
			pBuffer8[ 9] = *(((U8 *)dwMemAddr)+ 9);
			pBuffer8[10] = *(((U8 *)dwMemAddr)+10);
			pBuffer8[11] = *(((U8 *)dwMemAddr)+11);
			pBuffer8[12] = *(((U8 *)dwMemAddr)+12);
			pBuffer8[13] = *(((U8 *)dwMemAddr)+13);
			pBuffer8[14] = *(((U8 *)dwMemAddr)+14);
			pBuffer8[15] = *(((U8 *)dwMemAddr)+15);

        	printf("%02X "  , pBuffer8[ 0] );
        	printf("%02X "  , pBuffer8[ 1] );
        	printf("%02X "  , pBuffer8[ 2] );
        	printf("%02X "  , pBuffer8[ 3] );
        	printf("%02X "  , pBuffer8[ 4] );
        	printf("%02X "  , pBuffer8[ 5] );
        	printf("%02X "  , pBuffer8[ 6] );
        	printf("%02X ", pBuffer8[ 7] );
        	printf("%02X "  , pBuffer8[ 8] );
        	printf("%02X "  , pBuffer8[ 9] );
        	printf("%02X "  , pBuffer8[10] );
        	printf("%02X "  , pBuffer8[11] );
        	printf("%02X "  , pBuffer8[12] );
        	printf("%02X "  , pBuffer8[13] );
        	printf("%02X "  , pBuffer8[14] );
        	printf("%02X "  , pBuffer8[15] );

        	Cnt+=16;
		}

    	// Print 16 byte character information
    	/*
    	for( j=0 ; j<16 ; j++ )
    	{
	    	if( pBuffer8[j] >= 0x21 &&
	    	    pBuffer8[j] <= 0x7e &&
	    	    pBuffer8[j] != '%' )
	    	    printf( "%c", pBuffer8[j] );
	      	else
	      	    printf( "." );
        }
		*/
	    printf("\r\n");

		dwMemAddr += 16;
	}
	printf("Cnt : %d \r\n", Cnt );

	return CTRUE;
}
#endif

static inline U32 iget_fcs(U32 fcs, U32 data)
{
	register int i;
	fcs ^= data;
	for(i=0; i<32; i++)
	{
		if(fcs & 0x01)
			fcs ^= POLY;
		fcs >>= 1;
	}
	return fcs;
}

static unsigned int globalk = 0;
unsigned int get_fcs(unsigned int fcs, unsigned char data)
{
	register int i;
	fcs ^= (unsigned int)data;
	for(i = 0; i < 8; i++)
	{
	   if(fcs & 0x01)
	   		fcs ^= POLY;
	   fcs >>= 1;
	}
#if 0
	if( fcs == 0x01729E99 )
		printf("[%d]St FCS Match : %8X\r\n", globalk, fcs );
	else
		printf("[%d]St FCS Not Match : %8X\r\n", globalk, fcs );
	globalk += 1;
//#else
	if( ((globalk++) % 8) == 0 )
		printf("\r\n[%d]: ", globalk);
	printf("%8X ", fcs );
#endif
	return fcs;
}


static inline unsigned int __icalc_crc (void *addr, int len)
{
	U32 *c = (U32*)addr;
	U32 crc = 0, chkcnt = ((len+3)/4);
	U32 i;

	for (i = 0; chkcnt > i; i += CHKSTRIDE, c += CHKSTRIDE) {
		crc = iget_fcs(crc, *c);
	}

	return crc;
}

static inline unsigned int __calc_crc (void *addr, int len)
{
	U8 *c = (U8*)addr;
	U32 crc = 0;
	int i;
	for (i = 0; len > i; i++)
	{
		crc = get_fcs(crc, c[i]);
	}
	return crc;
}


// Nexell System Information Header
//------------------------------------------------------------------------------
int ProcessNSIH( const char *pfilename, U8 *pOutData )
{
	FILE	*fp;
	char ch;
	int writesize, skipline, line, bytesize, i;
	U32 writeval;

	fp = fopen( pfilename, "rb" );
	if( !fp )
	{
		printf( "ProcessNSIH : ERROR - Failed to open %s file.\n", pfilename );
		return 0;
	}

	bytesize = 0;
	writeval = 0;
	writesize = 0;
	skipline = 0;
	line = 0;

	while( 0 == feof( fp ) )
	{
		ch = fgetc( fp );

		if( skipline == 0 )
		{
			if( ch >= '0' && ch <= '9' )
			{
				writeval = writeval * 16 + ch - '0';
				writesize += 4;
			}
			else if( ch >= 'a' && ch <= 'f' )
			{
				writeval = writeval * 16 + ch - 'a' + 10;
				writesize += 4;
			}
			else if( ch >= 'A' && ch <= 'F' )
			{
				writeval = writeval * 16 + ch - 'A' + 10;
				writesize += 4;
			}
			else
			{
				if( writesize == 8 || writesize == 16 || writesize == 32 )
				{
					for( i=0 ; i<writesize/8 ; i++ )
					{
						pOutData[bytesize++] = (U8)(writeval & 0xFF);
						writeval >>= 8;
					}
				}
				else
				{
					if( writesize != 0 )
						printf("ProcessNSIH : Error at %d line.\n", line+1 );
				}
				writesize = 0;
				skipline = 1;
			}
		}

		if( ch == '\n' )
		{
			line++;
			skipline = 0;
			writeval = 0;
		}
	}

	NX_DEBUG_MSG( "ProcessNSIH : %d line processed.\n", line+1 );
	NX_DEBUG_MSG( "ProcessNSIH : %d bytes generated.\n", bytesize );

	fclose( fp );

	return bytesize;
}

U32 HexAtoInt( const char *string )
{
	char 	ch;
	U32 	result = 0;

	while( (ch = *string++) != 0 )
	{
		if( ch >= '0' && ch <= '9' )
		{
			result = result * 16 + (ch - '0');
		}
		else if( ch >= 'a' && ch <= 'f' )
		{
			result = result * 16 + (ch - 'a') + 10;
		}
		else if( ch >= 'A' && ch <= 'F' )
		{
			result = result * 16 + (ch - 'A') + 10;
		}
	}

	return result;
}


static void usage(void)
{
	printf("--------------------------------------------------------------------------\n");
	printf(" Release  Version         : Ver.%03d                                      \n", BOOT_BINGEN_VER );
	printf(" Author & Version Manager : Deoks (S/W 1Team)                             \n");
	printf("--------------------------------------------------------------------------\n");

	printf(" Usage : This will tell you How to Use Help.					          \n");
	printf("--------------------------------------------------------------------------\n" );
	printf("   -h [HELP]                     : show usage                             \n");
	printf("   -c [NXP4330/NXP5430/S5P4418]  : What is the cpu?	   			          \n");
	printf("   -t [2nboot/3rdboot]           : What is the Boot?	   		          \n");
	printf("   	->[2ndboot]              : NXP4330 is fixed to 16KB 2ndboot size.     \n");
	printf("   	->[3rdboot]              : 3rdboot size is flexible.	   	    	  \n");
	printf("   -n [file name]                : [NSIH] file name	   					  \n");
	printf("   -i [file name]                : [INPUT]file name      				  \n");
	printf("   -o [file name]                : [OUTPUT]file name	   				  \n");
	printf("   -l [load address]             : Will address run the following code?	  \n");
	printf("   	-> Default Load	  Address : 0x40100000   					          \n");
	printf("   -e [launch address]           : Will address run the following code?	  \n");
	printf("   	-> Default Launch Address : 0x40100000   					          \n");
	printf("--------------------------------------------------------------------------\n");
    printf("   This Option is Not Required. (if you do not use NSIH.txt Default.)     \n");
    printf("--------------------------------------------------------------------------\n");
	printf("   -b [boot mode]                : What is the Boot Mode? (Prefix Set) 	  \n");
	printf("   	-> SPI (Device Addr : 0x10000, Address Step : 3 (24Bit))              \n");
	printf("   	-> SD  (Device Addr : 0x8000)       					              \n");
	printf("   -a [device address]           : What is the Device Address?   		  \n");
	printf("   -u [device port]              : device channel                         \n");
    printf("--------------------------------------------------------------------------\n");
    printf(" Remark & Reference Message                                               \n");
    printf(" The current version has not been applied to the NAND BINGEN version..    \n");
    printf("--------------------------------------------------------------------------\n");
	printf("\n");
	printf(" Usage: How to use the program? 			                              \n");
	printf(" Ubuntu  > How to use?                                                    \n");
	printf("  #>./BOOT_BINGEN -h 0 or ./BOOT_BINGEN \n");
	printf("  #>./BOOT_BINGEN -c NXP4330 -t 2ndboot -b SPI -n NXP4330_NSIH_V05_spi_800.txt -i pyrope_2ndboot_spi.bin -o 2ndboot_spi.bin \n" );
	printf("  #>./BOOT_BINGEN -c NXP4330 -t 2ndboot -b SD  -n NXP4330_NSIH_V05_sd_800.txt  -i pyrope_2ndboot_sdmmc.bin -o 2ndboot_sdmmc.bin \n" );
	printf("  #>./BOOT_BINGEN -c NXP4330 -t 3rdboot -b SPI -n NXP4330_NSIH_V05_spi_800.txt -i u-boot.bin -o 3rdboot_spi.bin -l 40100000 -e 40100000 \n" );
	printf("  #>./BOOT_BINGEN -c NXP4330 -t 3rdboot -b SD  -n NXP4330_NSIH_V05_sd_800.txt  -i u-boot.bin -o 3rdboot_sdmmc.bin -l 40100000 -e 40100000 \n" );
    printf("\n");
	printf(" Windows > How to use?                                                    \n");
    printf("  #>BOOT_BINGEN.exe -h 0 or BOOT_BINGEN.exe \n");
	printf("  #>BOOT_BINGEN.exe -c NXP4330 -t 2ndboot -b SPI -n NXP4330_NSIH_V05_spi_800.txt -i pyrope_2ndboot_spi.bin -o 2ndboot_spi.bin \n" );
	printf("  #>BOOT_BINGEN.exe -c NXP4330 -t 2ndboot -b SD  -n NXP4330_NSIH_V05_sd_800.txt  -i pyrope_2ndboot_sdmmc.bin -o 2ndboot_sdmmc.bin \n" );
	printf("  #>BOOT_BINGEN.exe -c NXP4330 -t 3rdboot -b SPI -n NXP4330_NSIH_V05_spi_800.txt -i u-boot.bin -o 3rdboot_spi.bin -l 40100000 -e 40100000 \n" );
	printf("  #>BOOT_BINGEN.exe -c NXP4330 -t 3rdboot -b SD  -n NXP4330_NSIH_V05_sd_800.txt  -i u-boot.bin -o 3rdboot_sdmmc.bin -l 40100000 -e 40100000 \n" );

	printf("\n");
}

static void print_bingen_info( void )
{
	printf( "----------------------------------------------------\n" );
	printf( " %s %s Binary file Information.\n", cpu_name, option_name );
	printf( " %s Binary Boot Mode 	 		\n", boot_mode );
	printf( " NSIH   Text   File : %s		\n", nsih_name   ? (char*)nsih_name   : "NULL" );
	printf( " Input  Binary File : %s		\n", input_name  ? (char*)input_name  : "NULL" );
	printf( " Output Binary File : %s		\n", output_name ? (char*)output_name : "NULL" );
	printf( " Input  Binary Size : %d Byte (%dKB)	\n", InputSize   ? InputSize   : 0, (InputSize+1024-1)/1024 );
	printf( " Output Binary Size : %d Byte (%dKB)\n", OutputSize  ? OutputSize  : 0, (OutputSize+1024-1)/1024 );
	printf( "----------------------------------------------------\n" );
	printf( " NSIH(Header) Information.  \n" );
	printf( " %s Binary Boot Mode 	\n", boot_mode );

	if( (0 == strcmp( boot_mode, "SPI" ))
	|| (pBootInfo->DBI.SPIBI._Reserved1 == 0x01000000 ) )
		printf( "  -> Addr  Step : %8Xh \r\n", pBootInfo->DBI.SPIBI.AddrStep );
	else
		printf( "  -> DevicePort : %8Xh \r\n", pBootInfo->DBI.SDMMCBI.PortNumber );

	printf( "  -> DeviceAddr : %8Xh \r\n", pBootInfo->DEVICEADDR );
	printf( "  -> LoadSize	: %8Xh \r\n", pBootInfo->LOADSIZE   );
	printf( "  -> LoadAddr	: %8Xh \r\n", pBootInfo->LOADADDR   );
	printf( "  -> LauchAddr	: %8Xh \r\n", pBootInfo->LAUNCHADDR );
	printf( "  -> SigNature	: %8Xh \r\n", pBootInfo->SIGNATURE );
	printf( "  -> CRC32	: %8Xh \r\n", pBootInfo->DBI.SDMMCBI.CRC32 );
#if SWAP_ENABLE
	printf( "  -> SWAP	:  %8s \r\n", SwapEnb );
#endif
#if 0
	printf("----------------------------------------------------\n");
	printf(" Release  Version         : Ver.%03d                \n", BOOT_BINGEN_VER );
	printf(" Author & Version Manager : Deoks (S/W 1Team)       \n");
	printf("----------------------------------------------------\n");
#endif
}

