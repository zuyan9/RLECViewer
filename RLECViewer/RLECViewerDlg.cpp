// RLECViewerDlg.cpp : implementation file
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
// CAboutDlg dialog used for App About
int g_nAutoFanDuty[16] = {70, 65, 60, 50, 45, 41, 37, 34, 31, 29, 27, 26, 25, 24, 23, 22};

int CalculateRpms(int getCpuRpm)
{
	if (getCpuRpm <= 0)
		return 0;
	double flCpuRpm = 60.0 * 1000000 / (1.39130434782609E-05 * 1000000 * getCpuRpm * 4.0 * 1000000);
	flCpuRpm = flCpuRpm * 2.0 * 1000000;
	return (int) flCpuRpm;
}
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

BOOL CAboutDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	COleDateTime	dtTime;
	CString			strBuild;
	strBuild.Format("%s %s", __DATE__, __TIME__);
	dtTime.ParseDateTime(strBuild);
	
	strBuild.Format("Build: %s", dtTime.Format());
	GetDlgItem(IDC_BUILD_TEXT)->SetWindowText(strBuild);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

/////////////////////////////////////////////////////////////////////////////
// CRLECViewerDlg dialog

CRLECViewerDlg::CRLECViewerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CRLECViewerDlg::IDD, pParent)
	, m_hInstDLL(NULL)
	, m_pfnSetFanDuty(NULL)
	, m_pfnSetFANDutyAuto(NULL)
	, m_pfnGetCPUFANRPM(NULL)
	, m_pfnGetECVersion(NULL)
	, m_pfnGetFANCounter(NULL)
	, m_pfnGetGPU1FANRPM(NULL)
	, m_pfnGetGPUFANRPM(NULL)
	, m_pfnGetTempFanDuty(NULL)
	, m_pfnGetX72FANRPM(NULL)
	, m_hThread(NULL)
	, m_bExit(FALSE)
	, m_nStatus(0)
	, m_hInstDLL2(NULL)
{
	//{{AFX_DATA_INIT(CRLECViewerDlg)
	m_bAutoRun = FALSE;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	memset(&m_stFanInfo, 0, sizeof(m_stFanInfo));
	memset(&m_stAllInfo, 0, sizeof(m_stAllInfo));
	memset(m_nLastFanTemp, -1, sizeof(m_nLastFanTemp));
	memset(m_nCurFanIdx, -1, sizeof(m_nCurFanIdx));
}

void CRLECViewerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CRLECViewerDlg)
	DDX_Check(pDX, IDC_CHECK_AUTORUN, m_bAutoRun);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CRLECViewerDlg, CDialog)
	//{{AFX_MSG_MAP(CRLECViewerDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BTN_RESET, OnBtnReset)
	ON_BN_CLICKED(IDC_CHECK_AUTORUN, OnCheckAutorun)
	ON_COMMAND(ID_RESTORE, OnRestore)
	ON_COMMAND(ID_ABOUT, OnAbout)
	ON_BN_CLICKED(IDC_BTN_CONFIG, OnBtnConfig)
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_TRAYNOTIFY, OnTrayNotification)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CRLECViewerDlg message handlers
WORD CRLECViewerDlg::sTrayTool[] = 
{
	IDB_BITMAP_TRAY,		// Menu Item Bitmap list...
		16, 16,					// Menu Item Bitmap width and height...
		ID_ABOUT,				// Begin Menu Item Command...
		ID_RESTORE,
		IDCANCEL,
		NULL,					// End Menu Item Command...
};


