#include <stdio.h>
#include <stdlib.h>

#include "CEepromDataParser.h"

#ifdef WIN32
#define FILENAME	".\\data\\CinemaLED_ModeImage.bin"
#define OUTPUTPATH	".\\output\\"
#else
#define FILENAME	"./data/CinemaLED_ModeImage.bin"
#define OUTPUTPATH	"./output"
#endif

extern "C" void DumpTconInfo( TCON_EEPROM_INFO *pTconInfo, const char *outputPath );

int main( int argc, char *argv[] )
{
	TCON_EEPROM_INFO *pTconInfo = NULL;
	CEepromDataParser *pParser = new CEepromDataParser();
	for( int i=0 ; i<1 ; i++ )
	{
		//	File interface
		if( 1 )
		{
			if( 0 != pParser->Init( FILENAME ) )
			{
				printf("Init failed !!!\n");
				exit(-1);
			}
			if( 0 != pParser->Parse( &pTconInfo ) )
			{
				printf("Parsing failed!!!\n");
				exit(-1);
			}

			pParser->WriteTconInfo( pTconInfo, OUTPUTPATH );
			pParser->Deinit();
		}
		//	Buffer interface
		if( 0 )
		{
			uint8_t *pInBuf;
			long fileSize;
			FILE *pFd = fopen(FILENAME,"rb");
			if(NULL == pFd)
			{
				printf("Cannot open file!!!!\n");
				return -1;
			}

			//	File file size
			fseek(pFd,0,SEEK_END);
			fileSize = ftell(pFd);
			fseek(pFd,0,SEEK_SET);

			//	Allocate Input Buffer
			pInBuf = (uint8_t*)malloc(fileSize);
			//	Read Buffer from file
			fileSize = fread(pInBuf, 1, fileSize, pFd);
			fclose(pFd);

			pParser->Init(pInBuf, (int32_t)fileSize);
			pParser->Parse( &pTconInfo );
			pParser->Deinit();
		}
		if( i%1000 == 0 )
			printf("Loop %d\n\r", i);
	}
	printf("Done\n");
	return 0;
}
