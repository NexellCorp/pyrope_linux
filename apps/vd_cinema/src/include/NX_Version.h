//------------------------------------------------------------------------------
//
//	Copyright (C) 2017 Nexell Co. All Rights Reserved
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

#ifndef __NX_VERSION_H__
#define __NX_VERSION_H__

#include <stdint.h>

//
//	Version Information
//
#define NX_VERSION_NAP			"Version 1.3.0"
#define NX_VERSION_SAP			"Version 1.0.0"
#define NX_VERSION_IPC_SERVER	"v1.00.00"
#define NX_VERSION_IPC_CLIENT	"v1.00.00"
#define NX_VERSION_TMS_SERVER	"v1.00.00"
#define NX_VERSION_TMS_CLIENT	"v1.00.00"


//
//	__DATE__ ( May 16 2018 ) to Integer ( 20180516 )
//	__TIME__ ( 16:11:21 ) to Integer ( 161121 )
//
#define IS_JAN()	(__DATE__[0] == 'J' && __DATE__[1] == 'a' && __DATE__[2] == 'n')
#define IS_FEB()	(__DATE__[0] == 'F' && __DATE__[1] == 'e' && __DATE__[2] == 'b')
#define IS_MAR()	(__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'r')
#define IS_APR()	(__DATE__[0] == 'A' && __DATE__[1] == 'p' && __DATE__[2] == 'r')
#define IS_MAY()	(__DATE__[0] == 'M' && __DATE__[1] == 'a' && __DATE__[2] == 'y')
#define IS_JUN()	(__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'n')
#define IS_JUL()	(__DATE__[0] == 'J' && __DATE__[1] == 'u' && __DATE__[2] == 'l')
#define IS_AUG()	(__DATE__[0] == 'A' && __DATE__[1] == 'u' && __DATE__[2] == 'g')
#define IS_SEP()	(__DATE__[0] == 'S' && __DATE__[1] == 'e' && __DATE__[2] == 'p')
#define IS_OCT()	(__DATE__[0] == 'O' && __DATE__[1] == 'c' && __DATE__[2] == 't')
#define IS_NOV()	(__DATE__[0] == 'N' && __DATE__[1] == 'o' && __DATE__[2] == 'v')
#define IS_DEC()	(__DATE__[0] == 'D' && __DATE__[1] == 'e' && __DATE__[2] == 'c')

#define GET_MONTH()	(IS_JAN() ?  1 : \
					 IS_FEB() ?  2 : \
					 IS_MAR() ?  3 : \
					 IS_APR() ?  4 : \
					 IS_MAY() ?  5 : \
					 IS_JUN() ?  6 : \
					 IS_JUL() ?  7 : \
					 IS_AUG() ?  8 : \
					 IS_SEP() ?  9 : \
					 IS_OCT() ? 10 : \
					 IS_NOV() ? 11 : \
					 IS_DEC() ? 12 : 0 )

#define NX_DATE()	(uint64_t)(	(__DATE__[7]  - '0')* 10000000	+	\
								(__DATE__[8]  - '0')* 1000000	+	\
								(__DATE__[9]  - '0')* 100000	+	\
								(__DATE__[10] - '0')* 10000		+	\
								GET_MONTH()         * 100		+	\
								(__DATE__[4]  - '0')* 10		+	\
								(__DATE__[5]  - '0')* 1 )

#define NX_TIME()	(uint64_t)( (__TIME__[0]  - '0')* 100000	+	\
								(__TIME__[1]  - '0')* 10000		+	\
								(__TIME__[3]  - '0')* 1000		+	\
								(__TIME__[4]  - '0')* 100		+	\
								(__TIME__[6]  - '0')* 10		+	\
								(__TIME__[7]  - '0')* 1 )

#endif	// __NX_VERSION_H__
