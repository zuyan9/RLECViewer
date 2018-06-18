/*
Module : NTRAY.CPP
Purpose: implementation for a MFC class to encapsulate Shell_NotifyIcon
Created: PJN / NOTSHELL/1 / 14-05-1997
History: PJN / 25-11-1997 : Addition of the following
1. HideIcon(), ShowIcon() & MoveToExtremeRight 
2. Support for animated tray icons
PJN / 23-06-1998 : Class now supports the new Taskbar Creation Notification 
message which comes with IE 4. This allows the tray icon
to be recreated whenever the explorer restarts (Crashes!!)
PJN / 22-07-1998 : 1. Code now compiles cleanly at warning level 4
2. Code is now UNICODE enabled + build configurations are 
provided
3. The documentation for the class has been updated

  Copyright (c) 1997 by PJ Naughter.  
  All rights reserved.
  
*/

/////////////////////////////////  Includes  //////////////////////////////////
#include "stdafx.h"
#include "ntray.h"
#include "RLECViewer.h"

#include "RLECViewerDlg.h"


/////////////////////////////////  Macros /////////////////////////////////////

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


///////////////////////////////// Implementation //////////////////////////////

const UINT wm_TaskbarCreated = RegisterWindowMessage(_T("TaskbarCreated"));

IMPLEMENT_DYNAMIC(CTrayRessurectionWnd, CFrameWnd)

BEGIN_MESSAGE_MAP(CTrayRessurectionWnd, CFrameWnd)
//{{AFX_MSG_MAP(CTrayRessurectionWnd)
//}}AFX_MSG_MAP
ON_REGISTERED_MESSAGE(wm_TaskbarCreated, OnTaskbarCreated)
END_MESSAGE_MAP()

LRESULT CTrayRessurectionWnd::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
	ASSERT(m_pTrayIcon);
	
	//Refresh the tray icon if necessary
	if (m_pTrayIcon->IsShowing())
	{
		m_pTrayIcon->HideIcon();
		m_pTrayIcon->ShowIcon();
	}
	
	return 0L;
}

CTrayRessurectionWnd::CTrayRessurectionWnd(CTrayNotifyIcon* pTrayIcon)
{
	//must have at valid tray notify instance
	ASSERT(pTrayIcon);
	
	//Store the values away
	m_pTrayIcon = pTrayIcon;
}

IMPLEMENT_DYNAMIC(CTrayTimerWnd, CFrameWnd)

BEGIN_MESSAGE_MAP(CTrayTimerWnd, CFrameWnd)
//{{AFX_MSG_MAP(CTrayTimerWnd)
ON_WM_CREATE()
ON_WM_DESTROY()
ON_WM_TIMER()
//}}AFX_MSG_MAP
END_MESSAGE_MAP()

CTrayTimerWnd::CTrayTimerWnd(CTrayNotifyIcon* pTrayIcon, HICON* phIcons, int nNumIcons, DWORD dwDelay)
{
	m_nCurrentIconIndex = 0;
	
	//must have a valid tray notify instance
	ASSERT(pTrayIcon);
	
	//must have at least 1 icon
	ASSERT(nNumIcons);
	
	//array of icon handles must be valid
	ASSERT(phIcons);
	
	//must be non zero timer interval
	ASSERT(dwDelay);
	
	//Store the values away
	m_pTrayIcon = pTrayIcon;
	
	m_phIcons = new HICON[nNumIcons];
	CopyMemory(m_phIcons, phIcons, nNumIcons * sizeof(HICON));
	m_nNumIcons = nNumIcons;
	m_dwDelay = dwDelay;
}

CTrayTimerWnd::~CTrayTimerWnd()
{
	if (m_phIcons)
	{
		delete [] m_phIcons;
		m_phIcons = NULL;
	}
}

int CTrayTimerWnd::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	
	//create the animation timer
	m_nTimerID = SetTimer(1, m_dwDelay, NULL);
	
	return 0;
}

void CTrayTimerWnd::OnDestroy() 
{
	//kill the animation timer
	KillTimer(m_nTimerID);
	
	CFrameWnd::OnDestroy();
}

