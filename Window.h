#pragma once
#include "WindowData.h"

class CHWNDWrapper {
public:
	BOOL Invalidate(BOOL bErase = FALSE);
	BOOL IsWindowVisible();

	virtual nullptr_t DestroyWindow();
	virtual BOOL ShowWindow(int nCmdShow);

	static  HFONT MakeFont(LPCWSTR Typeface, int nSize, BOOL bBold = FALSE, BOOL bItalic = FALSE);

	// �������� ������ ���� �������, ����� ��� ������ �� ����������� ���������� ������ ������
	HWND GetWindowHandle();
protected:
	WINDOW_DATA m_data;
};

// ������� ����������� ����� ����
class CWnd : public CHWNDWrapper {
public:
	CWnd();
	~CWnd();

	virtual BOOL WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	static  CWnd* FindWnd(HWND hWnd);
};

// ��������������� ����� ��� ����������� ����
class CWndRegistrator {
	LPCWSTR	  m_lpszClassName;
	HINSTANCE m_hInst;
public:
	CWndRegistrator(const WNDCLASS& wc);
	CWndRegistrator(WNDPROC lpfnWndProc, LPCWSTR lpszClassName, int cbWndExtra = 0);
	~CWndRegistrator();
};