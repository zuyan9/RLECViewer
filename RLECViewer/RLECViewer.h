// RLECViewer.h : main header file for the RLECVIEWER application
//

#if !defined(AFX_RLECVIEWER_H__2C1C1730_87A8_4133_9302_2FE63E40500D__INCLUDED_)
#define AFX_RLECVIEWER_H__2C1C1730_87A8_4133_9302_2FE63E40500D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols
CString RLGetExePath();

#include "RLECViewerHeader.h"

#define RL_MAX_FAN		4
#define RL_FAN_LEVEL	10

#define WM_TRAYNOTIFY				(WM_USER + 2)


enum
{
	EC_STATUS_INIT = 1,
	EC_STATUS_INIT2 = 2,
	EC_STATUS_RUNNING = 3,
	EC_STATUS_RUNNING2 = 4,
};

typedef struct  
{
	int		nFanCount;
	int		nFanDuty[RL_MAX_FAN];
	int		nFanTempRemote[RL_MAX_FAN];
	int		nFanTempLocal[RL_MAX_FAN];
	string	strVer;
	
	int		nCpuFanRPM;
	int		nGpu1FanRPM;
	int		nGpu2FanRPM;
}RLEC_STATUS;


typedef struct  
{
	BOOL	bAutoMode;
	BOOL	bAutoFanCount;
	BOOL	bCoolOverLevel;			// 是否启用越级降温
	int		nTempLimit;
	int		nFanCountManual;		// 手动设置风扇数量
	int		nFanDutyArray[RL_MAX_FAN][RL_FAN_LEVEL];
}RLEC_CONFIG;

/////////////////////////////////////////////////////////////////////////////
// CRLECViewerApp:
// See RLECViewer.cpp for the implementation of this class
//

class CRLECViewerApp : public CWinApp
{
public:
	CRLECViewerApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRLECViewerApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CRLECViewerApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RLECVIEWER_H__2C1C1730_87A8_4133_9302_2FE63E40500D__INCLUDED_)