void CTrayTimerWnd::OnTimer(UINT nIDEvent) 
{
	if(nIDEvent == 1)
	{
		//increment the icon index
		++m_nCurrentIconIndex;
		m_nCurrentIconIndex = m_nCurrentIconIndex % m_nNumIcons;
		
		//update the tray icon
		m_pTrayIcon->m_NotifyIconData.uFlags = NIF_ICON;
		m_pTrayIcon->m_NotifyIconData.hIcon = m_phIcons[m_nCurrentIconIndex];
		Shell_NotifyIcon(NIM_MODIFY, &m_pTrayIcon->m_NotifyIconData);
		
		//--------------------为本程序而改
		// Modified for this program
		if(m_nCurrentIconIndex)
		{
			KillTimer(1);
			SetTimer(2, 600, NULL);
		}
	}
	
	if(nIDEvent == 2)
	{
		m_pTrayIcon->m_NotifyIconData.hIcon = m_phIcons[m_nCurrentIconIndex - 1];
		Shell_NotifyIcon(NIM_MODIFY, &m_pTrayIcon->m_NotifyIconData);
		KillTimer(2);
		SetTimer(1, m_dwDelay, NULL);
	}
	
}

IMPLEMENT_DYNAMIC(CTrayNotifyIcon, CObject)

CTrayNotifyIcon::CTrayNotifyIcon()
{
	memset(&m_NotifyIconData, 0, sizeof(m_NotifyIconData));
	m_bCreated = FALSE;
	m_bHidden = FALSE;
	m_pNotificationWnd = NULL;
	m_pResurrectionWnd = NULL;
	m_pTimerWnd = NULL;
	m_bAnimated = FALSE;
	}

CTrayNotifyIcon::~CTrayNotifyIcon()
{
	DestroyTimerWindow();
	DestroyResurrectionWindow();
	RemoveIcon();

	if(m_pTimerWnd)
	{
		delete m_pTimerWnd;
		m_pTimerWnd = NULL;
	}

	if(m_pResurrectionWnd)
	{
		delete m_pResurrectionWnd;
		m_pResurrectionWnd = NULL;
	}
}

void CTrayNotifyIcon::HideIcon()
{
	ASSERT(m_bCreated);
	if (!m_bHidden) 
	{
		m_NotifyIconData.uFlags = NIF_ICON;
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
		m_bHidden = TRUE;
	}
}

void CTrayNotifyIcon::ShowIcon()
{
	ASSERT(m_bCreated);
	if (m_bHidden) 
	{
		m_NotifyIconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
		Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
		m_bHidden = FALSE;
	}
}

void CTrayNotifyIcon::RemoveIcon()
{
	if (m_bCreated)
	{
		m_NotifyIconData.uFlags = 0;
		Shell_NotifyIcon(NIM_DELETE, &m_NotifyIconData);
		m_bCreated = FALSE;
	}
}

void CTrayNotifyIcon::MoveToExtremeRight()
{
	HideIcon();
	ShowIcon();
}

BOOL CTrayNotifyIcon::Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON hIcon, UINT nNotifyMessage)
{
	//Create the ressurection window
	if (!CreateRessurectionWindow())
		return FALSE;
	
	//Make sure Notification window is valid
	ASSERT(pNotifyWnd && ::IsWindow(pNotifyWnd->GetSafeHwnd()));
	m_pNotificationWnd = pNotifyWnd;
	
	//Make sure we avoid conflict with other messages
	ASSERT(nNotifyMessage >= WM_USER);
	
	//Tray only supports tooltip text up to 64 characters
	ASSERT(_tcslen(pszTooltipText) <= 64);
	
	m_NotifyIconData.cbSize = sizeof(m_NotifyIconData);
	m_NotifyIconData.hWnd = pNotifyWnd->GetSafeHwnd();
	m_NotifyIconData.uID = uID;
	m_NotifyIconData.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_NotifyIconData.uCallbackMessage = nNotifyMessage;
	m_NotifyIconData.hIcon = hIcon;
	_tcscpy(m_NotifyIconData.szTip, pszTooltipText);
	
	BOOL rVal = Shell_NotifyIcon(NIM_ADD, &m_NotifyIconData);
	m_bCreated = rVal;
	if( uID > 0 )
	{
		m_menu.DestroyMenu();
		m_menu.LoadMenu(uID);
	}
	return rVal;
}

