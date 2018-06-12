//------------------------------------------------------------------------------
//
//	Copyright (C) 2017 Nexell Co. All Rights Reserved
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

#ifndef __CNX_FILE_H__
#define __CNX_FILE_H__

#include <stdint.h>

#define NX_ENABLE_CHECKSUM		0
#define NX_ENABLE_REGEX_PARSE	0

class CNX_File
{
public:
	CNX_File();
	CNX_File( const char *pFile );
	~CNX_File();

public:
	enum { MAX_LIST_NUM = 32 };

public:
	int32_t Open( const char *pFile );
	void	Close();

	int32_t IsExist();
	int32_t IsRegularFile();
	int32_t IsDirectory();

	int32_t GetSize();
	int32_t GetName( char **ppResult );
	int32_t GetDirname( char **ppResult );
	int32_t GetBasename( char **ppResult );
	int32_t GetExtension( char **ppResult );

	int32_t GetAccessDate( char **ppResult );
	int32_t GetModifyDate( char **ppResult );
	int32_t GetChangeDate( char **ppResult );
#if NX_ENABLE_CHECKSUM
	int32_t GetChecksum( char **ppResult );
#endif

public:
	static int32_t Copy( const char *pSrc, const char *pDst );
	static int32_t Copy( const char *pSrc, const char *pDst, void (*cbFunc)(int64_t, int64_t, void *pObj), void *pObj );
	static int32_t Move( const char *pSrc, const char *pDst );
	static int32_t Remove( const char *pFile );
	static int32_t GetList( const char *pDir, const char *pPattern, char *pList[MAX_LIST_NUM], int32_t *iListNum );
	static int32_t FreeList( char *pList[MAX_LIST_NUM], int32_t iListNum );

private:
	enum { MAX_DATE_NUM = 19+1 };

	char	*m_pFile;
	char	*m_pBasename;
	char	*m_pDirname;
	char	*m_pExtension;
	char	*m_pAccessDate;
	char	*m_pModifyDate;
	char	*m_pChangeDate;
	char	*m_pChecksum;

private:
	CNX_File (const CNX_File &Ref);
	CNX_File &operator=(const CNX_File &Ref);
};

#endif	// __CNX_FILE_H__
