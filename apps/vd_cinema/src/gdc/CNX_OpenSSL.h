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

#ifndef __CNX_OPENSSL_H__
#define __CNX_OPENSSL_H__

#ifdef __cplusplus

#include <stdint.h>
#include <openssl/pem.h>
#include <openssl/rsa.h>
#include <openssl/x509.h>

class CNX_OpenSSL
{
public:
	CNX_OpenSSL();
	~CNX_OpenSSL();

public:
	int32_t ReadPrivateKey( const char *pKeyFile );
	int32_t ReadPublicKey( const char *pKeyFile );
	int32_t ReadCertificate( const char *pCertFile );
	int32_t ReadCertificate( uint8_t *pBuf, int32_t iSize );

	int32_t GetPrivateKey( uint8_t **ppBuf );
	int32_t GetPublicKey( uint8_t **ppBuf );
	int32_t GetCertificate( uint8_t **ppBuf );

	int32_t	GetEncryptSize( void );
	int32_t Encrypt( uint8_t *pInData, int32_t iInSize, uint8_t **ppOutData, int32_t *iOutSize );
	int32_t Decrypt( uint8_t *pInData, int32_t iInSize, uint8_t **ppOutData, int32_t *iOutSize );

	int32_t Sign( uint8_t *pPlaneData, int32_t iPlaneSize, uint8_t **pSignData, int32_t *iSignSize );
	int32_t Verify( uint8_t *pPlaneData, int32_t iPlaneSize, uint8_t *pSignData, int32_t iSignSize );

	// For Debugging
	void	DumpHex( const char *pMsg, const uint8_t *pData, int32_t iSize );
	int32_t	CompareData( uint8_t *pData1, int32_t iSize1, uint8_t *pData2, int32_t iSize2 );

private:
	RSA			*m_pPrivKey;
	RSA			*m_pPubKey;
	X509		*m_pCert;

	uint8_t		*m_pPrivKeyBuf;
	uint8_t		*m_pPubKeyBuf;
	uint8_t		*m_pCertBuf;

	int32_t		m_iPrivKeySize;
	int32_t		m_iPubKeySize;
	int32_t		m_iCertSize;

private:
	CNX_OpenSSL (const CNX_OpenSSL &Ref);
	CNX_OpenSSL &operator=(const CNX_OpenSSL &Ref);
};

#endif	// __cplusplus

#endif	// __CNX_OPENSSL_H__
