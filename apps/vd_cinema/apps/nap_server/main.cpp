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
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/un.h>
#include <linux/reboot.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <netinet/in.h>
#include <net/if.h>

#include <CNX_Base.h>
#include <CNX_GpioControl.h>

#include <NX_IPCServer.h>
#include <NX_TMSServer.h>
#include <NX_SecureLinkServer.h>
#include <NX_Pwm.h>

#include <Board_Port.h>
#include <NX_Version.h>

#include <cutils/android_reboot.h>

#define NX_DTAG	"[N.AP Server]"
#include <NX_DbgMsg.h>

//------------------------------------------------------------------------------
//
//	Tamper Checker : Door Tamper / Marriage Tamper
//

//------------------------------------------------------------------------------
class NapTamperChecker
	: public CNX_Thread
{
public:
	NapTamperChecker()
		: m_bThreadRun( false )
		, m_pCbPrivate( NULL )
		, m_pCallback( NULL )
	{
		m_hGpio[DOOR_TAMPER] = new CNX_GpioControl();
		m_hGpio[DOOR_TAMPER]->Init( GPIO_TAMPER_DOOR );

		for( int32_t i = 0; i < MAX_TAMPER_NUM; i++ )
		{
			m_hGpio[i]->SetDirection( GPIO_DIRECTION_IN );
			m_GpioValue[i] = 0;
		}

		m_GpioValue[DOOR_TAMPER] = 1;

		m_bThreadRun = true;
		Start();
	}

	virtual ~NapTamperChecker()
	{
		if( true == m_bThreadRun )
		{
			m_bThreadRun = false;
			Stop();
		}

		for( int32_t i = 0; i < MAX_TAMPER_NUM; i++ )
		{
			m_hGpio[i]->Deinit();
			delete m_hGpio[i];
		}
	}

	//	Implement Pure Virtual Function
	virtual void ThreadProc()
	{
		while(m_bThreadRun)
		{
			usleep(100000);
			for( int32_t i = 0; i < MAX_TAMPER_NUM; i++ )
			{
				int32_t iValue = m_hGpio[i]->GetValue();
				if( 0 > iValue ) continue;
				if( m_GpioValue[i] != iValue )
				{
					printf("Detect Gpio %s.\n", (iValue == 1) ? "High" : "Low");

					if( m_pCallback ) m_pCallback( m_pCbPrivate, i, iValue );
					m_GpioValue[i] = iValue;
				}
			}
		}
	}

	void RegisterCallback( int32_t (*callback)( void *, uint32_t , uint32_t ), void *pCbPrivate )
	{
		m_pCbPrivate = pCbPrivate;
		m_pCallback  = callback;
	}

private:
	enum { DOOR_TAMPER, MAX_TAMPER_NUM };

	bool m_bThreadRun;
	void *m_pCbPrivate;
	int32_t (*m_pCallback)( void *, uint32_t , uint32_t );

	CNX_GpioControl *m_hGpio[MAX_TAMPER_NUM];
	int32_t m_GpioValue[MAX_TAMPER_NUM];
};

//------------------------------------------------------------------------------
class NapPwm
{
public:
	NapPwm()
	{
		m_hFanL = NX_PwmInit( PFPGA_FAN_L );
		m_hFanR = NX_PwmInit( PFPGA_FAN_R );
	}

	virtual ~NapPwm()
	{
		NX_PwmDeinit( m_hFanL, true );
		NX_PwmDeinit( m_hFanR, true );
	}

public:
	void SetDuty( uint32_t iDuty )
	{
		NX_PwmSetFreqDuty( m_hFanL, DEF_FREQUENCY, iDuty );
		NX_PwmSetFreqDuty( m_hFanR, DEF_FREQUENCY, iDuty );
	}

private:
	enum { DEF_FREQUENCY = 1000 };

	NX_PWM_HANDLE	m_hFanL;
	NX_PWM_HANDLE	m_hFanR;
};

