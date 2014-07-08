#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <poll.h>
#include <fcntl.h>
#include <uevent.h>

#include <HDMITest.h>
#include <utils.h>

#define HDMI_STATE_FILE     "/sys/class/switch/hdmi/state"


CHDMITest::CHDMITest()
{
	pthread_mutex_init( &m_hMutex, NULL );
}

CHDMITest::~CHDMITest()
{
	Stop();
	pthread_mutex_destroy( &m_hMutex );
}

bool CHDMITest::Start()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		return false;
	}

	m_bRunning = true;
	if( 0 != pthread_create( &m_hThread, NULL, ThreadStub, this ) )
	{
		m_bRunning = false;
		return false;
	}
	return true;
}

bool CHDMITest::Stop()
{
	CNX_AutoLock lock(&m_hMutex) ;
	if( m_bRunning )
	{
		pthread_join( m_hThread, NULL );
		m_bRunning = false;
	}
	return true;
}


void CHDMITest::ThreadProc()
{
	int err;	
	int fd;
	struct pollfd fds[1];
	static unsigned char uevent_desc[2048];
	char val;

	uevent_init();
	fd = open(HDMI_STATE_FILE, O_RDONLY);
	if( fd > 0 )
	{
		if (read(fd, (char *)&val, 1) == 1 && val == '1')
		{
			m_bIsConnected = true;
		}
		else
		{
			m_bIsConnected = false;
		}
		close(fd);
	}

    fds[0].fd = uevent_get_fd();
    fds[0].events = POLLIN;
	while(m_bRunning)
	{
        err = poll(fds, 1, 500);

		if (err > 0)
		{
			if (fds[0].revents & POLLIN)
			{
				uevent_next_event((char *)uevent_desc, sizeof(uevent_desc) - 2);
				int hdmi = !strcmp((const char *)uevent_desc, (const char *)"change@/devices/virtual/switch/hdmi");
				if (hdmi)
				{
					fd = open(HDMI_STATE_FILE, O_RDONLY);
					if (fd < 0)
					{
						printf("failed to open hdmi state fd: %s", HDMI_STATE_FILE);
					}
					if( fd > 0 )
					{
						if (read(fd, &val, 1) == 1 && val == '1') {
							m_bIsConnected = true;
							printf("Connected\n");
						}
						else
						{
							m_bIsConnected = false;
							printf("Disconnected\n");
						}
						close(fd);
					}
				}
			}
		}
		else if (err == -1)
		{
			printf("error in vsync thread \n");
		}
    }
}
