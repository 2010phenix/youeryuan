// MyProgress.cpp : implementation file
//

#include "stdafx.h"
#include "MyProgress.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// MyProgress dialog


MyProgress::MyProgress(enum TRANSTYPE type, CWnd* pParent /*=NULL*/)
	: CDialog(MyProgress::IDD, pParent), m_type(type)
{
	//{{AFX_DATA_INIT(MyProgress)
	
	//}}AFX_DATA_INIT
}


void MyProgress::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(MyProgress)
	DDX_Control(pDX, IDC_FROM, m_fromS);
	DDX_Control(pDX, IDC_TO, m_toS);
	DDX_Control(pDX, IDC_NAME, m_nameS);
	DDX_Control(pDX, IDC_REMAINING, m_remainS);
	DDX_Control(pDX, IDC_SPEED, m_speedS);
	DDX_Control(pDX, IDC_PROGRESSCTRL, m_progress);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(MyProgress, CDialog)
	//{{AFX_MSG_MAP(MyProgress)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void MyProgress::OnOK()
{
	if(UpdateData(TRUE))
		DestroyWindow();
}

void MyProgress::OnCancel()
{
	DestroyWindow();
}

void MyProgress::PostNcDestroy()
{
	CDialog::PostNcDestroy();
	delete this;
}

BOOL MyProgress::OnInitDialog() 
{
	DWORD dw = GetWindowLong(m_hWnd, GWL_STYLE);
	dw &= ~WS_SYSMENU;
	SetWindowLong(m_hWnd, GWL_STYLE, dw);

	if(m_type == UPLOAD)	SetWindowText("�����ϴ�...");
	else				SetWindowText("��������...");
	m_progress.SetRange(0, 100);

	CDialog::OnInitDialog();

	return TRUE;  
}
	
//���ô����ļ�
void MyProgress::SetFile(const char* name)
{
	m_nameS.SetWindowText(name);
}

//���ô���Ŀ���ַ��Դ��ַ
void MyProgress::SetSource(const char* src)
{
	m_from2 = src;
	m_fromS.SetWindowText(src);
}

void MyProgress::SetTarget(const char* dst)
{
	m_to2 = dst;
	m_toS.SetWindowText(dst);
}

static CString fromSize(__int64 rSize)
{
	CString str;
	if(rSize < 1024)			str.Format("%lld�ֽ�", rSize);
	else if(rSize < (1<<20))	str.Format("%lldǧ�ֽ�", rSize/1024);
	else if(rSize < (1<<30))	str.Format("%.1f���ֽ�", 1.0*rSize/(1024*1024));
	else						str.Format("%.2f���ֽ�", 1.0*rSize/(1024*1024*1024));
	return str;
}

static CString fromTime(__int64 t)
{
	CString str;
	if(t < 60)			str.Format("%lld��", t);
	else if(t < 3600)	str.Format("%lld��%lld��", t/60, t%60);
	else				str.Format("%lldʱ%lld��%lld��", t/3600, t%3600/60, t%3600%60);
	return str;
}

void MyProgress::SetTotalBytes(__int64 tSize)
{
	totalSize = tSize;
	m_remainS.SetWindowText("��ʣ" + fromSize(tSize));
	m_progress.SetPos(0);
}

void MyProgress::SetSubdir(const char* subDir)
{
	CString from = m_from2 + "\\" + subDir;
	m_fromS.SetWindowText(from);

	CString to = m_to2 + "\\" + subDir;
	m_toS.SetWindowText(to);
}

//�Ѿ������˶����ֽ�
void MyProgress::SetElapsedBytes(__int64 nowSize)
{
	__int64 rSize = totalSize - nowSize;
	m_remainS.SetWindowText("��ʣ" + fromSize(rSize));

	m_progress.SetPos(100 * nowSize / totalSize);
}

void MyProgress::SetSpeed(__int64 speed, int isTime)
{
	if(isTime == 0)	m_speedS.SetWindowText("ÿ����" + fromSize(speed));
	else
	{
		CString str;
		m_speedS.GetWindowTextA(str);
		m_speedS.SetWindowText(str + "(" + fromTime(speed) + "���)");
	}
}

/////////////////////////////////////////////////////////////////////////////
// MyProgress message handlers
