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

#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <CNX_OpenSSL.h>

#define NX_DTAG		"[CNX_OpenSSL]"
#include <NX_DbgMsg.h>

//------------------------------------------------------------------------------
CNX_OpenSSL::CNX_OpenSSL()
	: m_pPrivKey( NULL )
	, m_pPubKey( NULL )
	, m_pCert( NULL )
	, m_pPrivKeyBuf( NULL )
	, m_pPubKeyBuf( NULL )
	, m_pCertBuf( NULL )
{

}

//------------------------------------------------------------------------------
CNX_OpenSSL::~CNX_OpenSSL()
{
	if( m_pPrivKey )	RSA_free( m_pPrivKey );
	if( m_pPubKey )		RSA_free( m_pPubKey );
	if( m_pPrivKeyBuf )	free( m_pPrivKeyBuf );
	if( m_pPubKeyBuf )	free( m_pPubKeyBuf );
	if( m_pCertBuf )	free( m_pCertBuf );
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::ReadPrivateKey( const char *pKeyFile )
{
	if( pKeyFile == NULL || access(pKeyFile, F_OK) )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, Access KeyFile.\n");
		return -1;
	}

	FILE *pFile = fopen( pKeyFile, "r" );
	if( pFile == NULL )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, fopen(). ( %s )\n", pKeyFile );
		return -1;
	}

	m_pPrivKey = PEM_read_RSAPrivateKey( pFile, NULL, NULL, NULL );
	if( m_pPrivKey == NULL )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, PEM_read_RSAPrivateKey().\n");
		fclose( pFile );
		return -1;
	}

	rewind( pFile );
	fseek( pFile, 0, SEEK_END );

	m_iPrivKeySize = ftell( pFile );
	m_pPrivKeyBuf = (uint8_t*)malloc( m_iPrivKeySize );

	memset( m_pPrivKeyBuf, 0x00, m_iPrivKeySize );
	rewind( pFile );
	int32_t iRet =  fread( m_pPrivKeyBuf, 1, m_iPrivKeySize, pFile );
	fclose( pFile );

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::ReadPublicKey( const char *pKeyFile )
{
	if( pKeyFile == NULL || access(pKeyFile, F_OK) )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, Access KeyFile.\n");
		return -1;
	}

	FILE *pFile = fopen( pKeyFile, "r" );
	if( pFile == NULL )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, fopen(). ( %s )\n", pKeyFile );
		return -1;
	}

	m_pPubKey = PEM_read_RSA_PUBKEY( pFile, NULL, NULL, NULL );
	if( m_pPubKey == NULL )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, PEM_read_RSA_PUBKEY().\n");
		fclose( pFile );
		return -1;
	}

	rewind( pFile );
	fseek( pFile, 0, SEEK_END );

	m_iPubKeySize = ftell( pFile );
	m_pPubKeyBuf = (uint8_t*)malloc( m_iPubKeySize );

	memset( m_pPubKeyBuf, 0x00, m_iPubKeySize );
	rewind( pFile );
	int32_t iRet = fread( m_pPubKeyBuf, 1, m_iPubKeySize, pFile );
	fclose( pFile );

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::ReadCertificate( const char *pCertFile )
{
	if( pCertFile == NULL || access(pCertFile, F_OK) )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, Access CertFile.\n");
		return -1;
	}

	FILE *pFile = fopen( pCertFile, "r" );
	if( pFile == NULL )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, fopen(). ( %s )\n", pCertFile );
		return -1;
	}

	m_pCert = PEM_read_X509( pFile, NULL, NULL, NULL );
	if( m_pCert == NULL )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, PEM_read_X509().\n");
		fclose( pFile );
		return -1;
	}

	rewind( pFile );
	fseek( pFile, 0, SEEK_END );

	m_iCertSize = ftell( pFile );
	m_pCertBuf = (uint8_t*)malloc( m_iCertSize );

	memset( m_pCertBuf, 0x00, m_iCertSize );
	rewind( pFile );
	int32_t iRet = fread( m_pCertBuf, 1, m_iCertSize, pFile );
	fclose( pFile );

	EVP_PKEY *pPubKey = X509_get_pubkey( m_pCert );
	m_pPubKey = EVP_PKEY_get1_RSA( pPubKey );
	EVP_PKEY_free( pPubKey );

	return iRet;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::ReadCertificate( uint8_t *pBuf, int32_t iSize )
{
	BIO *pBio;

	pBio = BIO_new_mem_buf( (void*)pBuf, -1 );
	m_pCert = PEM_read_bio_X509( pBio, NULL, 0, NULL );
	if( m_pCert == NULL )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, PEM_read_bio_X509().\n");
		return -1;
	}

	EVP_PKEY *pPubKey = X509_get_pubkey( m_pCert );
	m_pPubKey = EVP_PKEY_get1_RSA( pPubKey );
	EVP_PKEY_free( pPubKey );

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::GetPrivateKey( uint8_t **ppBuf )
{
	*ppBuf = m_pPrivKeyBuf;
	return m_iPrivKeySize;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::GetPublicKey( uint8_t **ppBuf )
{
	*ppBuf = m_pPubKeyBuf;
	return m_iPubKeySize;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::GetCertificate( uint8_t **ppBuf )
{
	*ppBuf = m_pCertBuf;
	return m_iCertSize;
}

//------------------------------------------------------------------------------
void CNX_OpenSSL::DumpHex( const char *pMsg, const uint8_t *pData, int32_t iSize )
{
	printf("%s ( %d bytes ):", pMsg, iSize);

	for( int32_t i = 0; i < iSize; ++i)
	{
		if ((i % 16) == 0)
		{
			printf("\n%04x", i);
		}

		printf(" %02x", pData[i]);
	}

	printf("\n");
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::CompareData( uint8_t *pData1, int32_t iSize1, uint8_t *pData2, int32_t iSize2 )
{
	if( iSize1 != iSize2 )
	{
		NxDbgMsg(NX_DBG_DEBUG, "Fail, Mismatch Size.\n");
		return -1;
	}

	for( int32_t i = 0; i < iSize1; i++ )
	{
		if(  pData2[i] != pData2[i] )
		{
			NxDbgMsg(NX_DBG_DEBUG, "Fail, Mismatch Data.\n");
			return -1;
		}
	}

	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::GetEncryptSize( void )
{
	return (m_pPubKey != NULL) ? RSA_size(m_pPubKey) : 0;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::Encrypt( uint8_t *pInData, int32_t iInSize, uint8_t **ppOutData, int32_t *iOutSize )
{
	*iOutSize = RSA_public_encrypt( iInSize, pInData, *ppOutData, m_pPubKey, RSA_PKCS1_OAEP_PADDING );
	return *iOutSize;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::Decrypt( uint8_t *pInData, int32_t iInSize, uint8_t **ppOutData, int32_t *iOutSize )
{
	*iOutSize = RSA_private_decrypt( iInSize, pInData, *ppOutData, m_pPrivKey, RSA_PKCS1_OAEP_PADDING );
	return *iOutSize;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::Sign( uint8_t *pPlaneData, int32_t iPlaneSize, uint8_t **pSignData, int32_t *iSignSize )
{
	uint8_t hash[SHA_DIGEST_LENGTH];
	SHA1( pPlaneData, iPlaneSize, hash );

	uint32_t iSize;
	if( !RSA_sign( NID_sha1, hash, SHA_DIGEST_LENGTH, *pSignData, &iSize, m_pPrivKey ) )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, RSA_sign().\n");
		return -1;
	}

	*iSignSize = iSize;
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_OpenSSL::Verify( uint8_t *pPlaneData, int32_t iPlaneSize, uint8_t *pSignData, int32_t iSignSize )
{
	uint8_t hash[SHA_DIGEST_LENGTH];
	SHA1( pPlaneData, iPlaneSize, hash );

	if( !RSA_verify( NID_sha1, hash, SHA_DIGEST_LENGTH, pSignData, iSignSize, m_pPubKey ) )
	{
		NxDbgMsg(NX_DBG_ERR, "Fail, RSA_verify().\n");
		return -1;
	}

	return 0;
}