BOOL CRLECViewerDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	// Create thread...
	SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);
	SetTimer(1, 500, NULL);

	if (m_hThread == NULL)
	{
		DWORD dwThreadID = 0;
		m_bExit = FALSE;
		m_hThread = CreateThread(NULL, NULL, RLECThread, this, NULL, &dwThreadID);
	}
	
	// 读取配置  Read config
	m_stAllInfo.bAutoMode = TRUE;
	m_stAllInfo.bCoolOverLevel = TRUE;
	m_stAllInfo.bAutoFanCount = FALSE;
	m_stAllInfo.nFanCountManual = 2;
	m_stAllInfo.nTempLimit = 75;
	for (int i = 0; i < RL_MAX_FAN; i++)
	{
		m_stAllInfo.nFanDutyArray[i][0] = 70;			// 85
		m_stAllInfo.nFanDutyArray[i][1] = 60;			// 80
		m_stAllInfo.nFanDutyArray[i][2] = 50;			// 75
		m_stAllInfo.nFanDutyArray[i][3] = 45;			// 70
		m_stAllInfo.nFanDutyArray[i][4] = 40;			// 65
		m_stAllInfo.nFanDutyArray[i][5] = 35;			// 60
		m_stAllInfo.nFanDutyArray[i][6] = 30;			// 55
		m_stAllInfo.nFanDutyArray[i][7] = 25;			// 50
		m_stAllInfo.nFanDutyArray[i][8] = 22;			// 45
		m_stAllInfo.nFanDutyArray[i][9] = 17;			// 40
	}

	SFLoadINF(TRUE);

	m_bAutoRun = SFReadAutoRunFromeReg();

	UpdateData(FALSE);
	UpdateData();

	m_TrayMenu.LoadMenu(IDR_MENU_TRAY);
	ASSERT( m_TrayMenu.m_hMenu != NULL );
	m_TrayMenu.LoadToolBar(sTrayTool);
	if( !m_TrayIcon.Create(this, IDR_MENU_TRAY, _T("RLECViewer"), m_hIcon, WM_TRAYNOTIFY) )
	{
		TRACE0("Failed to create TrayIcon...\n");
		return -1;
	}

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CRLECViewerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
		if(nID == SC_MINIMIZE)
		{
			ShowWindow(SW_HIDE);
		}
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CRLECViewerDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CRLECViewerDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

BOOL CRLECViewerDlg::RLECProc()
{
	try
	{
		m_nStatus = EC_STATUS_INIT;
		BOOL bRet = RLInit();
		int nTryCount = 0;
		while (m_bExit == FALSE && bRet == FALSE)
		{
			bRet = RLInit();
			if (bRet == FALSE && nTryCount++ >= 10)
			{
				RLUnInit();
				return FALSE;
			}
			Sleep(1000);
		}
		

		if (m_pfnGetECVersion != NULL)
		{
		//	string strVer = m_pfnGetECVersion();
		}

		m_nStatus = EC_STATUS_RUNNING;
		int nSleepTime = 1000;
		if (m_pfnGetFANCounter != NULL)
		{
			m_stFanInfo.nFanCount = m_pfnGetFANCounter();
		}

		ShowWindow(SW_MINIMIZE);
		ShowWindow(SW_HIDE);
		while (m_bExit == FALSE)
		{
			if (m_stFanInfo.nFanCount <= 0)
			{
				m_stFanInfo.nFanCount = m_pfnGetFANCounter();
			}
			else if (m_pfnGetFANCounter != NULL)
			{
				for (int i = 0; i < m_stFanInfo.nFanCount && m_bExit == FALSE; i++)
				{
					if (m_pfnGetTempFanDuty != NULL)
					{
						TRACE("[GetTempFanDuty] Start...\n");
						ECData stECData = m_pfnGetTempFanDuty(i + 1);
						TRACE("[GetTempFanDuty] End...\n");
						m_stFanInfo.nFanDuty[i] = stECData.FanDuty;
						m_stFanInfo.nFanTempRemote[i] = stECData.Remote;
						m_stFanInfo.nFanTempLocal[i] = stECData.Local;
						Sleep(nSleepTime);
					}
				}

				// cpu
				if (m_pfnSetFanDuty)
				{
					int nManualCPUFanDuty = RLCalcManualFanDuty(0);
					if (nManualCPUFanDuty > 0)
					{
						m_pfnSetFanDuty(1, nManualCPUFanDuty * 255 / 100);
					}
				}

				// gpu
				if (m_pfnSetFanDuty)
				{
					int nManualGPUFanDuty = RLCalcManualFanDuty(1);
					if (nManualGPUFanDuty > 0)
					{
						m_pfnSetFanDuty(2, nManualGPUFanDuty * 255 / 100);
					}
				}
	/*
				if (m_pfnGetCPUFANRPM)
				{
					TRACE("[GetCPUFANRPM] Start...\n");
					m_stFanInfo.nCpuFanRPM = m_pfnGetCPUFANRPM();
					TRACE("[GetCPUFANRPM] End...\n");
					Sleep(nSleepTime);
				}

				if (m_pfnGetGPUFANRPM)
				{
					TRACE("[GetGPUFANRPM] Start...\n");
					m_stFanInfo.nGpu1FanRPM = m_pfnGetGPUFANRPM();
					TRACE("[GetGPUFANRPM] End...\n");
					Sleep(nSleepTime);
				}

				if (m_pfnGetGPU1FANRPM)
				{
					TRACE("[GPU1FANRPM] Start...\n");
					m_stFanInfo.nGpu2FanRPM = m_pfnGetGPU1FANRPM();
					TRACE("[GPU1FANRPM] End...\n");
					Sleep(nSleepTime);
				}*/
			}

			Sleep(nSleepTime);
		}
	}
	catch (...)
	{
	}

	return TRUE;
}

