#ifndef __CRC32_H__
#define __CRC32_H__

#include <stdint.h>

uint32_t calc_crc32( uint32_t crc, const uint8_t *buf, int32_t len );

#endif	//	__CRC32_H__
