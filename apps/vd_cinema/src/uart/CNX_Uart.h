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

#ifndef __CNX_Uart_h__
#define __CNX_Uart_h__

#include <stdint.h>
#include <termios.h>

//#define DEFAULT_UART_SPEED	B230400
#define DEFAULT_UART_SPEED	B115200
//#define DEFAULT_UART_SPEED	B38400
//#define DEFAULT_UART_SPEED	B9600

class CNX_Uart{
public:
	CNX_Uart();
	virtual ~CNX_Uart();

	/*
		port : 0 ~ 5
		speed: B9600, B19200, B38400, B57600, B115200, B230400
	*/
	bool Init( int32_t port, int32_t speed = DEFAULT_UART_SPEED );
	int32_t Read( void *buf, int32_t size, int32_t timeout = 1000 );
	int32_t Write( void *buf, int32_t size );

private:
	int32_t	m_hUart;
	int32_t	m_PortNum;
	int32_t	m_Speed;
};

#endif	// __CNX_Uart_h__
