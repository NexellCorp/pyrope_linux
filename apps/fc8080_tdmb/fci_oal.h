/*****************************************************************************
 Copyright(c) 2009 FCI Inc. All Rights Reserved
 
 File name : fci_oal.h
 
 Description : OS Adatation Layer header
 
 History : 
 ----------------------------------------------------------------------
 2009/09/13 	jason		initial
*******************************************************************************/

#ifndef __FCI_OAL_H__
#define __FCI_OAL_H__

#ifdef __cplusplus
extern "C" {
#endif

extern void print_log(HANDLE handle, char *fmt, ...);
extern void ms_wait(s32 ms);

#ifdef __cplusplus
}
#endif

#endif		// __FCI_OAL_H__
