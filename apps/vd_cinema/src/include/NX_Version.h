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
#define NX_VERSION_NAP			"Version 1.1.0"
#define NX_VERSION_SAP			"Version 1.0.0"
#define NX_VERSION_IPC_SERVER	"IPC_SER_v1.00.00"
#define NX_VERSION_IPC_CLIENT	"IPC_CLI_v1.00.00"


//
//	__DATE__ ( May 16 2018 ) to Integer ( 20180516 )
//	__TIME__ ( 16:11:21 ) to Integer ( 161121 )
//
#define IS_JAN(DATE)	(DATE[0] == 'J' && DATE[1] == 'a' && DATE[2] == 'n')
#define IS_FEB(DATE)	(DATE[0] == 'F' && DATE[1] == 'e' && DATE[2] == 'b')
#define IS_MAR(DATE)	(DATE[0] == 'M' && DATE[1] == 'a' && DATE[2] == 'r')
#define IS_APR(DATE)	(DATE[0] == 'A' && DATE[1] == 'p' && DATE[2] == 'r')
#define IS_MAY(DATE)	(DATE[0] == 'M' && DATE[1] == 'a' && DATE[2] == 'y')
#define IS_JUN(DATE)	(DATE[0] == 'J' && DATE[1] == 'u' && DATE[2] == 'n')
#define IS_JUL(DATE)	(DATE[0] == 'J' && DATE[1] == 'u' && DATE[2] == 'l')
#define IS_AUG(DATE)	(DATE[0] == 'A' && DATE[1] == 'u' && DATE[2] == 'g')
#define IS_SEP(DATE)	(DATE[0] == 'S' && DATE[1] == 'e' && DATE[2] == 'p')
#define IS_OCT(DATE)	(DATE[0] == 'O' && DATE[1] == 'c' && DATE[2] == 't')
#define IS_NOV(DATE)	(DATE[0] == 'N' && DATE[1] == 'o' && DATE[2] == 'v')
#define IS_DEC(DATE)	(DATE[0] == 'D' && DATE[1] == 'e' && DATE[2] == 'c')

#define GET_MONTH(DATE)	(IS_JAN(DATE) ?  1 : \
        				 IS_FEB(DATE) ?  2 : \
        				 IS_MAR(DATE) ?  3 : \
        				 IS_APR(DATE) ?  4 : \
        				 IS_MAY(DATE) ?  5 : \
        				 IS_JUN(DATE) ?  6 : \
        				 IS_JUL(DATE) ?  7 : \
        				 IS_AUG(DATE) ?  8 : \
        				 IS_SEP(DATE) ?  9 : \
        				 IS_OCT(DATE) ? 10 : \
        				 IS_NOV(DATE) ? 11 : \
        				 IS_DEC(DATE) ? 12 : 0 )

#define NX_DATE(DATE)	(uint64_t)(	(DATE[7]  - '0')* 10000000	+	\
					 				(DATE[8]  - '0')* 1000000	+	\
					 				(DATE[9]  - '0')* 100000	+	\
					 				(DATE[10] - '0')* 10000		+	\
					 				GET_MONTH(DATE)	* 100		+	\
					 				(DATE[4]  - '0')* 10		+	\
					 				(DATE[5]  - '0')* 1 )

#define NX_TIME(TIME)	(uint64_t)( (TIME[0]  - '0')* 100000	+	\
					 				(TIME[1]  - '0')* 10000		+	\
					 				(TIME[2]  - '0')* 1000		+	\
					 				(TIME[3]  - '0')* 100		+	\
					 				(TIME[4]  - '0')* 10		+	\
					 				(TIME[5]  - '0')* 1 )

#endif	// __NX_VERSION_H__