BOOL CTrayNotifyIcon::CreateRessurectionWindow()
{
	//Create the resurrection window 
	ASSERT(m_pResurrectionWnd == NULL);
	m_pResurrectionWnd = new CTrayRessurectionWnd(this);
	if (!m_pResurrectionWnd)
	{
		return FALSE;
	}
	if (!m_pResurrectionWnd->Create(NULL, _T("CTrayNotifyIcon Resurrection Notification Window")))
	{
		return FALSE;
	}
	return TRUE;
}

void CTrayNotifyIcon::DestroyResurrectionWindow()
{
	if (m_pResurrectionWnd != NULL)
	{
		m_pResurrectionWnd->SendMessage(WM_CLOSE);
		m_pResurrectionWnd = NULL;
	}
}

BOOL CTrayNotifyIcon::CreateTimerWindow(HICON* phIcons, int nNumIcons, DWORD dwDelay)
{
	//create the hidden window which will contain the timer which will do the animation
	ASSERT(m_pTimerWnd == NULL); 

	m_pTimerWnd = new CTrayTimerWnd(this, phIcons, nNumIcons, dwDelay);
	if (!m_pTimerWnd)
	{
		return FALSE;
	}
	if (!m_pTimerWnd->Create(NULL, _T("CTrayNotifyIcon Animation Notification Window")))
	{
		return FALSE;
	}
	return TRUE;
}

void CTrayNotifyIcon::DestroyTimerWindow()
{
	if (m_pTimerWnd != NULL)
	{
		m_pTimerWnd->SendMessage(WM_CLOSE);
		m_pTimerWnd = NULL;
	}
}

BOOL CTrayNotifyIcon::Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage)
{
	//must be using at least 2 icons
	ASSERT(nNumIcons >= 2);
	ASSERT(pNotifyWnd != NULL);

	if (!CreateTimerWindow(phIcons, nNumIcons, dwDelay))
	{
		return FALSE;
	}
	//let the normal Create function do its stuff
	BOOL bSuccess = Create(pNotifyWnd, uID, pszTooltipText, phIcons[0], nNotifyMessage);
	m_bAnimated = TRUE;
	CRect rect;
	GetClientRect(pNotifyWnd->m_hWnd, &rect);
	if( uID > 0 )
	{
		m_menu.DestroyMenu();
		m_menu.LoadMenu(uID);
	}
	return bSuccess;
}

BOOL CTrayNotifyIcon::SetTooltipText(LPCTSTR pszTooltipText)
{
	if (!m_bCreated)
	{
		return FALSE;
	}
	m_NotifyIconData.uFlags = NIF_TIP;
	_tcscpy(m_NotifyIconData.szTip, pszTooltipText);
	
	return Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}

BOOL CTrayNotifyIcon::SetTooltipText(UINT nID)
{
	CString sToolTipText;
	VERIFY(sToolTipText.LoadString(nID));
	
	return SetTooltipText(sToolTipText);
}

BOOL CTrayNotifyIcon::SetIcon(HICON hIcon)
{
	if (!m_bCreated)
		return FALSE;
	
	DestroyTimerWindow();
	m_bAnimated = FALSE;
	m_NotifyIconData.uFlags = NIF_ICON;
	m_NotifyIconData.hIcon = hIcon;
	
	return Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}

BOOL CTrayNotifyIcon::SetIcon(LPCTSTR lpIconName)
{
	HICON hIcon = AfxGetApp()->LoadIcon(lpIconName);
	
	return SetIcon(hIcon);
}

BOOL CTrayNotifyIcon::SetIcon(UINT nIDResource)
{
	HICON hIcon = AfxGetApp()->LoadIcon(nIDResource);
	
	return SetIcon(hIcon);
}

