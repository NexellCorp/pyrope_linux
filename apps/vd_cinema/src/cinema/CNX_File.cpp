//------------------------------------------------------------------------------
//
//	Copyright (C) 2018 Nexell Co. All Rights Reserved
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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include <regex.h>
#include <time.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>
#if NX_ENABLE_CHECKSUM
#include <openssl/md5.h>
#endif

#include "CNX_File.h"

//------------------------------------------------------------------------------
CNX_File::CNX_File()
	: m_pFile( NULL )
	, m_pBasename( NULL )
	, m_pDirname( NULL )
	, m_pExtension( NULL )
	, m_pAccessDate( NULL )
	, m_pModifyDate( NULL )
	, m_pChangeDate( NULL )
	, m_pChecksum( NULL )
{

}

//------------------------------------------------------------------------------
CNX_File::CNX_File( const char *pFile )
	: m_pFile( NULL )
	, m_pBasename( NULL )
	, m_pDirname( NULL )
	, m_pExtension( NULL )
	, m_pAccessDate( NULL )
	, m_pModifyDate( NULL )
	, m_pChangeDate( NULL )
	, m_pChecksum( NULL )
{
	if( !access(pFile, F_OK) )
	{
		m_pFile = strdup( pFile );
	}
}

//------------------------------------------------------------------------------
CNX_File::~CNX_File()
{
	Close();
}

//------------------------------------------------------------------------------
int32_t CNX_File::Open( const char *pFile )
{
	if( 0 > access(pFile, F_OK) )
		return -1;

	Close();

	m_pFile = strdup( pFile );
	return 0;
}

//------------------------------------------------------------------------------
void CNX_File::Close()
{
	if( m_pFile )		free( m_pFile );
	if( m_pBasename )	free( m_pBasename );
	if( m_pDirname )	free( m_pDirname );
	if( m_pExtension )	free( m_pExtension );
	if( m_pAccessDate ) free( m_pAccessDate );
	if( m_pModifyDate ) free( m_pModifyDate );
	if( m_pChangeDate ) free( m_pChangeDate );
	if( m_pChecksum )	free( m_pChecksum );
}

//------------------------------------------------------------------------------
int32_t CNX_File::IsExist()
{
	struct stat statinfo;
	if( 0 > stat( m_pFile, &statinfo) )
		return false;

	return true;
}

//------------------------------------------------------------------------------
int32_t CNX_File::IsRegularFile()
{
	struct stat statinfo;
	if( 0 > stat( m_pFile, &statinfo) )
		return 0;

	return S_ISREG( statinfo.st_mode );
}

