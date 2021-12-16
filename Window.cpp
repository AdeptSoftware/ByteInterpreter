#include "pch.h"
#include "Window.h"
#include <vector>

// Карта созданных окон
static std::vector<CWnd*> g_wndMap; 

CWnd::CWnd() {
	g_wndMap.push_back(this);
} 

CWnd::~CWnd() {
	for (UINT i = 0; i < g_wndMap.size(); i++) {
		if (g_wndMap[i] == this) {
			g_wndMap.erase(g_wndMap.begin()+i);
			break;
		}
	}
	m_data.hWnd = nullptr;
}

CWnd* CWnd::FindWnd(HWND hWnd) {
	for (UINT i = 0; i < g_wndMap.size(); i++) {
		if (g_wndMap[i]->m_data.hWnd == hWnd)
			return g_wndMap[i];
	}
	return nullptr;
}

// ========= ========= ========= ========= ========= ========= ========= =========

HFONT CHWNDWrapper::MakeFont(LPCWSTR Typeface, int nSize, BOOL bBold, BOOL bItalic) {
	// Узнаем, сколько пикселей в одном логическом дюйме
	int pixelsPerInch = GetDeviceCaps(GetDC(nullptr), LOGPIXELSY);
	// Узнаем высоту в пикселях шрифта размером Size пунктов
	int fontHeight = -MulDiv(nSize, pixelsPerInch, 72);				// При шрифте 11 выдает -15, но сам шрифт 18px
	return CreateFont(fontHeight, 0, 0, 0, (bBold ? FW_BOLD : FW_NORMAL), bItalic,
		0, 0, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		PROOF_QUALITY, DEFAULT_PITCH | FF_DONTCARE, Typeface);
}

BOOL CHWNDWrapper::Invalidate(BOOL bErase) {
	return ::InvalidateRect(m_data.hWnd, nullptr, bErase);
}

BOOL CHWNDWrapper::ShowWindow(int nCmdShow) {
	return ::ShowWindow(m_data.hWnd, nCmdShow);
}

nullptr_t CHWNDWrapper::DestroyWindow() {
	if (::DestroyWindow(m_data.hWnd))
		m_data.hWnd = nullptr;
	return nullptr;
}

BOOL CHWNDWrapper::IsWindowVisible() {
	return ::IsWindowVisible(m_data.hWnd);
}

HWND CHWNDWrapper::GetWindowHandle() {
	return m_data.hWnd;
}

// ========= ========= ========= ========= ========= ========= ========= =========

CWndRegistrator::CWndRegistrator(const WNDCLASS& wc) {
	m_hInst			= wc.hInstance;
	m_lpszClassName = wc.lpszClassName;
	RegisterClass(&wc);
}

CWndRegistrator::CWndRegistrator(WNDPROC lpfnWndProc, LPCWSTR lpszClassName, int cbWndExtra) {
	m_lpszClassName  = lpszClassName;
	m_hInst			 = GetModuleHandle(nullptr);

	WNDCLASS wc;
	wc.style		 = 0;
	wc.cbClsExtra	 = 0;
	wc.hIcon		 = nullptr;
	wc.lpszMenuName  = nullptr; 
	wc.lpfnWndProc	 = lpfnWndProc;
	wc.cbWndExtra	 = cbWndExtra;
	wc.hInstance	 = m_hInst;
	wc.hCursor		 = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.lpszClassName = lpszClassName;
	RegisterClass(&wc);
}

CWndRegistrator::~CWndRegistrator() {
	UnregisterClass(m_lpszClassName, m_hInst);
}
