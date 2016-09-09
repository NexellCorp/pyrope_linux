#ifndef __CNX_I2C_H__
#define __CNX_I2C_H__

#include <stdint.h>

class CNX_I2C{
public:
	CNX_I2C( int32_t portNo );
	~CNX_I2C();

	bool Open();
	void Close();

	int32_t Read( int32_t id, int32_t addr );
	int32_t ReadNoStop( int32_t id, int32_t addr );
	int32_t Write( int32_t id, int32_t addr, int32_t value );
	int32_t WriteN( int32_t id, int32_t addr, void *buf, int32_t size );

private:
	bool IsValidFd();

private:
	int32_t		m_I2CPort;
	int32_t		m_Handle;		//	FD
};

#endif	// __CNX_I2C_H__
