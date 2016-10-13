//------------------------------------------------------------------------------
//
//	Copyright (C) 2016 Nexell Co. All Rights Reserved
//	Nexell Co. Proprietary & Confidential
//
//	NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND	WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//	Module		:
//	File		:
//	Description	:
//	Author		:
//	Export		:
//	History		:
//
//------------------------------------------------------------------------------

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>

#include <CNX_I2C.h>

#define MAX_COUNT	100000

#define I2C_PORT	0
#define I2C_SLAVE	0x09

//	Clock Setting
//
//	1M --> FAIL
//	900K, 0.28ms
//	400K, 0.36ms
//	100K, 0.79ms
//

//
// Mem: 346040K used, 483844K free, 0K shrd, 3316K buff, 169716K cached
// CPU:  1.2% usr 13.5% sys  0.0% nic 84.5% idle  0.0% io  0.6% irq  0.0% sirq
// Load average: 1.68 0.87 0.41 3/423 1033
//   PID  PPID USER     STAT   VSZ %VSZ CPU %CPU COMMAND
//    27     2 0        RW       0  0.0   0  9.1 [irq/15-nxp-i2c]
//  1030  1018 0        D    10000  1.2   1  4.8 /system/bin/nx_i2c
//


static uint16_t initData[][2] = {
	0x0005, 0x044C,
	0x0006, 0x0465,
	0x0007, 0x03C0,
	0x0008, 0x0438,
	0x0009, 0x0100,
	0x000A, 0x0168,
	0x000B, 0x0080,
	0x000C, 0x003C,
	0x000F, 0x0415,
	0x0010, 0x0497,
	0x0011, 0x044C,
	0x0012, 0x0486,
	0x001C, 0x0014,
	0x0017, 0x0015,
	0x001D, 0x001E,
	0x0121, 0xB400,
	0x0122, 0x0020,
	0x0123, 0x03C0,
};

static int32_t InitRandomValue( void )
{
	srand( time(NULL) + getpid() );
	return 0;
}

static int32_t GetRandomValue( int32_t iStartNum, int32_t iEndNum )
{
	int32_t iRandomTime = -1;

	if( iStartNum >= iEndNum ) {
		return iRandomTime;
	}
	
	iRandomTime = rand() % (iEndNum - iStartNum + 1) + iStartNum;
	return iRandomTime;
}

static uint64_t GetSystemTick( void )
{
	uint64_t ret;
	struct timeval	tv;
	struct timezone	zv;

	gettimeofday( &tv, &zv );

	ret = ((uint64_t)tv.tv_sec)*1000 + tv.tv_usec/1000;
	return ret;
}

int32_t main( void )
{
	uint64_t iCount = 0;
	
	int32_t iRandom = 0;
	uint16_t iReadData = 0;

	uint64_t iCurTime = 0, iGapTime = 0, iTotalTime = 0;

	InitRandomValue();

	CNX_I2C *pI2CCtrl = new CNX_I2C( I2C_PORT );
	if( 0 > pI2CCtrl->Open() )
	{
		printf("Fail, Open().\n");
		delete pI2CCtrl;
		return -1;
	}

	while( ++iCount < MAX_COUNT )
	{
		iRandom = GetRandomValue( 0, sizeof(initData) / sizeof(initData[0]) - 1 );

		iCurTime = GetSystemTick();
		iReadData = pI2CCtrl->Read( I2C_SLAVE, initData[iRandom][0] );
		iGapTime = GetSystemTick() - iCurTime;

		if( iReadData != initData[iRandom][1] )
		{
			printf("[%lld] addr(0x%04x) expect(0x%04x) <--> real(0x%04x)\n", 
				iCount,
				initData[iRandom][0],
				initData[iRandom][1],
				iReadData
			);
		}

		iTotalTime += iGapTime;
	}

	printf("--> I2C Read Average Time( %.02lf ms )\n", (double)iTotalTime / MAX_COUNT);

	if( pI2CCtrl )
	{
		pI2CCtrl->Close();
		delete pI2CCtrl;
	}

	return 0;
}