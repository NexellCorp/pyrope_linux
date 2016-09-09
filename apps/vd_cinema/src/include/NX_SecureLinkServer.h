#ifndef __NX_SecureLinkServer_h__
#define __NX_SecureLinkServer_h__

int32_t NX_SLinkServerStart();
void NX_SLinkServerStop();
int32_t NX_SLinkServerGotoSleep();
int32_t NX_SLinkServerPowerOn( int32_t on );
void NX_SLinkServerRegEventCB( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam );
void NX_SLinkServerRegAliveCB( int32_t (*callback)( void * ), void *pParam );

#endif	// __NX_SecureLinkServer_h__
