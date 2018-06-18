/*
Module : NTRAY.H
Purpose: Interface for a MFC class to encapsulate Shell_NotifyIcon
Created: PJN / NOTSHELL/1 / 14-05-1997
History: None

  Copyright (c) 1997 by PJ Naughter.  
  All rights reserved.
  
*/


////////////////////////////////// Macros ///////////////////////////

#ifndef _NTRAY_H__
#define _NTRAY_H__

/////////////////////////// Classes /////////////////////////////////

//forward declaration
class CTrayNotifyIcon;


//internal class used to handle IE4 taskbar creation notification message
class CTrayRessurectionWnd : public CFrameWnd
{
public: 
	//Constructors / Destructors
	CTrayRessurectionWnd(CTrayNotifyIcon* pTrayIcon);
	
protected:
	CTrayNotifyIcon* m_pTrayIcon;
	
	//{{AFX_MSG(CTrayRessurectionWnd)
	//}}AFX_MSG
	afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
		
		DECLARE_DYNAMIC(CTrayRessurectionWnd)
};



//internal class used to implement animated tray icons
class CTrayTimerWnd : public CFrameWnd
{
public: 
	//Constructors / Destructors
	CTrayTimerWnd(CTrayNotifyIcon* pTrayIcon, HICON* phIcons, int nNumIcons, DWORD dwDelay);
	~CTrayTimerWnd();
	
	//retreive the current icon
	HICON GetCurrentIcon() const { return m_phIcons[m_nCurrentIconIndex]; };
	
protected:
	CTrayNotifyIcon* m_pTrayIcon;
	HICON*           m_phIcons;
	int              m_nNumIcons;
	DWORD            m_dwDelay;
	UINT             m_nTimerID;
	int              m_nCurrentIconIndex;
	
	//{{AFX_MSG(CTrayTimerWnd)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	//}}AFX_MSG
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg LRESULT OnTaskbarCreated(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
		
		DECLARE_DYNAMIC(CTrayTimerWnd)
};


//the actual tray notification class wrapper
class CTrayNotifyIcon : public CObject
{
public:
	//Constructors / Destructors
	CTrayNotifyIcon();
	~CTrayNotifyIcon();
	
	//Create the tray icon
	BOOL Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON hIcon, UINT nNotifyMessage);                  
	BOOL Create(CWnd* pNotifyWnd, UINT uID, LPCTSTR pszTooltipText, HICON* phIcons, int nNumIcons, DWORD dwDelay, UINT nNotifyMessage);
	
	//Change or retrieve the Tooltip text
	BOOL    SetTooltipText(LPCTSTR pszTooltipText);
	BOOL    SetTooltipText(UINT nID);
	CString GetTooltipText() const;
	//Receive the gived Menu;
	
	//Change or retrieve the icon displayed
	BOOL  SetIcon(HICON hIcon);
	BOOL  SetIcon(LPCTSTR lpIconName);
	BOOL  SetIcon(UINT nIDResource);
	BOOL  SetIcon(HICON* phIcons, int nNumIcons, DWORD dwDelay);
	BOOL  SetStandardIcon(LPCTSTR lpIconName);
	BOOL  SetStandardIcon(UINT nIDResource);
	HICON GetIcon() const;
	BOOL  UsingAnimatedIcon() const { return m_bAnimated; };
	
	//Change or retrieve the window to send notification messages to
	BOOL  SetNotificationWnd(CWnd* pNotifyWnd);
	CWnd* GetNotificationWnd() const;
	
	//Modification of the tray icons
	void  HideIcon();
	void  ShowIcon();
	void  RemoveIcon();
	void  MoveToExtremeRight();
	
	//Default handler for tray notification message
	virtual LRESULT OnTrayNotification(WPARAM uID, LPARAM lEvent);
	
	//Status information
	BOOL IsShowing() const { return !IsHidden(); };
	BOOL IsHidden() const { return m_bHidden; };
	UINT nToolBarID;

	CMenu * SFGetSubMenu(int nPos) { return m_menu.GetSubMenu(nPos); }
protected:
	BOOL CreateTimerWindow(HICON* phIcons, int nNumIcons, DWORD dwDelay);
	void DestroyTimerWindow();
	BOOL CreateRessurectionWindow();
	void DestroyResurrectionWindow();
	
	CMenu					m_menu;
	NOTIFYICONDATA			m_NotifyIconData;
	BOOL					m_bCreated;
	BOOL					m_bHidden;
	CWnd			*		m_pNotificationWnd;
	CTrayTimerWnd	*		m_pTimerWnd;
	CTrayRessurectionWnd *	m_pResurrectionWnd;
	BOOL					m_bAnimated;

	DECLARE_DYNAMIC(CTrayNotifyIcon)
		
		friend class CTrayTimerWnd;
};




#endif //_NTRAY_H__
