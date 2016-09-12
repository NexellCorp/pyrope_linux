#ifndef __NX_SecureLinkServer_h__
#define __NX_SecureLinkServer_h__

int32_t NX_SLinkServerStart();
void NX_SLinkServerStop();
int32_t NX_SLinkServerGotoSleep();
void NX_SLinkServerRegEventCB( int32_t (*callback)( void *, int32_t , void *, int32_t ), void *pParam );

int32_t NX_SapPowerOn( int32_t on );
#endif	// __NX_SecureLinkServer_h__
