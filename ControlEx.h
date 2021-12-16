#pragma once
#include <Windows.h>


// Обертка для отлова сообщений от hWndTarger
class CControlEx {
	WNDPROC m_lpfnDefaultWndProc	= nullptr;
public:
	HWND	m_hWndHandler			= nullptr;
	UINT	m_uID					= 0;

	BOOL Attach(HWND hWndTarget, HWND hWndHandler, UINT uID, WNDPROC lpfnWndProc) {
		if (m_lpfnDefaultWndProc)
			return FALSE;

		m_lpfnDefaultWndProc = reinterpret_cast<WNDPROC>(GetWindowLong(hWndTarget, GWLP_WNDPROC));
		m_hWndHandler		 = hWndHandler;
		m_uID				 = uID;

		SetWindowLong(hWndTarget, GWLP_USERDATA, reinterpret_cast<LONG>(this));
		SetWindowLong(hWndTarget, GWLP_WNDPROC,  reinterpret_cast<LONG>(lpfnWndProc));
		return TRUE;
	}

	LRESULT Call(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
		return CallWindowProc(m_lpfnDefaultWndProc, hWnd, uMsg, wParam, lParam);
	}

	LRESULT SendCommand(UINT uNotify, LPARAM lParam = 0u) {
		return SendMessageW(m_hWndHandler, WM_COMMAND, MAKEWPARAM(m_uID, uNotify), lParam);
	}

	static CControlEx* Get(HWND hWndTarget) {
		return reinterpret_cast<CControlEx*>(GetWindowLong(hWndTarget, GWLP_USERDATA));
	}
};
