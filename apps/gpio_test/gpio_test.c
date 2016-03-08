//------------------------------------------------------------------------------
//
//	Copyright (C) 2013 Nexell Co. All Rights Reserved
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>		//	strncmp
#include <stdint.h>		//	Standard Int
#include <unistd.h>		//	getopt & optarg
#include <nx_gpio.h>	//	See : <top>/library/include

void print_usage( char *appName )
{
	printf(
		"Usage : %s [options] -i [input file], [M] = mandatory, [O] = Optional \n"
		"  common options :\n"
		"     -h                : help\n"
		"     -g [a|b|c|d|e|l]  : GPIO group (a~e, l(alive)), default(a)\n"
		"     -p [0~31]         : Group a~e : 0~31, alive : 0~7, default(0)\n"
		"     -d [delay time]   : GPIO hold time, unit : msec, default(1000)\n"
		"     -v [0 or 1]       : Output value(0 or 1), default(1)\n"
		, appName
	);
}

//
//	GPIO Output Example Application
//
int main(int argc, char *argv[])
{
	NX_GPIO_HANDLE	hGpio = NULL;
	int32_t group = 0;
	int32_t port = 0;
	int32_t delay = 1000;	//	1sec
	int32_t value = 0;
	int32_t gpioNumber;
	int opt;
	int assign_group = 0;
	int assign_port = 0;

	if (argc == 1) {
		print_usage(argv[0]);
		return 0;
	}

	while( -1 != (opt=getopt(argc, argv, "hg:p:d:v:")))
	{
		switch( opt ){
			case 'h':
				print_usage(argv[0]);
				return 0;
			case 'g':
				if( 0 == strncmp(optarg, "a", 1) )
				{
					group = 0;
				}
				else if( 0 == strncmp(optarg, "b", 1) )
				{
					group = 1;
				}
				else if( 0 == strncmp(optarg, "c", 1) )
				{
					group = 2;
				}
				else if( 0 == strncmp(optarg, "d", 1) )
				{
					group = 3;
				}
				else if( 0 == strncmp(optarg, "e", 1) )
				{
					group = 4;
				}
				else if( 0 == strncmp(optarg, "l", 1) )
				{
					group = 5;
				}
				else{
					printf("invalid group id!!!\n");
					print_usage(argv[0]);
					return -1;
				}
				assign_group = 1;
				break;
			case 'p':
				{
					port = atoi(optarg);
					printf("Assigned Port number = %d\n", port);
					if( (port < 0) || (port > 31) )
					{
						printf("invalid port number!!!\n");
						print_usage(argv[0]);
						return -1;
					}
					assign_port = 1;
					break;
				}
			case 'd':
				delay = atoi(optarg);
				break;
			case 'v':
				value = atoi(optarg);
				if( value != 0 )
					value = 1;
				break;
			default:
				break;
		}
	}

	if (assign_group != 1 || assign_port != 1) {
		printf("Did not specify group or port\n");
		return -1;
	}

	gpioNumber = group * 32 + port;

	if( gpioNumber >= GPIO_MAX )
	{
		printf("Invalid GPIO port number!!!( %d >= GPIO_MAX(%d) )\n", gpioNumber, GPIO_MAX);
		return -1;
	}

	hGpio = NX_GpioInit(gpioNumber);
	if( hGpio == NULL )
	{
		printf("NX_GpioInit() failed!!!\n");
		return -1;
	}

	if( 0 != NX_GpioDirection(hGpio, GPIO_DIRECTION_OUT) )
	{
		printf("NX_GpioDirection() failed!!!\n");
		goto ErrorExit;
	}

	if( 0 != NX_GpioSetValue(hGpio, value) )
	{
		printf("NX_GpioSetValue((%d)) failed!!!\n", value);
		goto ErrorExit;
	}

	usleep(delay*1000);

ErrorExit:
	if( hGpio )
		NX_GpioDeinit(hGpio);

	return 0;
}