//------------------------------------------------------------------------------
class NapHelper
	: public CNX_Thread
{
public:
	NapHelper()
	{
		m_bThreadRun = true;
		Start();
	}

	virtual ~NapHelper()
	{
		if( true == m_bThreadRun )
		{
			m_bThreadRun = false;
			Stop();
		}
	}

	//	Implement Pure Virtual Function
	virtual void ThreadProc()
	{
		const char *pSockName	= "cinema.helper";

		const char *pReqNetwork	= "network";
		const char *pReqLink	= "link";
		const char *pBusyBox	= "busybox";

		const char *pResult[2] = {
			"false", "true",
		};

		int32_t sock, len, clnSock;
		struct sockaddr_un addr;

		if( 0 > (sock = socket(AF_UNIX, SOCK_STREAM, 0)) )
		{
			printf("Fail, socket().\n");
			return ;
		}

		addr.sun_family  = AF_UNIX;
		addr.sun_path[0] = '\0';	// for abstract namespace
		strcpy( addr.sun_path + 1, pSockName );

		len = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

		if( 0 > bind(sock, (struct sockaddr *)&addr, len) )
		{
			printf("Fail, bind().\n");
			return ;
		}

		if( 0 > listen(sock, 5) )
		{
			printf("Fail, listen().\n");
			return ;
		}

		char readBuf[1024];

		while(m_bThreadRun)
		{
			clnSock = accept( sock, (struct sockaddr*)&addr, (socklen_t*)&len );

			memset( readBuf, 0x00, sizeof(readBuf) );
			read( clnSock, readBuf, sizeof(readBuf) );

			if( !strncmp( readBuf, pReqNetwork, strlen(pReqNetwork)) )
			{
				FILE *pFile = fopen("/system/bin/nap_network", "w");
				fprintf(pFile, "%s\n", readBuf + strlen(pReqNetwork));
				fclose(pFile);
				sync();

				system("/system/bin/nap_network.sh restart");
			}

			if( !strncmp( readBuf, pReqLink, strlen(pReqLink)) )
			{
				if( 0 < CheckEthLink("eth0") )
				{
					write( clnSock, pResult[1], strlen(pResult[1]) );
				}
				else
				{
					write( clnSock, pResult[0], strlen(pResult[0]) );
				}
			}

			if( !strncmp( readBuf, pBusyBox, strlen(pBusyBox)) )
			{
				system( readBuf );
			}

			close(clnSock);
		}
	}

private:
	int32_t CheckEthLink( const char *pIfName )
	{
		int32_t sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);
		if( 0 > sock ) {
			printf("Fail, socket().\n");
			return -1;
		}

		struct ifreq if_req;
		(void)strncpy( if_req.ifr_name, pIfName, sizeof(if_req.ifr_name) );
		int32_t ret = ioctl( sock, SIOCGIFFLAGS, &if_req );
		close( sock );
		if( 0 > ret ) {
			printf("Fail, ioctl().\n");
			return -1;
		}

		return (if_req.ifr_flags & IFF_UP) && (if_req.ifr_flags & IFF_RUNNING);
	}

private:
	bool m_bThreadRun;
};

//------------------------------------------------------------------------------
static NapTamperChecker	*gstTamper = NULL;
static NapHelper *gstHelper = NULL;

//------------------------------------------------------------------------------
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

	if( NULL != gstTamper) delete gstTamper;
	if( NULL != gstHelper ) delete gstHelper;

	NX_SLinkServerStop();
	NX_TMSServerStop();
	NX_IPCServerStop();

	exit(EXIT_FAILURE);
}

//------------------------------------------------------------------------------
static void register_signal( void )
{
	signal( SIGINT,  signal_handler );
	signal( SIGTERM, signal_handler );
	signal( SIGABRT, signal_handler );
}

//------------------------------------------------------------------------------
#define POWER_UNKNOWN	0
#define POWER_UNPLUGED	1
#define POWER_PLUGED	2

#define BATTERY_STATUS	"/sys/devices/platform/i2c-gpio.3/i2c-3/3-0032/nxe2000-battery/power_supply/battery/status"

