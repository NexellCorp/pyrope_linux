#include <stdio.h>
#include "NX_I2CController.h"
#include <pthread.h>
#include <unistd.h>

//I2C enabled...can not test
// static int32_t temp = 0;

class TestThread
{
public:
	TestThread(int32_t iCmd)
	{
		cmd = iCmd;
	}
	~TestThread()
	{
		pthread_join(m_Thread , NULL);
	}

	void Start( int id )
	{
		m_ThreadId = id;
		printf("threadid = %d\n", id);
		pthread_create(&m_Thread , NULL , TestThread::threadStub, (void*)this);
	}

	static void* threadStub(void* obj)
	{
		TestThread* m_pObj = (TestThread*) obj;
		m_pObj->threadProc();
		return (void*)0x1111;
	}

	void threadProc()
	{
		int32_t iIdx = -1;

		while(1)
		{
			if( 7 == m_ThreadId )
			{
				iIdx++;
			}

			if( 2 == iIdx )
			{
				cmd = 222;
				iIdx = 0;
			}



			int32_t iRet = NX_SendData((void*)this, cmd, NULL, m_ThreadId, NULL , NULL);

			if( 0 == iIdx )
			{
				cmd = 111;
			}

			usleep(1);
		}
	}

private:
	pthread_t m_Thread;
	int32_t cmd;
	int32_t m_ThreadId;
};


// void* threadwork(void *pThreadID)
// {
// 	int32_t m_ThreadId = *((int32_t*)pThreadID);
// 	int32_t iIdx = -1;
// 	int32_t cmd = 1;
// 	while(1)
// 	{
// 		if( 7 == m_ThreadId )
// 		{
// 			iIdx++;
// 		}
// 		if( 2 == iIdx )
// 		{
// 			cmd = 222;
// 			iIdx = 0;
// 		}
// 		NX_SendData((void*)&temp, cmd, NULL, m_ThreadId, NULL , NULL);
// 		if( 0 == iIdx )
// 		{
// 			cmd = 111;
// 		}
// 		usleep(1);
// 	}
// }

#define MAX_THREADS	20

int main (void)
{

	TestThread *thread[MAX_THREADS];

	for( int32_t i=0 ; i< MAX_THREADS ; i++ )
	{
		if(i == 7)
		{
			thread[i] = new TestThread(111);
			thread[i]->Start( i );
			continue;
		}
		thread[i] = new TestThread(1);
		thread[i]->Start( i );
	}

	for( int32_t i = 0 ; i< MAX_THREADS ; i++ )
	{
		delete thread[i];
	}

//===============================================================
	// int32_t a = 7;

	// pthread_t thread;
	// pthread_create(&thread , NULL ,  threadwork , (void*)&a);

	// pthread_join(thread , NULL);
//===============================================================

	int32_t iRet = NX_ReleaseInstance();
	printf("NX_ReleaseInstance ret : %d\n",iRet);
	printf("end of main cpp\n");
	return 0;
}
