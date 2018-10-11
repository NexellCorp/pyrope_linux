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

#include "stdafx.h"
#include "NX_I2CControl.h"

//------------------------------------------------------------------------------
static BOOL Execute( CString &szResult, LPCTSTR szFormat, ... )
{
	SECURITY_ATTRIBUTES securityAttr;
	ZeroMemory( &securityAttr, sizeof(SECURITY_ATTRIBUTES) );
	securityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	securityAttr.bInheritHandle = TRUE;
	securityAttr.lpSecurityDescriptor = NULL;

	HANDLE hReadPipe, hWritePipe;
	CreatePipe( &hReadPipe, &hWritePipe, &securityAttr, 0 );

	STARTUPINFO startupInfo;
	PROCESS_INFORMATION processInfo;
	ZeroMemory( &startupInfo, sizeof(STARTUPINFO) );
	ZeroMemory( &processInfo, sizeof(PROCESS_INFORMATION) );

	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	startupInfo.wShowWindow = SW_HIDE;

	startupInfo.hStdOutput = hWritePipe;
	startupInfo.hStdError = hWritePipe;

	CString szTemp;
	va_list args;
	va_start( args, szFormat );
	szTemp.FormatV( szFormat, args );
	va_end( args );

	TCHAR tcPath[MAX_PATH];
	GetModuleFileName( NULL, tcPath, MAX_PATH );

	CString szCommand;
	szCommand.Format( _T("%s"), tcPath );
	szCommand = szCommand.Left( szCommand.ReverseFind('\\') );
	szCommand += L"\\bin\\";
	szCommand += szTemp;

	int iRet = CreateProcess( NULL, (TCHAR*)(LPCTSTR)szCommand, NULL, NULL, TRUE, 0, NULL, NULL, &startupInfo, &processInfo );
	if( FALSE == iRet )
	{
		return FALSE;
	}

	if( szResult )
	{
		szResult = _T("");
	}
	
	do {
		CHAR result[4096] = {0x00, };
		DWORD dwTotalByteAvail = 0, dwRead = 0, dwLeftThisMessage = 0;

		BOOL bSuccess = PeekNamedPipe( hReadPipe, result, 1, &dwRead, &dwTotalByteAvail, &dwLeftThisMessage);
		if( TRUE == bSuccess )
		{
			if( 0 < dwRead )
			{
				bSuccess = ReadFile( hReadPipe, result, sizeof(result) - 1, &dwRead, NULL );
				if( TRUE == bSuccess )
				{
					result[dwRead] = NULL;

					CString szTemp(result);
					if( szResult )
					{
						szResult += szTemp;
					}
				}
			}
		}
	} while( WAIT_OBJECT_0 != WaitForSingleObject(processInfo.hProcess, 0) );

	CloseHandle( processInfo.hThread );
	CloseHandle( processInfo.hProcess );

	CloseHandle( hReadPipe );
	CloseHandle( hWritePipe );

	return TRUE;
}

//------------------------------------------------------------------------------
static BOOL Execute( LPCTSTR szFormat, ... )
{
	CString szCommand;
	CString szResult;

	va_list args;
	va_start( args, szFormat );
	szCommand.FormatV( szFormat, args );
	va_end( args );

	return Execute( szResult, szCommand );
}

//------------------------------------------------------------------------------
static BOOL MakeScript( CString &szFile, LPCTSTR szFormat, ... )
{
	CString szCommand;

	va_list args;
	va_start( args, szFormat );
	szCommand.FormatV( szFormat, args );
	va_end( args );

	TCHAR tcPath[MAX_PATH];
	GetModuleFileName( NULL, tcPath, MAX_PATH );

	CString szTemp;
	szTemp.Format( _T("%s"), tcPath );
	szTemp = szTemp.Left( szTemp.ReverseFind('\\') );
	szTemp += L"\\bin\\";
	szTemp += NX_I2C_SCRIPT_NAME;

	if( szFile )
		szFile = szTemp;

	CFile hFile;
	if( !hFile.Open(szTemp, CFile::modeCreate | CFile::modeWrite ) )
	{
		return FALSE;
	}

	CString szData = _T("");
	szData += _T("#!/system/bin/sh\n");
	szData += szCommand;
	szData += _T("\n");

	hFile.Write( szData, szData.GetLength()*sizeof(TCHAR) );
	hFile.Close();

	return TRUE;
}

//------------------------------------------------------------------------------
static BOOL IsError( CString szData )
{
	CString szResult = szData;
	szResult.Trim();
	return !szResult.Find( _T("error: device not found") );
}

//------------------------------------------------------------------------------
static int32_t GetResult( CString szData )
{
	CString szResult, szLine;

	int iLIneCount = 0;
	do {
		if( !AfxExtractSubString( szLine, szData, iLIneCount++, '\n' ) )
			break;
	
		szLine.Trim();
		if( !szLine.Compare( _T("") ) )
			continue;

		szResult = szLine;
	} while( TRUE );

	return _tstoi(szResult);
}