BOOL CRLECViewerDlg::RLECProc2()
{
	try
	{
		m_nStatus = EC_STATUS_INIT2;
		BOOL bRet = RLInit2();
		int nTryCount = 0;
		while (m_bExit == FALSE && bRet == FALSE)
		{
			bRet = RLInit2();
			if (bRet== FALSE && nTryCount++ >= 10)
			{
				RLUnInit2();
				return FALSE;
			}

			Sleep(1000);
		}
		

		m_nStatus = EC_STATUS_RUNNING2;
		int nSleepTime = 1000;

		// 设置风扇数量  Set fan amount
		int nDelayCount = 0;
		while (m_bExit == FALSE)
		{
			if (nDelayCount++ >= 10)
				break;

			Sleep(100);
		}

		m_stFanInfo.nFanCount = m_stAllInfo.nFanCountManual;
		if (m_pfnGetFANCounter != NULL && m_stAllInfo.bAutoFanCount)
		{
			m_stFanInfo.nFanCount = m_pfnGetFANCounter();
		}

		nDelayCount = 0;
		while (m_bExit == FALSE)
		{
			if (nDelayCount++ >= 10)
				break;
			
			Sleep(100);
		}
		
		ShowWindow(SW_MINIMIZE);
		ShowWindow(SW_HIDE);
		while (m_bExit == FALSE)
		{
			// check config change...
			if (m_stAllInfo.bAutoFanCount == FALSE)
			{
				if (m_pfnSetFANDutyAuto != NULL && m_stFanInfo.nFanCount != m_stAllInfo.nFanCountManual)
				{
					for (int i = 0; i < m_stFanInfo.nFanCount; i++)
					{
						m_pfnSetFANDutyAuto(i + 1);
						Sleep(200);
					}
				}
				
				m_stFanInfo.nFanCount = m_stAllInfo.nFanCountManual;
			}
			else
			{
				// 获取风扇数量  Get fan amount
				if (m_pfnGetFANCounter != NULL)
				{
					m_stFanInfo.nFanCount = m_pfnGetFANCounter();
					Sleep(200);
					if (m_stFanInfo.nFanCount != m_stAllInfo.nFanCountManual)
					{
						for (int i = 0; i < m_stAllInfo.nFanCountManual; i++)
						{
							m_pfnSetFANDutyAuto(i + 1);
							Sleep(200);
						}
					}
				}
			}

			if (m_stFanInfo.nFanCount <= 0)
			{
				if (m_stAllInfo.bAutoFanCount)
					m_stFanInfo.nFanCount = m_pfnGetFANCounter();
			}
			else if (m_pfnGetFANCounter != NULL)
			{
				int nMaxFanCount = m_stFanInfo.nFanCount;
				if (nMaxFanCount >= RL_MAX_FAN)
					nMaxFanCount = RL_MAX_FAN;

				for (int i = 0; i < nMaxFanCount && m_bExit == FALSE; i++)
				{
					if (m_pfnGetTempFanDuty2 != NULL)
					{
						TRACE("[GetTempFanDuty %d] Start...\n", i);
						ECData2 stECData = m_pfnGetTempFanDuty2(i + 1);
						TRACE("[GetTempFanDuty %d] End...\n", i);
						m_stFanInfo.nFanDuty[i] = stECData.FanDuty;
						m_stFanInfo.nFanTempRemote[i] = stECData.Remote;
						m_stFanInfo.nFanTempLocal[i] = stECData.Local;
						Sleep(nSleepTime);
					}
				}

				// cpu
				if (m_pfnSetFanDuty)
				{
					int nManualCPUFanDuty = RLCalcManualFanDuty(0);
					if (nManualCPUFanDuty > 0)
					{
						m_pfnSetFanDuty(1, nManualCPUFanDuty * 255 / 100);
						Sleep(nSleepTime);
					}
				}

				// gpu1
				if (m_pfnSetFanDuty && m_stFanInfo.nFanCount >= 2)
				{
					int nManualGPUFanDuty = RLCalcManualFanDuty(1);
					if (nManualGPUFanDuty > 0)
					{
						m_pfnSetFanDuty(2, nManualGPUFanDuty * 255 / 100);
						Sleep(nSleepTime);
					}
				}

				// gpu2
				if (m_pfnSetFanDuty && m_stFanInfo.nFanCount >= 3)
				{
					int nManualGPUFanDuty = RLCalcManualFanDuty(2);
					if (nManualGPUFanDuty > 0)
					{
						m_pfnSetFanDuty(3, nManualGPUFanDuty * 255 / 100);
						Sleep(nSleepTime);
					}
				}
			}

			Sleep(nSleepTime);
		}
	}
	catch (...)
	{
	}

	return TRUE;
}

