#ifndef __NX_TMSServer_h__
#define __NX_TMSServer_h__

#include <stdint.h>

#define	TMS_SERVER_FILE "/data/local/tmp/tms_server"
#define	MAX_PAYLOAD_SIZE	(128*1024)

int32_t NX_TMSServerStart();
void NX_TMSServerStop();

#endif	// __NX_TMSServer_h__
