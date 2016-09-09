#ifndef __NX_TMSClient_h__
#define __NX_TMSClient_h__

#include <NX_TMSCommand.h>

int32_t NX_TConCommand( int32_t id, int32_t cmd, uint8_t *pBuf, int32_t *size );
int32_t NX_PFPGACommand( int32_t cmd, uint8_t *pBuf, int32_t *size );
int32_t NX_BATCommand( int32_t cmd, uint8_t *pBuf, int32_t *size );

#endif	// __NX_TMSClient_h__
