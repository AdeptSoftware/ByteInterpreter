#pragma once
#include "WindowData.h"
#include <vector>


class CControlManager {
	const WINDOW_DATA* m_pData;
	HFONT*			   m_hFontPtr;
	BOOL			   m_bFailed;
	UINT			   m_uFirstID;

	std::vector<HWND>  m_list;
public:
	CControlManager(const WINDOW_DATA* pData, UINT uFirstID, HFONT* hFontPtr = nullptr)
		: m_pData(pData), m_uFirstID(uFirstID), m_bFailed(FALSE), m_hFontPtr(hFontPtr) {}
	
	~CControlManager() {
		m_list.clear();
	}

	void Append(HWND hWnd, WNDPROC lpfnWndProc = nullptr) {
		if (hWnd) {
			m_list.push_back(hWnd);
			if (*m_hFontPtr)
				::SendMessage(hWnd, WM_SETFONT, reinterpret_cast<WPARAM>(*m_hFontPtr), NULL);
			if (lpfnWndProc)
				SetWindowLong(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG>(lpfnWndProc));
		}
		else
			m_bFailed = TRUE;
	}

	BOOL Create(int x, int y, int w, int h, LPCWSTR lpszClassName, DWORD dwStyle, 
		        LPCWSTR lpszText = L"", WNDPROC lpfnWndProc = nullptr) {
		if (!m_bFailed) {
			HMENU hID = reinterpret_cast<HMENU>(m_uFirstID+m_list.size());
			HWND hWnd = ::CreateWindow(lpszClassName, lpszText, dwStyle, x, y, w, h,
									   m_pData->hWnd, hID, m_pData->hInst, nullptr);
			Append(hWnd, lpfnWndProc); // m_bFailed - здесь может обновиться
		}
		return !m_bFailed;
	}

	// Если uLast == 0		=> перебираем все элементы. Кроме того, uStart должно быть < uLast!
	// Если pFn вернет TRUE	=> завершение цикла
	BOOL Search(BOOL (*pFn)(HWND), UINT uStart = 0, UINT uLast = 0) {
		auto it_end = (uLast < m_list.size() ? m_list.begin()+uLast : m_list.end());
		for (auto it = m_list.begin() + uStart; it != it_end; it++) {
			if (pFn(*it))
				return TRUE;
		}
		return FALSE;
	}

	HWND operator [](UINT uID) {
		return m_list[uID-m_uFirstID];
	}

	HWND GetAt(UINT uIndex) {
		return m_list[uIndex];
	}

	UINT GetCount() const {
		return m_list.size();
	}

	HFONT GetFont() const {
		return *m_hFontPtr;
	}

	BOOL IsOK() const {
		return !m_bFailed;
	}

	// Формат строки list - см. CComboBoxTree::Fill
	BOOL FillComboBox(UINT uID, LPCWSTR list, int nSel = 0) {
		HWND hComboBox = m_list[uID-m_uFirstID];
		SendMessageW(hComboBox, CB_RESETCONTENT, NULL, NULL);
		for (; wcslen(list) != 0; list += wcslen(list)+1)
			SendMessageW(hComboBox, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(list));
		SendMessageW(hComboBox, CB_SETCURSEL, nSel, NULL);
		return TRUE;
	}

	template <typename T>
	void FillNumberEdit(UINT uID, LPCWSTR fmt, T value) {
		wchar_t buf[512]{ 0 };
		swprintf_s(buf, fmt, value);
		SetWindowTextW(m_list[uID-m_uFirstID], buf);
	}
};