//------------------------------------------------------------------------------
BOOL NX_I2CConnect()
{
	BOOL bConnected = FALSE;
	CString szResult;

	if( FALSE == Execute( _T("adb.exe start-server") ) )
		return FALSE;

	if( FALSE == Execute( szResult, _T("adb.exe devices") ) )
		return FALSE;

	int iLIneCount = 0;
	do {
		CString szLine;
		CString szInfo[2];
		if( !AfxExtractSubString( szLine, szResult, iLIneCount++, '\n' ) )
			break;
	
		szLine.Trim();

		if( !szLine.Compare( _T("") ) || 0 <= szLine.Find( _T("List of devices attached") ) )
			continue;

		int iTokenCount = 0;
		do {
			CString szTemp;
			if( !AfxExtractSubString( szTemp, szLine, iTokenCount, '\t' ) )
				break;

			szTemp.Trim();
			szInfo[iTokenCount] = szTemp;

			iTokenCount++;
		} while( TRUE );

		if( !szInfo[0].Compare( NX_I2C_DEV ) && !szInfo[1].Compare( _T("device" ) ) )
		{
			bConnected = TRUE;
			break;
		}
	} while( TRUE );

	if( FALSE == bConnected )
		return FALSE;

	// get root permission
	if( FALSE == Execute( szResult, _T("adb.exe -s %s root"), NX_I2C_DEV ) )
		return FALSE;

	if( IsError(szResult) )
		return FALSE;
	
	return TRUE;
}

//------------------------------------------------------------------------------
int32_t NX_I2CWrite( int32_t iPort, int32_t iSlave, uint16_t iAddr, uint16_t iBuf )
{
	CString szResult;

#if 1
	//
	// shell command base
	//
	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"nx_i2c -d -p %d -s 0x%02x -r 0x%04x -w 0x%04x\""), NX_I2C_DEV, iPort, iSlave, iAddr, iBuf ) )
	{
		return -1;
	}
#else
	//
	// script base
	//
	CString szFile;
	if( FALSE == MakeScript( szFile, _T("nx_i2c -d -p %d -s 0x%02x -r 0x%04x -w 0x%04x"), iPort, iSlave, iAddr, iBuf ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s push %s %s/%s"), NX_I2C_DEV, szFile, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell sync"), NX_I2C_DEV) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"chmod 777 %s/%s\""), NX_I2C_DEV, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"%s/%s\""), NX_I2C_DEV, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}
#endif

	return (0 > GetResult(szResult) || IsError(szResult)) ? -1 : 0;
}

//------------------------------------------------------------------------------
int32_t NX_I2CWrite( int32_t iPort, int32_t iSlave, uint16_t iAddr, uint16_t *pBuf, int32_t iSize )
{
	CString szResult;

	if( pBuf == NULL || iSize == 0 )
	{
		return -1;
	}

	CString szData;
	szData.Format( _T("0x%04x"), pBuf[0] );
	for( int32_t i = 1; i < iSize; i++ )
	{
		szData.AppendFormat( _T(",0x%04x"), pBuf[i] );
	}

#if 0
	//
	// shell command base
	//
	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"nx_i2c -d -p %d -s 0x%02x -r 0x%04x -w %s\""), NX_I2C_DEV, iPort, iSlave, iAddr, szData ) )
	{
		return -1;
	}

	return (0 > GetResult(szResult)) ? -1 : 0;
#else
	//
	// script base
	//
	CString szFile;
	if( FALSE == MakeScript( szFile, _T("nx_i2c -d -p %d -s 0x%02x -r 0x%04x -w %s"), iPort, iSlave, iAddr, szData ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s push %s %s/%s"), NX_I2C_DEV, szFile, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell sync"), NX_I2C_DEV) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"chmod 777 %s/%s\""), NX_I2C_DEV, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"%s/%s\""), NX_I2C_DEV, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}
#endif

	return (0 > GetResult(szResult) || IsError(szResult)) ? -1 : 0;
}

//------------------------------------------------------------------------------
int32_t NX_I2CRead( int32_t iPort, int32_t iSlave, uint16_t iAddr )
{
	CString szResult;

#if 1
	//
	// shell command base
	//
	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"nx_i2c -d -p %d -s 0x%02x -r 0x%04x\""), NX_I2C_DEV, iPort, iSlave, iAddr ) )
	{
		return -1;
	}
#else
	//
	// script base
	//
	CString szFile;

	if( FALSE == MakeScript( szFile, _T("nx_i2c -d -p %d -s 0x%02x -r 0x%04x"), iPort, iSlave, iAddr ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s push %s %s/%s"), NX_I2C_DEV, szFile, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell sync"), NX_I2C_DEV) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"chmod 777 %s/%s\""), NX_I2C_DEV, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}

	if( FALSE == Execute( szResult, _T("adb.exe -s %s shell \"%s/%s\""), NX_I2C_DEV, NX_I2C_SCRIPT_PATH, NX_I2C_SCRIPT_NAME ) )
	{
		return -1;
	}
#endif

	return IsError(szResult) ? -1 : GetResult(szResult);
}
