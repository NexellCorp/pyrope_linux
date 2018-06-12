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

#ifndef __CNX_PREFERENCE_H__
#define __CNX_PREFERENCE_H__

#include <stdio.h>
#include <libxml/parser.h>
#include <libxml/xmlmemory.h>

#include <CNX_Base.h>
#include <INX_Preference.h>

class CNX_Preference
	: public INX_Preference
{
public:
	CNX_Preference();
	~CNX_Preference();

public:
	int32_t Open( const char *pFile );
	void	Close( void );

	int32_t Write( const char *pKey, char *pValue );
	int32_t Read( const char *pKey, char **ppValue );
	int32_t Remove( const char *pKey );

	void	Dump( void );

private:
	CNX_Mutex		m_hLock;
	char*			m_pFile;

	xmlDocPtr		m_hDoc;
	xmlNodePtr		m_hRoot;

	xmlChar*		m_pValue;
	xmlChar*		m_pResult;

private:
	CNX_Preference (const CNX_Preference &Ref);
	CNX_Preference &operator=(const CNX_Preference &Ref);
};

#endif	// __CNX_PREFERENCE_H__
