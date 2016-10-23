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

#ifndef __NX_QUEUE_H__
#define __NX_QUEUE_H__

#include <pthread.h>

#define NX_MAX_QUEUE_ELEMENT	128

typedef struct NX_QUEUE{
	unsigned int head;
	unsigned int tail;
	unsigned int maxElement;
	unsigned int curElements;
	int bEnabled;
	void *pElements[NX_MAX_QUEUE_ELEMENT];
	pthread_mutex_t	hMutex;
}NX_QUEUE;

int NX_InitQueue( NX_QUEUE *pQueue, unsigned int maxNumElement );
int NX_PushQueue( NX_QUEUE *pQueue, void *pElement );
int NX_PopQueue( NX_QUEUE *pQueue, void **pElement );
int NX_GetNextQueuInfo( NX_QUEUE *pQueue, void **pElement );
unsigned int NX_GetQueueCnt( NX_QUEUE *pQueue );
void NX_DeinitQueue( NX_QUEUE *pQueue );

#endif	// __NX_QUEUE_H__
