#include "pch.h"
#include "ComboBoxTree.h"	// CComboBoxTree::m_data.hWnd - COMBOBOX


CComboBoxTree::~CComboBoxTree() {
	if (m_hTreeViewFont)
		::DeleteObject(m_hTreeViewFont);
}

nullptr_t CComboBoxTree::DestroyWindow() {
	if (m_hPopupWnd)	 ::DestroyWindow(m_hPopupWnd);
	if (m_data.hWnd)	 ::DestroyWindow(m_data.hWnd);
	if (m_hTreeViewFont) ::DeleteObject(m_hTreeViewFont);
	m_hTreeViewFont	= nullptr;
	m_hPopupWnd		= nullptr;
	m_hTreeView		= nullptr;
	m_data.hWnd		= nullptr;
	return nullptr;
}

BOOL CComboBoxTree::ShowWindow(int nCmdShow) {
	ShowDropDownList(SW_HIDE);
	return ::ShowWindow(m_data.hWnd, nCmdShow);
}

HWND CComboBoxTree::Create(int x, int y, int w, int h, HWND hWndParent, DWORD dwStyle, UINT uCBID, UINT uTVID) {
	m_hPopupWnd = CPopupTreeView::Create(0, 0, w, h, hWndParent, dwStyle, uTVID);
	if (!m_hPopupWnd)
		return CPopupTreeView::DestroyWindow();

	m_data.hWnd = CreateWindow(WC_COMBOBOX, L"", WS_CHILD | CBS_DROPDOWNLIST, x, y, w, 0, hWndParent, 
							   reinterpret_cast<HMENU>(uCBID), GetModuleHandle(nullptr), nullptr);
	if (!m_data.hWnd)
		return DestroyWindow();

	m_cex.m_uID		= uCBID;
	m_hTreeViewFont = MakeFont(L"Arial", 7);
	SetTreeViewFont(m_hTreeViewFont);
	return m_data.hWnd;
}

void CComboBoxTree::OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if ((uMsg == WM_LBUTTONDOWN || uMsg == WM_NCLBUTTONDOWN || uMsg == WM_SIZE) && ::IsWindowVisible(m_hPopupWnd))
		ShowDropDownList(SW_HIDE);
	else if (uMsg == WM_COMMAND) {
		UINT uID = LOWORD(wParam);
		if (uID == GetDlgCtrlID(m_data.hWnd) && HIWORD(wParam) == CBN_SETFOCUS) {
			if (!::IsWindowVisible(m_hPopupWnd)) {
				SendMessageW(m_data.hWnd, CB_SHOWDROPDOWN, TRUE, NULL);
				ShowDropDownList(SW_SHOW);
			}
			else
				OnCloseUp();
		}
		else if (uID == m_cex.m_uID && HIWORD(wParam) == TVN_EX_KEYDOWN) {
			if (lParam == VK_RETURN)
				OnCloseUp();
			else if (lParam >= VK_LEFT && lParam <= VK_DOWN) { 
				HTREEITEM hItem = GetCurSel();
				if (hItem) {
					SetComboBoxText(hItem);
					m_cex.SendCommand(TVN_EX_SELENDOK);
				}
			}
		}
	}
}

void CComboBoxTree::ShowDropDownList(int nCmdShow) {
	if (nCmdShow == SW_SHOW) {
		RECT rc;
		GetWindowRect(m_data.hWnd, &rc);
		SetWindowPos(m_hPopupWnd, HWND_TOPMOST, rc.left, rc.bottom, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW);
		SetFocus(m_hTreeView);
	}
	else {
		::ShowWindow(m_hPopupWnd, SW_HIDE);
		SetActiveWindow(m_hWndParent);
	}
}

void CComboBoxTree::OnCloseUp() {
	wchar_t buf[512];
	GetText(buf, 512);
	SetComboBoxText(buf);
	ShowDropDownList(SW_HIDE);
	m_cex.SendCommand(TVN_EX_SELENDOK);
}

void CComboBoxTree::SetComboBoxText(LPCWSTR lpszText) {
	SendMessageW(m_data.hWnd, CB_RESETCONTENT, NULL, NULL);
	SendMessageW(m_data.hWnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(lpszText));
	SendMessageW(m_data.hWnd, CB_SETCURSEL, 0, NULL);
}

void CComboBoxTree::SetComboBoxText(HTREEITEM hItem) {
	wchar_t buf[512];
	GetItemText(hItem, buf, 512);
	SetComboBoxText(buf);
}

void CComboBoxTree::SetCurSel(HTREEITEM hItem) {
	CPopupTreeView::SetCurSel(hItem);
	SetComboBoxText(hItem);
}

HTREEITEM CComboBoxTree::SetCurSel(int nIndex) {
	HTREEITEM hItem = CPopupTreeView::SetCurSel(nIndex);
	if (hItem)
		SetComboBoxText(hItem);
	return hItem;
}