//------------------------------------------------------------------------------
int32_t CNX_File::IsDirectory()
{
	struct stat statinfo;
	if( 0 > stat( m_pFile, &statinfo) )
		return 0;

	return S_ISDIR( statinfo.st_mode );
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetSize()
{
	struct stat statinfo;
	if( 0 > stat( m_pFile, &statinfo) )
		return 0;

	return statinfo.st_size;
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetName( char **ppResult )
{
	*ppResult = m_pFile;
	return (*ppResult != NULL) ? 0 : -1;
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetDirname( char **ppResult )
{
	*ppResult = NULL;
	if( m_pDirname ) {
		free( m_pDirname );
		m_pDirname = NULL;
	}

	m_pDirname = strdup( m_pFile );
	if( m_pDirname )
		*ppResult = dirname( m_pDirname );

	return (*ppResult != NULL) ? 0 : -1;
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetBasename( char **ppResult )
{
	*ppResult = NULL;
	if( m_pBasename ) {
		free( m_pBasename );
		m_pBasename = NULL;
	}

	m_pBasename = strdup( m_pFile );
	if( m_pBasename )
		*ppResult = basename( m_pBasename );

	return (*ppResult != NULL) ? 0 : -1;
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetExtension( char **ppResult )
{
	*ppResult = NULL;
	if( m_pExtension ) {
		free( m_pExtension );
		m_pExtension = NULL;
	}

	char *pTemp = m_pFile + strlen(m_pFile) - 1;
	while( pTemp != m_pFile )
	{
		if( *pTemp == '.' ) break;
		pTemp--;
	}

	if( pTemp != m_pFile ) {
		m_pExtension = (char*)malloc( strlen(pTemp+1) + 1 );
		memcpy( m_pExtension, pTemp+1, strlen(pTemp+1) + 1 );
		*ppResult = m_pExtension;
	}

	return (*ppResult != NULL) ? 0 : -1;
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetAccessDate( char **ppResult )
{
	*ppResult = NULL;
	if( m_pAccessDate ) {
		free( m_pAccessDate );
		m_pAccessDate = NULL;
	}

	struct stat statinfo;
	if( 0 > stat(m_pFile, &statinfo) )
		return -1;

	m_pAccessDate = (char*)malloc( MAX_DATE_NUM );
	memset( m_pAccessDate, 0x00, MAX_DATE_NUM );

	struct tm *tminfo = localtime( (time_t*)&statinfo.st_atime );
	sprintf( m_pAccessDate, "%04d-%02d-%02d %02d:%02d:%02d",
		tminfo->tm_year + 1900, tminfo->tm_mon + 1, tminfo->tm_mday,
		tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec );

	*ppResult = m_pAccessDate;
	return statinfo.st_atime;
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetModifyDate( char **ppResult )
{
	*ppResult = NULL;
	if( m_pModifyDate ) {
		free( m_pModifyDate );
		m_pModifyDate = NULL;
	}

	struct stat statinfo;
	if( 0 > stat(m_pFile, &statinfo) )
		return -1;

	m_pModifyDate = (char*)malloc( MAX_DATE_NUM );
	memset( m_pModifyDate, 0x00, MAX_DATE_NUM );

	struct tm *tminfo = localtime( (time_t*)&statinfo.st_mtime );
	sprintf( m_pModifyDate, "%04d-%02d-%02d %02d:%02d:%02d",
		tminfo->tm_year + 1900, tminfo->tm_mon + 1, tminfo->tm_mday,
		tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec );

	*ppResult = m_pModifyDate;
	return statinfo.st_mtime;
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetChangeDate( char **ppResult )
{
	*ppResult = NULL;
	if( m_pChangeDate ) {
		free( m_pChangeDate );
		m_pChangeDate = NULL;
	}

	struct stat statinfo;
	if( 0 > stat(m_pFile, &statinfo) )
		return -1;

	m_pChangeDate = (char*)malloc( MAX_DATE_NUM );
	memset( m_pChangeDate, 0x00, MAX_DATE_NUM );

	struct tm *tminfo = localtime( (time_t*)&statinfo.st_ctime );
	sprintf( m_pChangeDate, "%04d-%02d-%02d %02d:%02d:%02d",
		tminfo->tm_year + 1900, tminfo->tm_mon + 1, tminfo->tm_mday,
		tminfo->tm_hour, tminfo->tm_min, tminfo->tm_sec );

	*ppResult = m_pChangeDate;
	return statinfo.st_ctime;
}

#if NX_ENABLE_CHECKSUM
//------------------------------------------------------------------------------
int32_t CNX_File::GetChecksum( char **ppResult )
{
	*ppResult = NULL;
	if( m_pChecksum ) {
		free( m_pChecksum );
		m_pChecksum = NULL;
	}

	struct stat statinfo;
	if( 0 > stat( m_pFile, &statinfo) )
		return -1;

	FILE *hFile = fopen( m_pFile, "r" );
	if( NULL == hFile )
		return -1;

	if( hFile )
	{
		size_t iSize = statinfo.st_size;
		char *pData = (char*)malloc( iSize );

		size_t iReadSize = fread( pData, 1, iSize, hFile );
		fclose( hFile );

		if( iSize != iReadSize ) {
			free( pData );
			return -1;
		}

		uint8_t digest[MD5_DIGEST_LENGTH];
		MD5_CTX ctx;
		MD5_Init( &ctx );
		MD5_Update( &ctx, pData, iSize );
		MD5_Final( digest, &ctx );

		m_pChecksum = (char*)malloc( MD5_DIGEST_LENGTH * 2 + 1 );
		memset( m_pChecksum, 0x00, MD5_DIGEST_LENGTH * 2 + 1 );

		for( int32_t i = 0; i < MD5_DIGEST_LENGTH; i++ )
		{
			sprintf( m_pChecksum + (i*2), "%02X", digest[i] );
		}

		*ppResult = m_pChecksum;
		free( pData );
	}
	return 0;
}
#endif

//------------------------------------------------------------------------------
int32_t CNX_File::Copy( const char *pSrc, const char *pDst )
{
	return CNX_File::Copy( pSrc, pDst, NULL, NULL );
}

//------------------------------------------------------------------------------
int32_t CNX_File::Copy( const char *pSrc, const char *pDst, void (*cbFunc)(int64_t, int64_t, void *pObj), void *pObj )
{
	int32_t hSrc, hDst;
	struct stat statinfo;
	char buf[1024 * 1024];
	int32_t iReadSize, iWriteSize;
	int64_t iWrittenSize = 0;

	if( 0 > (hSrc = open( pSrc, O_RDONLY)) )
	{
		printf("Fail, open(). ( src: %s )\n", pSrc);
		return -1;
	}

	if( 0 > fstat( hSrc, &statinfo ) )
	{
		return -1;
	}

	if( 0 > (hDst = open( pSrc, O_WRONLY | O_CREAT, statinfo.st_mode)) )
	{
		printf("Fail, open(). ( dst: %s )\n", pDst );
		return -1;
	}

	while( 0 < (iReadSize = read(hSrc, buf, sizeof(buf))) )
	{
		iWriteSize = write( hDst, buf, iReadSize );
		if( iReadSize != iWriteSize )
		{
			printf("Fail, write(). ( readsize: %d, writesize: %d )\n", iReadSize, iWriteSize);
			goto ERROR;
		}

		iWrittenSize += iWriteSize;

		if( cbFunc )
			cbFunc( iWrittenSize, statinfo.st_size, pObj );
	}

	fchown( hDst, statinfo.st_uid, statinfo.st_gid );

	if( hSrc ) close( hSrc );
	if( hDst ) close( hDst );

	sync();
	return 0;

ERROR:
	if( hSrc ) close( hSrc );
	if( hDst ) close( hDst );

	CNX_File::Remove( pDst );
	return -1;
}

//------------------------------------------------------------------------------
int32_t CNX_File::Move( const char *pSrc, const char *pDst )
{
	if( 0 > rename( pSrc, pDst ) )
	{
		printf("Fail, rename(). ( %s )\n", strerror(errno));
		return -1;
	}

	sync();
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_File::Remove( const char *pFile )
{
	if( 0 > remove( pFile ) )
	{
		printf("Fail, remove(). ( %s )\n", strerror(errno));
		return -1;
	}

	sync();
	return 0;
}

//------------------------------------------------------------------------------
static int32_t GetBracketNum( const char *pData )
{
	char *pPtr = (char*)pData;

	int32_t iBraketOpen = 0;
	int32_t iBraketClose = 0;

	for( int32_t i = 0; i < (int32_t)strlen(pData); i++ )
	{
		if( *pPtr == '(' && (*(pPtr-1) != '\\' || pPtr == pData) )
		{
			iBraketOpen++;
		}

		if( *pPtr == ')' && *(pPtr-1) != '\\' )
		{
			iBraketClose++;
		}

		pPtr++;
	}

	return (iBraketOpen == iBraketClose) ? iBraketOpen : -1;
}

//------------------------------------------------------------------------------
int32_t CNX_File::GetList( const char *pDir, const char *pPattern, char *pList[MAX_LIST_NUM], int32_t *iListNum )
{
	DIR *hDir;
	struct dirent *pDirent;

	int32_t iCount = 0;

	int32_t iRet;
	char szErr[128];
	regex_t hRegex;
	regmatch_t* pRegmatch = NULL;

#if NX_ENABLE_REGEX_PARSE
	int32_t iBraketNum = GetBracketNum( pPattern );
#else
	int32_t iBraketNum = -1;
#endif

	for( int32_t i = 0; i < MAX_LIST_NUM; i++ )
		pList[i] = NULL;

	if( 0 < iBraketNum )
	{
		pRegmatch = (regmatch_t*)malloc( sizeof(regmatch_t) * iBraketNum );
	}

	iRet = regcomp( &hRegex, pPattern, REG_EXTENDED );
	if( 0 != iRet )
	{
		regerror( iRet, &hRegex, szErr, sizeof(szErr) );
		printf("Fail, regcomp(). ( %s )\n", szErr);
		return -1;
	}

	if( NULL == (hDir = opendir( pDir )) )
	{
		return -1;
	}

	while( NULL != (pDirent = readdir(hDir)) )
	{
		if( !strcmp( pDirent->d_name, "." ) || !strcmp( pDirent->d_name, ".." ) )
			continue;

		iRet = regexec( &hRegex, pDirent->d_name, pRegmatch ? iBraketNum : 0, pRegmatch ? pRegmatch : NULL, 0 );
		if( 0 == iRet )
		{
			char szTemp[512];
			sprintf( szTemp, "%s/%s", pDir, pDirent->d_name );
			pList[iCount] = strdup( szTemp );
			iCount++;

			//	index 0		: full match
			//	index 1..	: match of bracket in regular expression
			//
			if( pRegmatch )
			{
				printf(">>> %s\n", pDirent->d_name);
				for( int32_t i = 0; i < iBraketNum; i++ )
				{
					char szToken[64] = { 0x00, };
					memcpy( szToken, pDirent->d_name + pRegmatch[i].rm_so, pRegmatch[i].rm_eo - pRegmatch[i].rm_so);
					printf("index %d [ %3lld : %3lld ] : %s\n", i, (int64_t)pRegmatch[i].rm_so, (int64_t)pRegmatch[i].rm_eo, szToken);
				}
			}
		}
		else if( REG_NOMATCH == iRet )
		{
			// printf("No match. ( %s )\n", pDirent->d_name);
		}
		else
		{
			regerror( iRet, &hRegex, szErr, sizeof(szErr) );
			printf("Fail, regexec(). ( %s )\n", szErr);
		}
	}

	if( pRegmatch )
	{
		free( pRegmatch );
		pRegmatch = NULL;
	}

	regfree( &hRegex );
	closedir( hDir );

	*iListNum = iCount;
	return 0;
}

//------------------------------------------------------------------------------
int32_t CNX_File::FreeList( char *pList[MAX_LIST_NUM], int32_t iListNum )
{
	for( int32_t i = 0; i < iListNum; i++ )
	{
		if( pList[i] )
		{
			free( pList[i] );
			pList[i] = NULL;
		}
	}
	return 0;
}
