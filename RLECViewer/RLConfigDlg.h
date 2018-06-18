#if !defined(AFX_RLCONFIGDLG_H__72221789_F4A1_4124_882A_540D081555BA__INCLUDED_)
#define AFX_RLCONFIGDLG_H__72221789_F4A1_4124_882A_540D081555BA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RLConfigDlg.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CRLConfigDlg dialog

class CRLConfigDlg : public CDialog
{
// Construction
public:
	CRLConfigDlg(CWnd* pParent = NULL);   // standard constructor
	RLEC_CONFIG		m_stConfig;

	int				m_nFanCount;
	void			RLUpdateFanDuty();

// Dialog Data
	//{{AFX_DATA(CRLConfigDlg)
	enum { IDD = IDD_DIALOG_CONFIG };
	UINT	m_nTempLimit;
	BOOL	m_bAutoFanCount;
	UINT	m_nFanCountManual;
	BOOL	m_bCoolOverLevel;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRLConfigDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CRLConfigDlg)
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	virtual void OnCancel();
	afx_msg void OnRadioModeauto();
	afx_msg void OnRadioModemanule();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnBtnDefault();
	afx_msg void OnCheckAutofancount();
	afx_msg void OnChangeEditFancount();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_RLCONFIGDLG_H__72221789_F4A1_4124_882A_540D081555BA__INCLUDED_)