BOOL CTrayNotifyIcon::SetStandardIcon(LPCTSTR lpIconName)
{
	HICON hIcon = LoadIcon(NULL, lpIconName);
	
	return SetIcon(hIcon);
}

BOOL CTrayNotifyIcon::SetStandardIcon(UINT nIDResource)
{
	HICON hIcon = LoadIcon(NULL, MAKEINTRESOURCE(nIDResource));
	
	return SetIcon(hIcon);
}

BOOL CTrayNotifyIcon::SetIcon(HICON* phIcons, int nNumIcons, DWORD dwDelay)
{
	ASSERT(nNumIcons >= 2);
	ASSERT(phIcons);
	
	if (!SetIcon(phIcons[0]))
	{
		return FALSE;
	}
	DestroyTimerWindow();
	if (!CreateTimerWindow(phIcons, nNumIcons, dwDelay))
	{
		return FALSE;
	}
	m_bAnimated = TRUE;
	
	return TRUE;
}

BOOL CTrayNotifyIcon::SetNotificationWnd(CWnd* pNotifyWnd)
{
	if (!m_bCreated)
	{
		return FALSE;
	}
	//Make sure Notification window is valid
	ASSERT(pNotifyWnd && ::IsWindow(pNotifyWnd->GetSafeHwnd()));
	
	m_pNotificationWnd = pNotifyWnd;
	m_NotifyIconData.hWnd = pNotifyWnd->GetSafeHwnd();
	m_NotifyIconData.uFlags = 0;
	
	return Shell_NotifyIcon(NIM_MODIFY, &m_NotifyIconData);
}

CString CTrayNotifyIcon::GetTooltipText() const
{
	CString sText;
	if (m_bCreated)
	{
		sText = m_NotifyIconData.szTip;
	}
	return sText;
}

HICON CTrayNotifyIcon::GetIcon() const
{
	HICON hIcon = NULL;
	if (m_bCreated)
	{
		if (m_bAnimated)
			hIcon = m_pTimerWnd->GetCurrentIcon();
		else
			hIcon = m_NotifyIconData.hIcon;
	}
	
	return hIcon;
}

CWnd* CTrayNotifyIcon::GetNotificationWnd() const
{
	return m_pNotificationWnd;
}

LRESULT CTrayNotifyIcon::OnTrayNotification(WPARAM wID, LPARAM lEvent)
{
	//Return quickly if its not for this tray icon
	if (wID != m_NotifyIconData.uID)
		return 0L;
	
	//As a default action use a menu resource with the same id 
	//as this was created with
	
	if (lEvent == WM_RBUTTONUP)
	{
		CMenu *pSubMenu;
		pSubMenu = m_menu.GetSubMenu(0);

		if (!pSubMenu)
			return 0;
		
		//Clicking with right button brings up a context menu			
		CWnd* pTarget = AfxGetMainWnd();
		
		CPoint pos;
		GetCursorPos(&pos);
		
		pTarget->SetForegroundWindow();  
		if(pSubMenu)
		{
			pSubMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON,pos.x,pos.y, pTarget);
		}
		
		// BUGFIX: See "PRB: Menus for Notification Icons Don't Work Correctly"
		pTarget->PostMessage(WM_NULL, 0, 0);
	//	AfxGetMainWnd()->SendMessage(POPUP_TRAYMENU);
	} 
	else if (lEvent == WM_LBUTTONDBLCLK) 
	{
		// double click received, the default action is to execute first menu item
		if(AfxGetMainWnd()->IsWindowVisible())
		{
			AfxGetMainWnd()->ShowWindow(SW_SHOWMINIMIZED);
			AfxGetMainWnd()->ShowWindow(SW_HIDE);
		}
		else
		{
			AfxGetMainWnd()->ShowWindow(SW_RESTORE);
			AfxGetMainWnd()->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
			AfxGetMainWnd()->SetForegroundWindow(); 
			AfxGetMainWnd()->SetWindowPos(&CWnd::wndNoTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		}
	}
	return 1; // handled
}
