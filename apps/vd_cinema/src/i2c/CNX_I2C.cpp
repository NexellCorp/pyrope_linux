#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h> 			/* error */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>

#include <linux/i2c.h>
#include <linux/i2c-dev.h>

#include <CNX_I2C.h>

#define I2C_DEVICE_PREFIX	"/dev/i2c-"


CNX_I2C::CNX_I2C( int32_t portNo )
	: m_I2CPort( portNo )
	, m_Handle( -1 )
{
}

CNX_I2C::~CNX_I2C()
{
	Close();
}

bool CNX_I2C::Open()
{
	char deviceName[64];
	sprintf( deviceName, "%s%d", I2C_DEVICE_PREFIX, m_I2CPort );
	m_Handle = open( deviceName, O_RDWR, 644 );

	if( 0 > m_Handle )
	{
		return false;
	}
	return true;
}

void CNX_I2C::Close()
{
	if( m_Handle > 0 )
	{
		int32_t nErr = fcntl(m_Handle, F_GETFL) != -1 || errno != EBADF;
		if( nErr )
		{
			close( m_Handle );
			m_Handle = -1;
		}
	}
}

int32_t CNX_I2C::Read( int32_t id, int32_t addr )
{
	// Use 16 Address
	uint8_t addrBuf[2];
	uint8_t dataBuf[2] = {0,0};
	int32_t ret;

	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msgs[1];

	if( !IsValidFd() )
		return -1;


	//	Make Address Buffer
	addrBuf[0] = (uint8_t)(addr & 0xFF);
	addrBuf[1] = (uint8_t)((addr >> 8) & 0xFF);

	//
	//	Write Address
	//
	{
		msgs[0].addr  = (id>>1);
		msgs[0].flags = !I2C_M_RD;
		msgs[0].len   = (2);
		msgs[0].buf   = (uint8_t *)addrBuf;

		rdwr.msgs = msgs;
		rdwr.nmsgs = 1;

		ret = ioctl(m_Handle, I2C_RDWR, &rdwr);
		if (0 > ret)
			return ret;
	}

	//
	//	Read Data
	//
	{
		msgs[0].addr  = (id>>1);
		msgs[0].flags = I2C_M_RD;
		msgs[0].len   = 2;
		msgs[0].buf   = (__u8 *)dataBuf;

		rdwr.msgs = msgs;
		rdwr.nmsgs = 1;
		ret = ioctl(m_Handle, I2C_RDWR, &rdwr);
		if (0 > ret)
			return ret;
	}
	ret = (dataBuf[0] << 8) | dataBuf[1];
	return ret;
}

int32_t CNX_I2C::ReadNoStop( int32_t id, int32_t addr )
{
	// Use 16 Address
	uint8_t addrBuf[2];
	uint8_t dataBuf[2] = {0,0};
	int32_t ret;

	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msgs[2];

	if( !IsValidFd() )
		return -1;


	//	Make Address Buffer
	addrBuf[0] = (uint8_t)(addr & 0xFF);
	addrBuf[1] = (uint8_t)((addr >> 8) & 0xFF);

	//	Address Setting
	msgs[0].addr  = (id>>1);
	msgs[0].flags = !I2C_M_RD;
	msgs[0].len   = (2);
	msgs[0].buf   = (uint8_t *)addrBuf;

	//	Data Part
	msgs[1].addr  = (id>>1);
	msgs[1].flags = I2C_M_RD;
	msgs[1].len   = 2;
	msgs[1].buf   = (__u8 *)dataBuf;


	rdwr.msgs = msgs;
	rdwr.nmsgs = 2;

	ret = ioctl(m_Handle, I2C_RDWR, &rdwr);
	if (0 > ret)
		return ret;

	ret = (dataBuf[0] << 8) | dataBuf[1];
	return ret;
}

int32_t CNX_I2C::WriteN( int32_t id, int32_t addr, void *buf, int32_t size )
{
	struct i2c_rdwr_ioctl_data rdwr;
	struct i2c_msg msgs[2];
	uint8_t *outBuf;
	int32_t ret;

	if( !IsValidFd() )
		return -1;

	outBuf = (uint8_t*)malloc(size+2);

	memcpy( outBuf + 2, buf, size );

	//	Write address to send buffer
	outBuf[0] = (uint8_t)(addr & 0xFF);
	outBuf[1] = (uint8_t)((addr >> 8) & 0xFF);

	//	Send Data
	msgs[0].addr  = (id>>1);
	msgs[0].flags = !I2C_M_RD;
	msgs[0].len   = (size + 2);
	msgs[0].buf   = (uint8_t *)outBuf;

	rdwr.msgs = msgs;
	rdwr.nmsgs = 1;

	ret = ioctl(m_Handle, I2C_RDWR, &rdwr);

	free( outBuf );
	return ret;
}

bool CNX_I2C::IsValidFd()
{
	int32_t nErr = fcntl(m_Handle, F_GETFL) != -1 || errno != EBADF;
	return nErr?true:false;
}
