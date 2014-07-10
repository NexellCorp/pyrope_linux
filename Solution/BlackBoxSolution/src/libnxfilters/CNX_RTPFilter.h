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

#ifndef __CNX_RTPFILTER_H__
#define __CNX_RTPFILTER_H__

#include <CNX_BaseFilter.h>

#ifdef __cplusplus

class CNX_RTPFilter
	: public CNX_BaseFilter
{
public:
	CNX_RTPFilter();
	virtual ~CNX_RTPFilter();

public:
	//------------------------------------------------------------------------
	virtual void		Init( void );
	virtual void		Deinit( void );
	//------------------------------------------------------------------------
	//	Override from CNX_BaseFilter
	//------------------------------------------------------------------------
	virtual	int32_t		Receive( CNX_Sample *pSample );
	virtual int32_t		ReleaseSample( CNX_Sample *pSample );

	virtual	int32_t		Run( void );
	virtual	int32_t		Stop( void );

protected:
	//------------------------------------------------------------------------
	virtual void		AllocateMemory( void );
	virtual void		FreeMemory( void );

	virtual int32_t		GetSample( CNX_Sample **ppSample );
	virtual int32_t		GetDeliverySample( CNX_Sample **ppSample );

protected:
			void		ThreadLoop( void );
	static 	void*		ThreadMain( void *arg );

private:
	//------------------------------------------------------------------------
	//	Filter status
	//------------------------------------------------------------------------
	int32_t				m_bInit;
	int32_t				m_bRun;
	CNX_Semaphore		*m_pSemIn;
	//------------------------------------------------------------------------
	//	Thread
	//------------------------------------------------------------------------
	int32_t				m_bThreadExit;
	pthread_t			m_hThread;
	//------------------------------------------------------------------------
	//	RTP
	//------------------------------------------------------------------------
	
	//------------------------------------------------------------------------
	//	Input / Output Buffer
	//------------------------------------------------------------------------
	enum { MAX_BUFFER = 32 };

	CNX_SampleQueue		m_SampleInQueue;
};

#endif //	__cplusplus

#endif	// __CNX_RTPFILTER_H__