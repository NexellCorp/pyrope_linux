//------------------------------------------------------------------------------
//
//  Copyright (C) 2013 Nexell Co. All Rights Reserved
//  Nexell Co. Proprietary & Confidential
//
//  NEXELL INFORMS THAT THIS CODE AND INFORMATION IS PROVIDED "AS IS" BASE
//  AND WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING
//  BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS
//  FOR A PARTICULAR PURPOSE.
//
//  Module      :
//  File        :
//  Description :
//  Author      : 
//  Export      :
//  History     :
//
//------------------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>		// malloc
#include <unistd.h>		// write
#include <fcntl.h>		// open
#include <assert.h>		// assert

#include "nx_gpio.h"

// create node / remove node
// node : /sys/calss/gpio/gpio[num]
// echo [gpio_num] > /sys/class/gpio/export
// echo [gpio_num] > /sys/class/gpio/unexport

// direction handling
// node : /sys/class/gpio/gpio[num]/direction
// echo "in" > direction
// echo "out" > direction

// write value / read value
// node : /sys/class/gpio/gpio[num]/value
// echo [valie] > value
// cat value

typedef struct {
	int32_t	port;			// gpio number
	int32_t direction;		// gpio direction
} NX_GPIO_HANDLE_INFO;

NX_GPIO_HANDLE NX_GpioInit( int32_t nGpio )
{
	int32_t fd = 0, len = 0;
	NX_GPIO_HANDLE_INFO	*pInfo;
	char buf[64];

	if( nGpio <= GPIO_ERROR || nGpio >= GPIO_MAX ) {
		printf("over range gpio number! ( gpio%d )\n", nGpio);
		return NULL;
	}

	// Check gpio node.
	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d", nGpio);

	if( !access(buf, F_OK) ) {
		printf("gpio%d already initialize.\n", nGpio);
	}
	else {

		if( 0 > (fd = open("/sys/class/gpio/export", O_WRONLY)) ){
			printf("cannot export gpio%d\n", nGpio);
			return NULL;
		}

		len = snprintf(buf, sizeof(buf), "%d", nGpio);
	
		if( 0 > write(fd, buf, len) ) {
			printf("cannot write gpio%d export file!\n", nGpio);
			close(fd);
			return NULL;
		}
	}

	// create handle.
	pInfo = (NX_GPIO_HANDLE_INFO*)malloc( sizeof(NX_GPIO_HANDLE_INFO) );

	pInfo->port			= nGpio;
	pInfo->direction	= GPIO_DIRECTION_IN;	// default input

	close(fd);

	return (NX_GPIO_HANDLE_INFO*)pInfo;
}

void	NX_GpioDeinit( NX_GPIO_HANDLE hGpio )
{
	int32_t fd = 0, len = 0;
	NX_GPIO_HANDLE_INFO	*pInfo;
	char buf[64];

	assert(hGpio);
	pInfo = (NX_GPIO_HANDLE_INFO*)hGpio;

	if( 0 > (fd = open("/sys/class/gpio/unexport", O_WRONLY)) ){
		printf("cannot unexport gpio%d\n", pInfo->port);
	}
	
	len = snprintf(buf, sizeof(buf), "%d", pInfo->port);

	if( 0 > write(fd, buf, len) ) {
		printf("cannot write gpio%d unexport file!\n", pInfo->port);
		close(fd);
	}
	
	close(fd);
	
	if(pInfo)
		free(pInfo);
}

int32_t	NX_GpioDirection( NX_GPIO_HANDLE hGpio, int32_t direction )
{
	int32_t fd = 0, len = 0;
	NX_GPIO_HANDLE_INFO	*pInfo;
	char buf[64];

	assert(hGpio);
	pInfo = (NX_GPIO_HANDLE_INFO*)hGpio;

	if( direction < GPIO_DIRECTION_IN || direction > GPIO_DIRECTION_OUT ) {
		printf("unknown gpio direction! ( %d )\n", direction);
		return -1;
	}

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/direction", pInfo->port);
	if( 0 > (fd = open(buf, O_WRONLY)) ){
		printf("cannot open gpio%d direction!\n", pInfo->port);
		return -1;
	}

	len = snprintf(buf, sizeof(buf), "%s", (direction == GPIO_DIRECTION_IN) ? "in" : "out");

	if( 0 > write(fd, buf, len) ) {
		printf("cannot write gpio%d direction!\n", pInfo->port);
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int32_t	NX_GpioSetValue( NX_GPIO_HANDLE hGpio, int32_t value )
{
	int32_t fd = 0, len = 0;
	NX_GPIO_HANDLE_INFO	*pInfo;
	char buf[64];

	assert(hGpio);
	pInfo = (NX_GPIO_HANDLE_INFO*)hGpio;

	if( value < 0 || value > 1 ) {
		printf("unknown gpio value! ( %d )\n", value);
		return -1;
	}

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", pInfo->port);
	if( 0 > (fd = open(buf, O_RDWR)) ){
		printf("cannot open gpio%d value!\n", pInfo->port);
		return -1;
	}

	len = snprintf(buf, sizeof(buf), "%d", value);
	if( 0 > write(fd, buf, len) ) {
		printf("cannot write gpio%d value!\n", pInfo->port);
		close(fd);
		return -1;
	}

	close(fd);

	return 0;
}

int32_t	NX_GpioGetValue( NX_GPIO_HANDLE hGpio )
{
	int32_t fd = 0, len = 0;
	NX_GPIO_HANDLE_INFO	*pInfo;
	char buf[64];

	assert(hGpio);
	pInfo = (NX_GPIO_HANDLE_INFO*)hGpio;

	len = snprintf(buf, sizeof(buf), "/sys/class/gpio/gpio%d/value", pInfo->port);
	if( 0 > (fd = open(buf, O_RDWR)) ){
		printf("cannot open gpio%d value!\n", pInfo->port);
		return -1;
	}

	if( 0 > (len = read(fd, buf, sizeof(buf))) ) {
		printf("cannot read gpio%d value!\n", pInfo->port);
		close(fd);
		return -1;
	}

	close(fd);
	
	return atoi(buf);
}
