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

#ifndef __BOARD_PORT_H__
#define __BOARD_PORT_H__

#include <CNX_GpioControl.h>

//
//	GPIO Port NAP <--> SAP
//
#define GPIO_UART_REQUEST		GPIOA16
#define GPIO_BOOT_OK_0			GPIOD11
#define GPIO_BOOT_OK_1			GPIOD10

//
//	PFGA <-> NAP Status
//
#define GPIO_PFPGA_DONE			GPIOA14

//	DOOR_TAMPER  --> SAP
//              |
//               --> NAP
#define GPIO_TAMPER_DOOR		ALIVE2
#define GPIO_TAMPER_MARRIAGE	// Not yet

//
//	PWM
//
#define PFPGA_FAN_L				1
#define PFPGA_FAN_R				2

#endif	// __BOARD_PORT_H__