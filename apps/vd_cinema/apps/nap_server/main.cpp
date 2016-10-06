#include <stdint.h>
#include <stdio.h>
#include <unistd.h>	// usleep
#include <stdlib.h>	// exit
#include <fcntl.h>
#include <linux/reboot.h>

#include <NX_TMSServer.h>
#include <NX_SecureLinkServer.h>
#include <CNX_GpioControl.h>
#include <Board_Port.h>

#include <cutils/android_reboot.h>

#define NX_DTAG	"[N.AP Server APP]"
#include <NX_DbgMsg.h>

static int32_t SecurelinkeEventCallback( void *pParam, int32_t eventCode, void *pEvtData, int32_t dataSize )
{
	(void*)pParam;
	(void*)pEvtData;
	NxDbgMsg(NX_DBG_VBS, "Receive Event Message (0x%08x, %d)\n", eventCode, dataSize);

	return 0;
}

//
//	UART_REQUEST / BOOT OK 0,1 / Door Temper / PFPGA OK
//

class NapGpio
{
public:
	NapGpio()
	{
		m_UartReq.Init(UART_REQUEST);
		m_UartReq.SetDirection(GPIO_DIRECTION_OUT);

		m_BootOk0.Init(BOOT_OK_0);
		m_BootOk0.SetDirection(GPIO_DIRECTION_OUT);

		m_BootOk1.Init(BOOT_OK_1);
		m_BootOk1.SetDirection(GPIO_DIRECTION_OUT);

		//	PFPGA
		m_PFPGAOk.Init(PFPGA_BOOT_OK);
		m_PFPGAOk.SetDirection(GPIO_DIRECTION_IN);

		//	Temper
		m_PFPGAOk.Init(DOOR_TEMPER);
		m_PFPGAOk.SetDirection(GPIO_DIRECTION_IN);
	}
	virtual ~NapGpio(){}

	//
	//	Command
	//
	void RequestMarriage()
	{
		m_UartReq.SetValue( 0 );
		usleep( 1000000 );
		m_UartReq.SetValue( 1 );
	}
	void RequestReboot()
	{
		m_BootOk1.SetValue( 0 );
		usleep( 1000000 );
		m_BootOk1.SetValue( 1 );
	}

public:
	CNX_GpioControl m_UartReq;
	CNX_GpioControl m_BootOk0;
	CNX_GpioControl m_BootOk1;
	CNX_GpioControl m_PFPGAOk;
	CNX_GpioControl m_DoorTemper;
};

static void signal_handler( int32_t signal )
{
	printf("Aborted by signal %s (%d)..\n", (char*)strsignal(signal), signal);

	switch( signal )
	{
		case SIGINT :
			printf("SIGINT..\n"); 	break;
		case SIGTERM :
			printf("SIGTERM..\n");	break;
		case SIGABRT :
			printf("SIGABRT..\n");	break;
		default :
			break;
	}

	NX_SLinkServerStop();
	NX_TMSServerStop();

	exit(EXIT_FAILURE);
}

static void register_signal( void )
{
	signal( SIGINT,  signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

#define POWER_PLUGED	2
#define POWER_UNPLUGED	1
#define POWER_UNKNOWN	0

#define BATTERY_STATUS	"/sys/devices/platform/i2c-gpio.3/i2c-3/3-0032/nxe2000-battery/power_supply/battery/status"

static int32_t GetBatteryStatus( void )
{
	int32_t fd = -1;
	char status[32] = {0, };

	if( 0 > (fd = open( BATTERY_STATUS, O_RDONLY )) ) {
		return -1;
	}

	if( 0 >= read( fd, status, sizeof(status) ) ) {
		close(fd);
		return -1;
	}

	if(fd > 0)
		close( fd );

	// Charging / Discharging / Full / ...
	if( !strncmp( "Discharging", status, 5 ) )
	{
		return POWER_UNPLUGED;
	}

	return POWER_PLUGED;
}

int32_t main( void )
{
	register_signal();

	//	Start TMS Server
	if( 0 != NX_TMSServerStart() )
	{
		NxErrMsg("TMS service demon start failed!!!\n");
		exit(-1);
	}

	//	Start Secure Link Service
	NX_SLinkServerRegEventCB( SecurelinkeEventCallback, NULL );

	if( 0 != NX_SLinkServerStart() )
	{
		NxErrMsg("Secure linke demon start failed!!!\n");
		exit(-1);
	}

	int32_t iBatteryStatus = POWER_UNKNOWN;

	while( 1 )
	{
		int32_t newStatus = GetBatteryStatus();
		if( iBatteryStatus != newStatus )
		{
			iBatteryStatus = newStatus;

			if( iBatteryStatus == POWER_UNPLUGED )
			{
				NxDbgMsg( NX_DBG_INFO, "Power Failure!!\n");
				
				NX_SLinkServerStop();
				NX_TMSServerStop();

				NX_SapPowerOn( 0 );
				android_reboot( ANDROID_RB_POWEROFF, 0, NULL );
			}
			else if( iBatteryStatus == POWER_PLUGED )
			{

			}
			else
			{
				NxErrMsg("Error, Power Detection!!\n");
			}
		}

		usleep(100000);
	}

	NX_SLinkServerStop();
	NX_TMSServerStop();

	return 0;
}
