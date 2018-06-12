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

#ifndef __INX_PREFERENCE_H__
#define __INX_PREFERENCE_H__

#include <stdint.h>

#define CINEMA_PREFERENCE_PATH		"/data/data/com.samsung.vd.cinemacontrolpanel/shared_prefs"
#define CINEMA_PREFERENCE_ACCOUNT	"/data/data/com.samsung.vd.cinemacontrolpanel/shared_prefs/cinema.account.xml"
#define CINEMA_PREFERENCE_CONFIG	"/data/data/com.samsung.vd.cinemacontrolpanel/shared_prefs/cinema.config.xml"

#define CINEMA_KEY_INTIAL_MODE		"initial.mode"
#define CINEMA_KEY_UPDATE_TGAM0		"update.tgam0"
#define CINEMA_KEY_UPDATE_TGAM1		"update.tgam1"
#define CINEMA_KEY_UPDATE_DGAM0		"update.dgam0"
#define CINEMA_KEY_UPDATE_DGAM1		"update.dgam1"

class INX_Preference
{
public:
	INX_Preference() {}
	virtual ~INX_Preference() {}

public:
	virtual int32_t Open( const char *pFile ) = 0;
	virtual void	Close( void ) = 0;

	virtual int32_t Write( const char *pKey, char *pValue ) = 0;
	virtual int32_t Read( const char *pKey, char **ppValue ) = 0;
	virtual int32_t Remove( const char *pKey ) = 0;

	virtual void	Dump( void ) = 0;

private:
	INX_Preference (const INX_Preference &Ref);
	INX_Preference &operator=(const INX_Preference &Ref);
};

extern INX_Preference* GetPreferenceHandle();

#endif	// __INX_PREFERENCE_H__
