#pragma once
#include "WindowData.h"

class CHWNDWrapper {
public:
	BOOL Invalidate(BOOL bErase = FALSE);
	BOOL IsWindowVisible();

	virtual nullptr_t DestroyWindow();
	virtual BOOL ShowWindow(int nCmdShow);

	static  HFONT MakeFont(LPCWSTR Typeface, int nSize, BOOL bBold = FALSE, BOOL bItalic = FALSE);

	// Избегать вызова этой функции, чтобы вся логика по возможности оставалась внутри класса
	HWND GetWindowHandle();
protected:
	WINDOW_DATA m_data;
};

// Базовый абстрактный класс окна
class CWnd : public CHWNDWrapper {
public:
	CWnd();
	~CWnd();

	virtual BOOL WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) = 0;
	static  CWnd* FindWnd(HWND hWnd);
};

// Вспомогательный класс для регистрации окон
class CWndRegistrator {
	LPCWSTR	  m_lpszClassName;
	HINSTANCE m_hInst;
public:
	CWndRegistrator(const WNDCLASS& wc);
	CWndRegistrator(WNDPROC lpfnWndProc, LPCWSTR lpszClassName, int cbWndExtra = 0);
	~CWndRegistrator();
};