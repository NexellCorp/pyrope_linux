#ifndef __SockUtil_h__
#define __SockUtil_h__

//	Local Socket Util
int32_t LS_Open( const char *pLocalFileName );
int32_t LS_Connect( const char *pLocalFileName );

//	TCP Socket Util
int32_t TCP_Open( short port );
int32_t TCP_Connect( const char *ipAddr, short port );
#endif	//	__SockUtil_h__
