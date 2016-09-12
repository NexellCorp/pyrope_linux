#include <CNX_GpioControl.h>

//	GPIO Port NAP <--> SAP
#define UART_REQUEST	GPIOA16
#define BOOT_OK0		GPIOD11
#define BOOT_OK1		GPIOD10


//	P.FPGA  --> SAP
//         |
//          --> NAP
#define PFPGA_OK		ALIVE2


#define PFPGA_FAN_L
#define PFPGA_FAN_R