//------------------------------------------------------------------------------
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

//------------------------------------------------------------------------------
static int32_t SendRemote( const char *pSockName, const char *pMsg )
{
	int32_t sock, len;
	struct sockaddr_un addr;

	if( 0 > (sock = socket(AF_UNIX, SOCK_STREAM, 0)) )
	{
		printf("Fail, socket().\n");
		return -1;
	}

	addr.sun_family  = AF_UNIX;
	addr.sun_path[0] = '\0';	// for abstract namespace
	strcpy( addr.sun_path + 1, pSockName );

	len = 1 + strlen(pSockName) + offsetof(struct sockaddr_un, sun_path);

	if( 0 > connect(sock, (struct sockaddr *) &addr, len))
	{
		printf("Fail, connect(). ( node: %s )\n", pSockName);
		close( sock );
		return -1;
	}

	if( 0 > write(sock, pMsg, strlen(pMsg)) )
	{
		printf("Fail, write().\n");
		close( sock );
		return -1;
	}

	close( sock );
	return 0;
}

//------------------------------------------------------------------------------
static int32_t SecurelinkeEventCallback( void * /*pParam*/, int32_t eventCode, void *pEvtData, int32_t dataSize )
{
	NxDbgMsg(NX_DBG_VBS, "Receive Event Message ( 0x%08x )\n", eventCode);
	static int32_t bSetVersion = false;

	const char *pResult[5] = { "BootDone", "Alive", "Marriage", "Divorce" };

	if( eventCode == 0x00000001 ) SendRemote( "cinema.secure", pResult[0] );			// CMD_BOOT_DONE
	if( eventCode == 0x00000002 ) SendRemote( "cinema.secure", pResult[1] );			// CMD_ALIVE
	if( eventCode == 0x00000101 ) SendRemote( "cinema.secure", pResult[2] );			// CMD_MARRIAGE
	if( eventCode == 0x00000102 ) SendRemote( "cinema.secure", pResult[3] );			// CMD_DIVORCE

	if( eventCode == 0x00000701 && pEvtData != NULL && dataSize != 0 && !bSetVersion )	// CMD_VERSION
	{
		char szVersion[64] = { 0x00, };
		memcpy( szVersion, pEvtData, dataSize );

		NX_SetSapVersion( (uint8_t*)szVersion );
		bSetVersion = true;
	}

	return 0;
}

//------------------------------------------------------------------------------
static int32_t TamperCheckerCallback( void * /*pPrivate*/, uint32_t iTamperIndex, uint32_t iTamperValue )
{
	const char *pTamperIndex[2] = {	"DoorTamper", "MarriageTamper" };
	char msg[1024] = {0x00, };

	if( iTamperIndex == 0 && iTamperValue == 1 )
	{
		sprintf( msg, "Error %s", pTamperIndex[iTamperIndex] );
		SendRemote( "cinema.tamper", msg );
	}

	if( iTamperIndex == 1 && iTamperValue == 0 )
	{
		sprintf( msg, "Error %s", pTamperIndex[iTamperIndex] );
		SendRemote( "cinema.tamper", msg );
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t main( void )
{
	register_signal();

	//	Set Version Information
	char szVersion[1024] = { 0x00, };
	snprintf( szVersion, sizeof(szVersion), "%s ( %lld )", NX_VERSION_NAP, NX_DATE(__DATE__) );
	NX_SetNapVersion( (uint8_t*)szVersion );

	//	Start IPC Server
	if( 0 != NX_IPCServerStart() )
	{
		NxErrMsg("IPC service demon start failed!!!\n");
		exit(-1);
	}

	//	Start TMS Server
	if( 0 != NX_TMSServerStart() )
	{
		NxErrMsg("TMS demon start failed!!!\n");
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

	//
	//
	//
	gstTamper = new NapTamperChecker();
	gstHelper = new NapHelper();

	gstTamper->RegisterCallback(TamperCheckerCallback, NULL);

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
				NX_IPCServerStop();

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
	NX_IPCServerStop();

	return 0;
}
