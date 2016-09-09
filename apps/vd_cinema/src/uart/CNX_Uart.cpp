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
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <termios.h>
#include <sys/poll.h>

#include "CNX_Uart.h"

#define UART_DEV_PREFIX		"/dev/ttyAMA"

CNX_Uart::CNX_Uart()
	: m_hUart(-1)
	, m_PortNum(0)
	, m_Speed(B9600)
{
}

CNX_Uart::~CNX_Uart()
{
	if( m_hUart > 0 )
	{
		close(m_hUart);
	}
}

/*
	port : 0 ~ 5
	speed: B9600, B19200, B38400, B57600, B115200, B230400
*/
bool CNX_Uart::Init( int32_t port, int32_t speed )
{
	struct termios	newtio, oldtio;
	struct pollfd	pollEvent;
	char devName[32];

	sprintf( devName, "%s%d", UART_DEV_PREFIX, port );

	if( 0 > (m_hUart = open(devName, O_RDWR | O_NOCTTY)) ) {
		return false;
	}
	// if( 0 > (m_hUart = open(devName, O_RDWR | O_NOCTTY | O_NONBLOCK)) ) {
	// 	return false;
	// }

	tcgetattr(m_hUart, &oldtio);
	memset( &newtio, 0x00, sizeof(newtio) );

#if 0
	// Serial Port Setting (Canonical Mode)
	newtio.c_cflag      = speed | CS8 | CLOCAL | CREAD;
	newtio.c_iflag      = IGNPAR | ICRNL;
	newtio.c_oflag      = 0;
	//newtio.c_lflag      = ICANON;
#else
	//	RAW Data Mode
	newtio.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
	newtio.c_oflag &= ~OPOST;
	newtio.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
	newtio.c_cflag &= ~(CSIZE | PARENB);
	newtio.c_cflag |= CS8;
	newtio.c_cflag |= speed;
#endif

	tcflush( m_hUart, TCIFLUSH );
	tcsetattr( m_hUart, TCSANOW, &newtio );
	m_Speed = speed;
	return true;
}

/*
	timeout : milli-seconds
*/
int32_t CNX_Uart::Read( void *buf, int32_t size, int32_t timeout )
{
	int32_t readSize;
	int32_t hPoll;
	struct pollfd	pollEvent;

	if( m_hUart < 0  )
		return -1;

	//	Wait Event form UART
	pollEvent.fd		= m_hUart;
	pollEvent.events	= POLLIN | POLLERR;
	pollEvent.revents	= 0;
	hPoll = poll( (struct pollfd*)&pollEvent, 1, timeout );

	if( hPoll < 0 ) {
		return -2;
	}
	else if( hPoll > 0 ) {
		readSize = read(m_hUart, buf, size);
		if( readSize < 0 ){
			return -1;
		}
		return readSize;
	}
	else {
		//	timeout
		return 0;
	}
}


int32_t CNX_Uart::Write( void *buf, int32_t size )
{
	int32_t writtenSize;
	if( m_hUart < 0 )
		return -1;

	writtenSize = write( m_hUart, buf, size );
	return writtenSize;
}