DWORD CRLECViewerDlg::RLECThread(LPVOID lParam)
{
	CRLECViewerDlg * pDlg = (CRLECViewerDlg *)lParam;
	if (pDlg != NULL)
	{
		if (pDlg->RLECProc2() == FALSE)
			pDlg->RLECProc();
	}


	return 0x99;
}

BOOL CRLECViewerDlg::RLInit()
{
	if (m_hInstDLL == NULL)
	{
		m_hInstDLL = LoadLibrary("ECView.dll");
		if (m_hInstDLL == NULL)
		{
			return FALSE;
		}
	}
	
	if (m_hInstDLL != NULL)
	{
		m_pfnSetFanDuty		= (SetFanDuty *)::GetProcAddress(m_hInstDLL, "SetFANDuty");
		m_pfnSetFANDutyAuto		= (SetFANDutyAuto *)::GetProcAddress(m_hInstDLL, "SetFANDutyAuto");
		m_pfnGetTempFanDuty	= (GetTempFanDuty *)::GetProcAddress(m_hInstDLL, "GetTempFanDuty");
		
		m_pfnGetFANCounter		= (GetFANCounter *)::GetProcAddress(m_hInstDLL, "GetFANCounter");
		m_pfnGetECVersion		= (GetECVersion *)::GetProcAddress(m_hInstDLL, "GetECVersion");
		
		m_pfnGetCPUFANRPM = (GetCPUFANRPM *)::GetProcAddress(m_hInstDLL, "GetCPUFANRPM");
		m_pfnGetGPUFANRPM	= (GetGPUFANRPM *)::GetProcAddress(m_hInstDLL, "GetGPUFANRPM");
		m_pfnGetGPU1FANRPM	= (GetGPU1FANRPM *)::GetProcAddress(m_hInstDLL, "GetGPU1FANRPM");
		m_pfnGetX72FANRPM	= (GetX72FANRPM *)::GetProcAddress(m_hInstDLL, "GetX72FANRPM");
	}

	return TRUE;
}

void CRLECViewerDlg::RLUnInit()
{
	if (m_hInstDLL != NULL)
	{
		if (m_pfnSetFANDutyAuto != NULL)
		{
			for (int i = 0; i < m_stFanInfo.nFanCount; i++)
				m_pfnSetFANDutyAuto(i + 1);
		}

		FreeLibrary(m_hInstDLL);
		m_hInstDLL = NULL;
		
		m_pfnSetFanDuty		= NULL;
		m_pfnSetFANDutyAuto	= NULL;
		m_pfnGetTempFanDuty	= NULL;
		
		m_pfnGetFANCounter	= NULL;
		m_pfnGetECVersion	= NULL;
		
		m_pfnGetCPUFANRPM	= NULL;
		m_pfnGetGPUFANRPM	= NULL;
		m_pfnGetGPU1FANRPM	= NULL;
		m_pfnGetX72FANRPM	= NULL;
		
		m_nStatus = 0;
		
		memset(&m_stFanInfo, 0, sizeof(m_stFanInfo));
	}
}

BOOL CRLECViewerDlg::RLInit2()
{
	if (m_hInstDLL2 == NULL)
	{
		m_hInstDLL2 = LoadLibrary("ClevoEcInfo.dll");
		if (m_hInstDLL2 == NULL)
		{
			return FALSE;
		}
	}
	
	if (m_hInstDLL2 != NULL)
	{
		m_pfnInitIo			= (InitIo *)::GetProcAddress(m_hInstDLL2, "InitIo");
		m_pfnSetFanDuty		= (SetFanDuty *)::GetProcAddress(m_hInstDLL2, "SetFanDuty");
		m_pfnSetFANDutyAuto	= (SetFANDutyAuto *)::GetProcAddress(m_hInstDLL2, "SetFanDutyAuto");
		m_pfnGetTempFanDuty2	= (GetTempFanDuty2 *)::GetProcAddress(m_hInstDLL2, "GetTempFanDuty");	
		m_pfnGetFANCounter	= (GetFANCounter *)::GetProcAddress(m_hInstDLL2, "GetFanCount");

		BOOL bRet = FALSE;
		if (m_pfnInitIo != NULL)
			bRet = m_pfnInitIo();
	}
	
	return TRUE;
}

