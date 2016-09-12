#include <CNX_GpioControl.h>
#include <NX_Pwm.h>

//	GPIO Port NAP <--> SAP
#define UART_REQUEST	GPIOA16
#define BOOT_OK_0		GPIOD11
#define BOOT_OK_1		GPIOD10

//	PFGA <-> NAP Status 
#define PFPGA_BOOT_OK	GPIOA14

//	DOOR_TEMPER  --> SAP
//              |
//               --> NAP
#define DOOR_TEMPER		ALIVE2



//
//	PWM
//
#define PFPGA_FAN_L		1
#define PFPGA_FAN_R		2
