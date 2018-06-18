// RLConfigDlg.cpp : implementation file
//

#include "stdafx.h"
#include "RLECViewer.h"
#include "RLECViewerDlg.h"
#include "RLConfigDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CRLConfigDlg dialog


CRLConfigDlg::CRLConfigDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRLConfigDlg::IDD, pParent)
	, m_nFanCount(0)
{
	//{{AFX_DATA_INIT(CRLConfigDlg)
	m_nTempLimit = 75;
	m_bAutoFanCount = FALSE;
	m_nFanCountManual = 2;
	m_bCoolOverLevel = FALSE;
	//}}AFX_DATA_INIT
	memset(&m_stConfig, 0, sizeof(m_stConfig));
}


void CRLConfigDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRLConfigDlg)
	DDX_Text(pDX, IDC_EDIT_TEMPLIMIT, m_nTempLimit);
	DDV_MinMaxUInt(pDX, m_nTempLimit, 50, 90);
	DDX_Check(pDX, IDC_CHECK_AUTOFANCOUNT, m_bAutoFanCount);
	DDX_Text(pDX, IDC_EDIT_FANCOUNT, m_nFanCountManual);
	DDV_MinMaxUInt(pDX, m_nFanCountManual, 0, 3);
	DDX_Check(pDX, IDC_CHECK_COOLOVERLEVEL, m_bCoolOverLevel);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CRLConfigDlg, CDialog)
	//{{AFX_MSG_MAP(CRLConfigDlg)
	ON_BN_CLICKED(IDC_RADIO_MODEAUTO, OnRadioModeauto)
	ON_BN_CLICKED(IDC_RADIO_MODEMANULE, OnRadioModemanule)
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_DEFAULT, OnBtnDefault)
	ON_BN_CLICKED(IDC_CHECK_AUTOFANCOUNT, OnCheckAutofancount)
	ON_EN_CHANGE(IDC_EDIT_FANCOUNT, OnChangeEditFancount)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRLConfigDlg message handlers

BOOL CRLConfigDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_nTempLimit = m_stConfig.nTempLimit;
	m_nFanCountManual = m_stConfig.nFanCountManual;
	m_bAutoFanCount = m_stConfig.bAutoFanCount;
	m_bCoolOverLevel = m_stConfig.bCoolOverLevel;
	if (m_stConfig.bAutoFanCount == FALSE)
		m_nFanCount = m_nFanCountManual;

	RLUpdateFanDuty();

	((CButton *)GetDlgItem(IDC_RADIO_MODEAUTO))->SetCheck(m_stConfig.bAutoMode);
	((CButton *)GetDlgItem(IDC_RADIO_MODEMANULE))->SetCheck(m_stConfig.bAutoMode == FALSE ? TRUE:FALSE);

	UpdateData(FALSE);
	UpdateData();

	SetTimer(2, 200, NULL);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CRLConfigDlg::OnOK() 
{
	// TODO: Add extra validation here
	if (UpdateData() == FALSE)
	{
		if (m_stConfig.bAutoMode || m_stConfig.bAutoFanCount)
			return;
	}

	m_stConfig.nTempLimit = m_nTempLimit;
	m_stConfig.bAutoFanCount = m_bAutoFanCount;
	m_stConfig.nFanCountManual = m_nFanCountManual;
	m_stConfig.bCoolOverLevel = m_bCoolOverLevel;

	int i = 0;
	CString strErr;
	for (i = IDC_EDIT_CPU0; i <= IDC_EDIT_CPU9; i++)
	{
		CString strDuty;
		GetDlgItem(i)->GetWindowText(strDuty);
		m_stConfig.nFanDutyArray[0][i - IDC_EDIT_CPU0] = atoi(strDuty);
		if (m_stConfig.nFanDutyArray[0][i - IDC_EDIT_CPU0] <= 10)
		{
			strErr.Format("Warning: [%d] CPU Fan speed low", i - IDC_EDIT_CPU0 + 1);
			MessageBox(strErr, "Warning", MB_ICONWARNING);
		}
	}
	
	for (i = IDC_EDIT_VGA10; i <= IDC_EDIT_VGA19; i++)
	{
		CString strDuty;
		GetDlgItem(i)->GetWindowText(strDuty);
		m_stConfig.nFanDutyArray[1][i - IDC_EDIT_VGA10] = atoi(strDuty);
		if (m_stConfig.nFanDutyArray[1][i - IDC_EDIT_VGA10] <= 10)
		{
			strErr.Format("Warning: [%d] VGA1 Fan speed low", i - IDC_EDIT_VGA10 + 1);
			MessageBox(strErr, "Warning", MB_ICONWARNING);
		}
	}
	
	for (i = IDC_EDIT_VGA20; i <= IDC_EDIT_VGA29; i++)
	{
		CString strDuty;
		GetDlgItem(i)->GetWindowText(strDuty);
		m_stConfig.nFanDutyArray[2][i - IDC_EDIT_VGA20] = atoi(strDuty);
		if (m_stConfig.nFanDutyArray[2][i - IDC_EDIT_VGA20] <= 10)
		{
			strErr.Format("Warning: [%d] VGA2 Fan speed low", i - IDC_EDIT_VGA20 + 1);
			MessageBox(strErr, "Warning", MB_ICONWARNING);
		}
	}

	CDialog::OnOK();
}

void CRLConfigDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	
	CDialog::OnCancel();
}

void CRLConfigDlg::OnRadioModeauto() 
{
	// TODO: Add your control notification handler code here
	m_stConfig.bAutoMode = TRUE;
}

void CRLConfigDlg::OnRadioModemanule() 
{
	// TODO: Add your control notification handler code here
	m_stConfig.bAutoMode = FALSE;
}

void CRLConfigDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 2)
	{
		GetDlgItem(IDC_EDIT_TEMPLIMIT)->EnableWindow(m_stConfig.bAutoMode);
		for (int i = IDC_EDIT_CPU0; i <= IDC_EDIT_VGA29; i++)
		{
			GetDlgItem(i)->EnableWindow(m_stConfig.bAutoMode == FALSE ? TRUE:FALSE);
		}

		GetDlgItem(IDC_EDIT_FANCOUNT)->ShowWindow(m_stConfig.bAutoFanCount == FALSE ? SW_SHOW:SW_HIDE);
		GetDlgItem(IDC_STATIC_FANCOUNTINFO)->ShowWindow(m_stConfig.bAutoFanCount == FALSE ? SW_SHOW:SW_HIDE);
	}
	CDialog::OnTimer(nIDEvent);
}

void CRLConfigDlg::OnBtnDefault() 
{
	m_nTempLimit = 75;
	for (int i = 0; i < RL_MAX_FAN; i++)
	{
		m_stConfig.nFanDutyArray[i][0] = 70;			// 85
		m_stConfig.nFanDutyArray[i][1] = 60;			// 80
		m_stConfig.nFanDutyArray[i][2] = 50;			// 75
		m_stConfig.nFanDutyArray[i][3] = 45;			// 70
		m_stConfig.nFanDutyArray[i][4] = 40;			// 65
		m_stConfig.nFanDutyArray[i][5] = 35;			// 60
		m_stConfig.nFanDutyArray[i][6] = 30;			// 55
		m_stConfig.nFanDutyArray[i][7] = 25;			// 50
		m_stConfig.nFanDutyArray[i][8] = 22;			// 45
		m_stConfig.nFanDutyArray[i][9] = 17;			// 40
	}

	RLUpdateFanDuty();
	UpdateData(FALSE);
	UpdateData();
}

void CRLConfigDlg::RLUpdateFanDuty()
{
	int i = 0;
	for (i = IDC_EDIT_CPU0; i <= IDC_EDIT_CPU9; i++)
	{
		CString strDuty;
		strDuty.Format("%d", m_stConfig.nFanDutyArray[0][i - IDC_EDIT_CPU0]);
		GetDlgItem(i)->SetWindowText(strDuty);
		if (m_nFanCount <= 0)
		{
			GetDlgItem(i)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_CPU)->ShowWindow(SW_HIDE);
		}
		else
		{
			GetDlgItem(i)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_CPU)->ShowWindow(SW_SHOW);
		}
	}

	for (i = IDC_EDIT_VGA10; i <= IDC_EDIT_VGA19; i++)
	{
		CString strDuty;
		strDuty.Format("%d", m_stConfig.nFanDutyArray[1][i - IDC_EDIT_VGA10]);
		GetDlgItem(i)->SetWindowText(strDuty);
		if (m_nFanCount <= 1)
		{
			GetDlgItem(i)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_VGA1)->ShowWindow(SW_HIDE);
		}
		else
		{
			GetDlgItem(i)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_VGA1)->ShowWindow(SW_SHOW);
		}
	}
	
	for (i = IDC_EDIT_VGA20; i <= IDC_EDIT_VGA29; i++)
	{
		CString strDuty;
		strDuty.Format("%d", m_stConfig.nFanDutyArray[2][i - IDC_EDIT_VGA20]);
		GetDlgItem(i)->SetWindowText(strDuty);
		if (m_nFanCount <= 2)
		{
			GetDlgItem(i)->ShowWindow(SW_HIDE);
			GetDlgItem(IDC_STATIC_VGA2)->ShowWindow(SW_HIDE);
		}
		else
		{
			GetDlgItem(i)->ShowWindow(SW_SHOW);
			GetDlgItem(IDC_STATIC_VGA2)->ShowWindow(SW_SHOW);
		}
	}
}

void CRLConfigDlg::OnCheckAutofancount() 
{
	// TODO: Add your control notification handler code here
	UpdateData();
	m_stConfig.bAutoFanCount = m_bAutoFanCount;

	if (m_bAutoFanCount == FALSE)
	{
		m_nFanCount = m_nFanCountManual;
	}
	else
	{
		CRLECViewerDlg * pDlg = (CRLECViewerDlg *)AfxGetMainWnd();
		m_nFanCount = pDlg->m_stFanInfo.nFanCount;
	}

	RLUpdateFanDuty();
}

void CRLConfigDlg::OnChangeEditFancount() 
{
	// TODO: If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialog::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.
	
	// TODO: Add your control notification handler code here
	UpdateData();
	if (m_nFanCountManual >= 0 && m_nFanCountManual <= 3)
	{
		m_nFanCount = m_nFanCountManual;
		RLUpdateFanDuty();
	}
}
