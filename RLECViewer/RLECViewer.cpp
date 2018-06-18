// RLECViewer.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "RLECViewer.h"
#include "RLECViewerDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CString RLGetExePath()
{
	char pathbuf[5000] = {0};
	int pathlen = ::GetModuleFileName(NULL,pathbuf,5000);
	
	while(TRUE)
	{
		if (pathbuf[pathlen--]=='\\')
		{
			break;
		}
	}
	pathbuf[++pathlen] = 0x0;
	CString fname = pathbuf;
	return fname;
}


/////////////////////////////////////////////////////////////////////////////
// CRLECViewerApp

BEGIN_MESSAGE_MAP(CRLECViewerApp, CWinApp)
	//{{AFX_MSG_MAP(CRLECViewerApp)
		// NOTE - the ClassWizard will add and remove mapping macros here.
		//    DO NOT EDIT what you see in these blocks of generated code!
	//}}AFX_MSG
	ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRLECViewerApp construction

CRLECViewerApp::CRLECViewerApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CRLECViewerApp object

CRLECViewerApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CRLECViewerApp initialization

BOOL CRLECViewerApp::InitInstance()
{
	HWND hwndFind = FindWindow(NULL, "RLECViewer");
	if (hwndFind != NULL)
	{
		ShowWindow(hwndFind, SW_RESTORE);
		SetForegroundWindow(hwndFind);
		return FALSE;
	}

	AfxEnableControlContainer();
	SetDialogBkColor(RGB(68,68,68), RGB(192,192,192));
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	//  of your final executable, you should remove from the following
	//  the specific initialization routines you do not need.

#ifdef _AFXDLL
	Enable3dControls();			// Call this when using MFC in a shared DLL
#else
	Enable3dControlsStatic();	// Call this when linking to MFC statically
#endif

	CRLECViewerDlg dlg;
	m_pMainWnd = &dlg;
	int nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return FALSE;
}