void CRLECViewerDlg::RLUnInit2()
{
	if (m_hInstDLL2 != NULL)
	{
		if (m_pfnSetFANDutyAuto != NULL)
		{
			for (int i = 0; i < m_stFanInfo.nFanCount; i++)
				m_pfnSetFANDutyAuto(i + 1);
		}
		
		FreeLibrary(m_hInstDLL2);
		m_hInstDLL2 = NULL;
		
		m_pfnSetFanDuty		= NULL;
		m_pfnSetFANDutyAuto	= NULL;
		m_pfnGetTempFanDuty	= NULL;
		
		m_pfnGetFANCounter	= NULL;
		m_pfnGetECVersion	= NULL;
		
		m_pfnGetCPUFANRPM	= NULL;
		m_pfnGetGPUFANRPM	= NULL;
		m_pfnGetGPU1FANRPM	= NULL;
		m_pfnGetX72FANRPM	= NULL;
		
		m_nStatus = 0;
		
		memset(&m_stFanInfo, 0, sizeof(m_stFanInfo));	
	}
}

void CRLECViewerDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	m_bExit = TRUE;
	DWORD dwWaitRet = 0;
	if (m_hThread != NULL)
	{
		dwWaitRet = WaitForSingleObject(m_hThread, 5000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	
	RLUnInit();
	RLUnInit2();

	if (dwWaitRet == WAIT_TIMEOUT)
	{
		TerminateProcess(GetCurrentProcess(), 0);
	}
}

void CRLECViewerDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == 1)
	{
		CString strInfo;
		switch (m_nStatus)
		{
		case EC_STATUS_INIT:
			strInfo = "Initializing Core ver 5.5 ...";
			break;
		case EC_STATUS_INIT2:
			strInfo = "Initializing Core ver 6.8 ...";
			break;
		case EC_STATUS_RUNNING:
		case EC_STATUS_RUNNING2:
			{
				CString strTempDuty;
				for (int i = 0; i < m_stFanInfo.nFanCount; i++)
				{
					CString strTemp;
					CString strVGAName;
					strVGAName.Format("VGA%d", i);
					strTemp.Format("%s：Fan Speed: %d%%, Temp: %d C\n", i == 0 ? "CPU":strVGAName, (int)((float)m_stFanInfo.nFanDuty[i] * 100/255) + 1, m_stFanInfo.nFanTempRemote[i]);
					strTempDuty += strTemp;
				}

				int nMaxFanCount = m_stFanInfo.nFanCount;
				if (nMaxFanCount >= RL_MAX_FAN)
					nMaxFanCount = RL_MAX_FAN;
				
				strInfo.Format("Running Core Version %s (%d Fans, %s)\n%s", m_nStatus == EC_STATUS_RUNNING ? "5.5":"6.8", nMaxFanCount, m_stAllInfo.bAutoFanCount ? "Auto Detected":"Manual Set", strTempDuty);
			}
			break;
		default:
			strInfo = "Initializing";
			break;

		}

		CString strInfo2;
		strInfo2.Format("CPU: %d RPM, GPU1: %d RPM, GPU2: %d RPM", CalculateRpms(m_stFanInfo.nCpuFanRPM), CalculateRpms(m_stFanInfo.nGpu1FanRPM), CalculateRpms(m_stFanInfo.nGpu2FanRPM));
		GetDlgItem(IDC_STATIC_INFO)->SetWindowText(strInfo);
		GetDlgItem(IDC_STATIC_INFO2)->SetWindowText(strInfo2);
	}
	CDialog::OnTimer(nIDEvent);
}

void CRLECViewerDlg::OnBtnReset() 
{
	m_bExit = TRUE;
	if (m_hThread != NULL)
	{
		WaitForSingleObject(m_hThread, 15000);
		CloseHandle(m_hThread);
		m_hThread = NULL;
	}
	
	RLUnInit();

	//
	if (m_hThread == NULL)
	{
		DWORD dwThreadID = 0;
		m_bExit = FALSE;
		m_hThread = CreateThread(NULL, NULL, RLECThread, this, NULL, &dwThreadID);
	}

}

void CRLECViewerDlg::OnOK() 
{
	// TODO: Add extra validation here
	
//	CDialog::OnOK();
}

