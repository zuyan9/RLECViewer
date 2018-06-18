// RLECViewerDlg.h : header file
//

#if !defined(AFX_RLECVIEWERDLG_H__1B131CFC_18CA_471C_A807_94718F6A300D__INCLUDED_)
#define AFX_RLECVIEWERDLG_H__1B131CFC_18CA_471C_A807_94718F6A300D__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "NTRAY.H"
#include "NewMenu.h"
#include "KHMutexDefine.h"
/////////////////////////////////////////////////////////////////////////////
// CRLECViewerDlg dialog

class CRLECViewerDlg : public CDialog
{
// Construction
public:
	CRLECViewerDlg(CWnd* pParent = NULL);	// standard constructor
	RLEC_STATUS			m_stFanInfo;

// Dialog Data
	//{{AFX_DATA(CRLECViewerDlg)
	enum { IDD = IDD_RLECVIEWER_DIALOG };
	BOOL	m_bAutoRun;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRLECViewerDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

	BOOL	RLInit();
	void	RLUnInit();

	BOOL	RLInit2();
	void	RLUnInit2();


private:
	BOOL					RLECProc();
	BOOL					RLECProc2();
	static DWORD WINAPI		RLECThread(LPVOID lParam);
	
	BOOL				SFReadAutoRunFromeReg();
	BOOL				SFSaveToReg(BOOL bRun);
	BOOL				SFSaveINF();
	BOOL				SFLoadINF(BOOL bWriteDefault);

	int					RLCalcManualFanDuty(int nFanIdx);

// Implementation
protected:
	HICON m_hIcon;
	BOOL				m_bExit;
	HANDLE				m_hThread;
	HINSTANCE			m_hInstDLL;
	SetFanDuty		*	m_pfnSetFanDuty;
	SetFANDutyAuto	*	m_pfnSetFANDutyAuto;
	GetTempFanDuty	*	m_pfnGetTempFanDuty;
	GetFANCounter	*	m_pfnGetFANCounter;
	GetECVersion	*	m_pfnGetECVersion;
	GetCPUFANRPM	*	m_pfnGetCPUFANRPM;
	GetGPUFANRPM	*	m_pfnGetGPUFANRPM;
	GetGPU1FANRPM	*	m_pfnGetGPU1FANRPM;
	GetX72FANRPM	*	m_pfnGetX72FANRPM;

	int					m_nStatus;
	RLEC_CONFIG			m_stAllInfo;

	CNewMenu			m_TrayMenu;
	CTrayNotifyIcon		m_TrayIcon;
	static	WORD		sTrayTool[];

	InitIo			*	m_pfnInitIo;
	HINSTANCE			m_hInstDLL2;
	GetTempFanDuty2	*	m_pfnGetTempFanDuty2;

	int					m_nLastFanTemp[RL_MAX_FAN];			// 记录最后一次温度
	int					m_nCurFanIdx[RL_MAX_FAN];			// 当前风扇转速索引
	int					m_nTargetFanIdx[RL_MAX_FAN];		// 目标风扇转速索引

	// Generated message map functions
	//{{AFX_MSG(CRLECViewerDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnReset();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnCheckAutorun();
	afx_msg void OnRestore();
	afx_msg void OnAbout();
	afx_msg void OnBtnConfig();
	//}}AFX_MSG
    afx_msg LRESULT OnTrayNotification(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RLECVIEWERDLG_H__1B131CFC_18CA_471C_A807_94718F6A300D__INCLUDED_)
