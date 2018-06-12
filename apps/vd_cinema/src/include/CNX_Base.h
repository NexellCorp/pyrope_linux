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

#ifndef __CNX_BASE_H__
#define __CNX_BASE_H__

#include <stdint.h>
#include <pthread.h>

//------------------------------------------------------------------------------
class CNX_Mutex
{
public:
	CNX_Mutex()
	{
		pthread_mutex_init( &m_hLock, NULL );
	}

	~CNX_Mutex()
	{
		pthread_mutex_destroy( &m_hLock );
	}

public:
	void Lock()
	{
		pthread_mutex_lock( &m_hLock );
	}

	void Unlock()
	{
		pthread_mutex_unlock( &m_hLock );
	}

private:
	pthread_mutex_t m_hLock;

private:
	CNX_Mutex (const CNX_Mutex &Ref);
	CNX_Mutex &operator=(const CNX_Mutex &Ref);
};

//------------------------------------------------------------------------------
class CNX_AutoLock
{
public:
	CNX_AutoLock( CNX_Mutex *pLock )
		: m_pLock( pLock )
	{
		m_pLock->Lock();
	}

	~CNX_AutoLock()
	{
		m_pLock->Unlock();
	}

protected:
	CNX_Mutex*	m_pLock;

private:
	CNX_AutoLock (const CNX_AutoLock &Ref);
	CNX_AutoLock &operator=(CNX_AutoLock &Ref);
};

//------------------------------------------------------------------------------
class CNX_Thread
{
public:
	CNX_Thread()
		: m_IsRunning( false )
	{
	}

	virtual ~CNX_Thread()
	{
		Stop();
	}

	virtual int32_t Start()
	{
		if( 0 != pthread_create( &m_hThread, NULL, CNX_Thread::ThreadStub, this ) )
		{
			m_IsRunning = false;
			return -1;
		}

		m_IsRunning = true;
		return 0;
	}

	virtual void Stop()
	{
		if( m_IsRunning )
		{
			pthread_join( m_hThread, NULL );
			//	consider pthread_tryjoin_np() function for dead lock
		}
	}

	virtual void ThreadProc() = 0;
	static void *ThreadStub( void *arg )
	{
		CNX_Thread *pObj = (CNX_Thread*) arg;
		pObj->ThreadProc();
		return (void*)0xDeadC0de;
	}
protected:
	pthread_t	m_hThread;
	bool		m_IsRunning;
};

#endif	// __CNX_BASE_H__
