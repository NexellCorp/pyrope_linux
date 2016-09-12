#include <stdint.h>
#include <stdio.h>
#include <unistd.h>	// usleep
#include <stdlib.h>	// exit

#include <NX_TMSServer.h>
#include <NX_SecureLinkServer.h>
#include <CNX_GpioControl.h>
#include <Board_Port.h>

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


int32_t main(int32_t argc, char *argv[])
{

	//	SAP Power On
	NX_SapPowerOn( 1 );

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

	while( 1 )
	{
		usleep(100000);
	}

	return 0;
}