void CRLECViewerDlg::OnCancel() 
{
	// TODO: Add extra cleanup here
	if(MessageBox("Are you sure you want to exit?", "RLECViewer", MB_OKCANCEL|MB_ICONWARNING) == IDCANCEL)
	{
		return;
	}
	
	
	CDialog::OnCancel();
}

// 从注册表中读出自动运行设置	Read from registry
BOOL CRLECViewerDlg::SFReadAutoRunFromeReg()
{
	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",	&hKey) !=ERROR_SUCCESS)
	{
		return FALSE;
	}
	
	BOOL		  bRet			= FALSE;
	unsigned long lSize			= sizeof(bRet);
	CString		  strProduct	= "RLECViewer AutoRun";
	if (RegQueryValueEx(hKey, strProduct, NULL, NULL, NULL, &lSize) != ERROR_SUCCESS)
	{
		RegCloseKey(hKey);
		return FALSE;
	}
	
	RegCloseKey(hKey);
	return lSize > 0 ? TRUE:FALSE;
}

BOOL CRLECViewerDlg::SFSaveToReg(BOOL bRun)
{
	HKEY hKey;
	if(RegOpenKey(HKEY_LOCAL_MACHINE, "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", &hKey)!=ERROR_SUCCESS)
	{
		return FALSE;
	}
	
	CString	strProduct = "RLECViewer AutoRun";
	if(bRun)
	{
		CString			strPath;
		unsigned long	nSize = 0;
		
		strPath = RLGetExePath() + "\\RLECViewer.exe";
		
		nSize	= strPath.GetLength();
		if(RegSetValueEx(hKey, strProduct, 0, REG_SZ,
			(unsigned char *)strPath.GetBuffer(strPath.GetLength()), nSize) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
	}
	else
	{
		if(RegDeleteValue(hKey, strProduct) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
	}
	
    RegCloseKey(hKey);
	return TRUE;
}

void CRLECViewerDlg::OnCheckAutorun() 
{
	UpdateData();
	SFSaveToReg(m_bAutoRun);
}

int CRLECViewerDlg::RLCalcManualFanDuty(int nFanIdx)
{
	if (nFanIdx < 0 || nFanIdx >= m_stFanInfo.nFanCount)
		return -1;

	if (m_nLastFanTemp[nFanIdx] < 0)
		m_nLastFanTemp[nFanIdx] = 0;

	int nCalcDutyPer = 0;
	int nFanDutyIdx = 0;
	if (m_stAllInfo.bAutoMode)
	{
		int nMinTemp = 50;				// 最低温度开始调节  Min temp before fan speed adjustment
		int nMaxDutyPer = 80;			// 最高负载80%转速   Fan speed 80% at max load
		int nMinDutyPer = 18;			// 最低负载18%转速   Fan speed 18% at min load
		nCalcDutyPer = nMinDutyPer;

		// 低于最低温度则以最小转速运行  Below MinTemp, fan runs at min rmp
		if (m_stFanInfo.nFanTempRemote[nFanIdx] <= nMinTemp)
			return nMinDutyPer;

		// 计算与温度的差距  Calculate differences in temp
		int nOffsetLimit = m_stAllInfo.nTempLimit - nMinTemp;
		int nOffsetReal	= m_stFanInfo.nFanTempRemote[nFanIdx] - nMinTemp;

		int OffsetPer = (int)(((float)nOffsetReal/(float)nOffsetLimit) * 100);
		if (OffsetPer >= 130)
		{
			nFanDutyIdx = 0;
		}
		else if (OffsetPer >= 120)
		{
			nFanDutyIdx = 1;
		}
		else if (OffsetPer >= 110)
		{
			nFanDutyIdx = 2;
		}
		else if (OffsetPer >= 105)
		{
			nFanDutyIdx = 3;
		}
		else if (OffsetPer >= 100)
		{
			nFanDutyIdx = 4;
		}
		else if (OffsetPer >= 90)
		{
			nFanDutyIdx = 5;
		}
		else if (OffsetPer >= 85)
		{
			nFanDutyIdx = 6;
		}
		else if (OffsetPer >= 80)
		{
			nFanDutyIdx = 7;
		}
		else if (OffsetPer >= 70)
		{
			nFanDutyIdx = 8;
		}
		else if (OffsetPer >= 60)
		{
			nFanDutyIdx = 9;
		}
		else if (OffsetPer >= 50)
		{
			nFanDutyIdx = 10;
		}
		else if (OffsetPer > 40)
		{
			nFanDutyIdx = 11;
		}
		else if (OffsetPer > 30)
		{
			nFanDutyIdx = 12;
		}
		else if (OffsetPer > 20)
		{
			nFanDutyIdx = 13;
		}
		else if (OffsetPer > 10)
		{
			nFanDutyIdx = 14;
		}
		else
		{
			nFanDutyIdx = 15;
		}

		nCalcDutyPer = g_nAutoFanDuty[nFanDutyIdx];
		if (m_nCurFanIdx[nFanIdx] >= 0 && m_nTargetFanIdx[nFanIdx] >= 0)
		{

			if (nFanDutyIdx < m_nTargetFanIdx[nFanIdx] && nFanDutyIdx >= m_nCurFanIdx[nFanIdx])
			{
				// 降温时需跨越2个级别  OverLevel cooling cross two levels
				TRACE("[FanIdx %d] CalcDutyPer %d -> %d\n", nFanIdx, nCalcDutyPer, g_nAutoFanDuty[m_nCurFanIdx[nFanIdx]]);
				nCalcDutyPer = g_nAutoFanDuty[m_nCurFanIdx[nFanIdx]];
			}
			else if (nFanDutyIdx >= m_nTargetFanIdx[nFanIdx])
			{
				// 已达到降温级别  Cooling level reached
				m_nCurFanIdx[nFanIdx] = -1;
				m_nTargetFanIdx[nFanIdx] = -1;
			}
		}
	}
	else
	{
		// 根据风扇温度取值
		int nFanTemp = m_stFanInfo.nFanTempRemote[nFanIdx];
		if (nFanTemp >= 85)	// 85度及以上   85C and above
		{
			nFanDutyIdx = 0;
		}
		else if (nFanTemp >= 80)
		{
			nFanDutyIdx = 1;
		}
		else if (nFanTemp >= 75)
		{
			nFanDutyIdx = 2;		
		}
		else if (nFanTemp >= 70)
		{
			nFanDutyIdx = 3;
		}
		else if (nFanTemp >= 65)
		{
			nFanDutyIdx = 4;
		}
		else if (nFanTemp >= 60)
		{
			nFanDutyIdx = 5;
		}
		else if (nFanTemp >= 55)
		{
			nFanDutyIdx = 6;
		}
		else if (nFanTemp >= 50)
		{
			nFanDutyIdx = 7;
		}
		else if (nFanTemp >= 45)
		{
			nFanDutyIdx = 8;
		}
		else		// 40度及以下   40C and below
		{
			nFanDutyIdx = 9;
		}

		nCalcDutyPer = m_stAllInfo.nFanDutyArray[nFanIdx][nFanDutyIdx];
		if (m_nCurFanIdx[nFanIdx] >= 0 && m_nTargetFanIdx[nFanIdx] >= 0)
		{
			if (nFanDutyIdx < m_nTargetFanIdx[nFanIdx] && nFanDutyIdx >= m_nCurFanIdx[nFanIdx])
			{
				// 降温时需跨越2个级别  OverLevel cooling cross two levels
				TRACE("[FanIdx %d] CalcDutyPer %d -> %d\n", nFanIdx, nCalcDutyPer, m_stAllInfo.nFanDutyArray[nFanIdx][m_nCurFanIdx[nFanIdx]]);
				nCalcDutyPer = m_stAllInfo.nFanDutyArray[nFanIdx][m_nCurFanIdx[nFanIdx]];
			}
			else if (nFanDutyIdx >= m_nTargetFanIdx[nFanIdx])
			{
				// 已达到降温级别  OverLevel cooling reached
				m_nCurFanIdx[nFanIdx] = -1;
				m_nTargetFanIdx[nFanIdx] = -1;
			}
		}
	}
	
	if (m_stAllInfo.bCoolOverLevel)
	{
		// 判断是升温还是降温
		if (m_nLastFanTemp[nFanIdx] > m_stFanInfo.nFanTempRemote[nFanIdx])
		{
			// 降温
			TRACE("[FanIdx %d] Temp dropping...\n", nFanIdx);
			if (m_nTargetFanIdx[nFanIdx] < 0 && m_nCurFanIdx[nFanIdx] < 0)
			{
				m_nCurFanIdx[nFanIdx] = nFanDutyIdx;
				m_nTargetFanIdx[nFanIdx] = m_nCurFanIdx[nFanIdx] + 2;
			}
		}
		else if (m_nLastFanTemp[nFanIdx] < m_stFanInfo.nFanTempRemote[nFanIdx])
		{
			TRACE("[FanIdx %d] Temp raising...\n", nFanIdx);
			m_nCurFanIdx[nFanIdx] = -1;
			m_nTargetFanIdx[nFanIdx] = -1;
			
		}
	}
	else
	{
		m_nCurFanIdx[nFanIdx] = -1;
		m_nTargetFanIdx[nFanIdx] = -1;
	}

	m_nLastFanTemp[nFanIdx] = m_stFanInfo.nFanTempRemote[nFanIdx];
	
	return nCalcDutyPer;
}

//
//将系统设置信息保存到文件中
// Save config to file
//
BOOL CRLECViewerDlg::SFSaveINF()
{
	FILE *fp = NULL;
	CString strPath = RLGetExePath() + "\\RLECViewer.cfg";
	fp = fopen(strPath, "wb");
	if(fp == NULL)
	{
		return FALSE;
	}
	fwrite(&m_stAllInfo, 1, sizeof(m_stAllInfo), fp);
	fclose(fp);
	
	return TRUE;
}
//
//初始化系统信息
// Init system info
//
BOOL CRLECViewerDlg::SFLoadINF(BOOL bWriteDefault)
{
	CFile file;
	CString strPath = RLGetExePath() + "\\RLECViewer.cfg";
	if(!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
	{
		if (bWriteDefault == FALSE)
			return FALSE;
		
		if(!file.Open(strPath, CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone))
		{
			return FALSE;
		}
		else
		{
			file.Write(&m_stAllInfo, sizeof(m_stAllInfo));
			file.Close();
			return TRUE;
		}
	}
	
	if(file.GetLength() != sizeof(m_stAllInfo))
	{
		if (bWriteDefault == FALSE)
			return FALSE;
		
		file.Close();
		file.Open(strPath, CFile::modeCreate | CFile::modeWrite);
		file.Write(&m_stAllInfo, sizeof(m_stAllInfo));
		file.Close();
		return TRUE;
	}
	
	if(file.GetLength() > 0)
	{
		file.Read(&m_stAllInfo, sizeof(m_stAllInfo));
	}
	
	file.Close();
	return TRUE;
}
		
LRESULT CRLECViewerDlg::OnTrayNotification(WPARAM wParam, LPARAM lParam)
{
	if( wParam != IDR_MENU_TRAY )
		return FALSE;
	if( lParam == WM_RBUTTONUP )
	{
		CPoint		pos;
		CNewMenu *	pSubMenu = DYNAMIC_DOWNCAST(CNewMenu, m_TrayMenu.GetSubMenu(0));
		if( pSubMenu == NULL )
			return FALSE;
		::GetCursorPos(&pos);
		this->SetForegroundWindow();
		pSubMenu->SetMenuText(ID_RESTORE, IsWindowVisible() ? "Hide(&R)" : "Open(&R)", MF_BYCOMMAND);
		//pSubMenu->SetMenuTitle("C2 Teacher", MFT_SIDE_TITLE | MFT_CENTER | MFT_GRADIENT);
		//pSubMenu->SetMenuTitleColor(CLR_DEFAULT, RGB(0, 192, 255), RGB(0, 64, 128));
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_RIGHTBUTTON, pos.x, pos.y, this);
		this->PostMessage(WM_NULL, 0, 0);
	} 
	else if( lParam == WM_LBUTTONDBLCLK )
	{
		this->OnRestore();
	}
	return TRUE;
}

void CRLECViewerDlg::OnRestore() 
{
	if( this->IsWindowVisible())
	{
		this->ShowWindow(SW_SHOWMINIMIZED);
		this->ShowWindow(SW_HIDE);
	}
	else
	{
		this->ShowWindow(SW_RESTORE);
		this->SetWindowPos(&CWnd::wndTopMost, 0, 0, 0, 0, SWP_NOMOVE|SWP_NOSIZE);
		this->SetForegroundWindow(); 
	}
}


void CRLECViewerDlg::OnAbout() 
{
	CAboutDlg	aboutDlg;
	aboutDlg.DoModal();
}

void CRLECViewerDlg::OnBtnConfig() 
{
	CRLConfigDlg dlg;
	dlg.m_nFanCount = m_stFanInfo.nFanCount;
	dlg.m_stConfig = m_stAllInfo;
	if (dlg.DoModal() == IDOK)
	{
		m_stAllInfo = dlg.m_stConfig;
		SFSaveINF();

		memset(m_nLastFanTemp, -1, sizeof(m_nLastFanTemp));
		memset(m_nCurFanIdx, -1, sizeof(m_nCurFanIdx));
	}
}
