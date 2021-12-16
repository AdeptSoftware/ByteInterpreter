#include "pch.h"
#include "PopupTreeView.h"	// CPopupTreeView::m_data.hWnd - POPUP WINDOW

LRESULT CALLBACK TreeViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	CControlEx* pData = CControlEx::Get(hWnd);
	LRESULT result = pData->Call(hWnd, uMsg, wParam, lParam);
	if (uMsg == WM_KEYDOWN)
		pData->SendCommand(TVN_EX_KEYDOWN, wParam);
	else if (uMsg == WM_LBUTTONDOWN)	//  акой-то баг с отображением, когда используетс€ SCROLL
		InvalidateRect(hWnd, nullptr, TRUE);
	return result;
}

nullptr_t CPopupTreeView::DestroyWindow() {
	if (m_data.hWnd)
		::DestroyWindow(m_data.hWnd);	// + уничтожает дочерние
	m_hTreeView		= nullptr;
	m_data.hWnd		= nullptr;
	return nullptr;
}

void CPopupTreeView::SetTreeViewFont(HFONT hFont) {
	SendMessage(m_hTreeView, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), NULL);
}

HWND CPopupTreeView::Create(int x, int y, int w, int h, HWND hWndParent, DWORD dwStyle, UINT uTVID) {
	DWORD dwWnd = WS_POPUP;
	if (!(dwStyle & WS_CHILD))	dwStyle |= WS_CHILD;
	if (!(dwStyle & WS_BORDER))	dwStyle |= WS_BORDER;
	if (dwStyle & WS_VISIBLE) {
		dwStyle &= ~WS_VISIBLE;
		dwWnd |= WS_VISIBLE;
	}

	m_data.hInst = GetModuleHandle(nullptr);
	m_data.hWnd = CreateWindow(WC_DIALOG, L"", dwWnd, 0, 0, w, h, hWndParent, nullptr, m_data.hInst, nullptr);
	if (!m_data.hWnd)
		return DestroyWindow();

	RECT rc;
	GetClientRect(m_data.hWnd, &rc);
	m_hTreeView = CreateWindow(WC_TREEVIEW, L"", dwStyle | WS_VISIBLE, 0, 0, rc.right, rc.bottom, m_data.hWnd,
							   reinterpret_cast<HMENU>(uTVID), m_data.hInst, nullptr);
	
	if (!m_hTreeView || !m_cex.Attach(m_hTreeView, hWndParent, uTVID, TreeViewProc))
		return DestroyWindow();
	return m_data.hWnd;
}

HTREEITEM CPopupTreeView::GetCurSel() {
	return TreeView_GetSelection(m_hTreeView);;
}

void CPopupTreeView::SetCurSel(HTREEITEM hItem) {
	TreeView_SelectItem(m_hTreeView, hItem);
}

HTREEITEM CPopupTreeView::SetCurSel(int nIndex) {
	int nPos = 0;
	HTREEITEM hCurItem = TreeView_GetRoot(m_hTreeView);
	if (GetItemIndex(hCurItem, nIndex, TVGN_NEXT_ITEM, nPos)) {
		TreeView_SelectItem(m_hTreeView, hCurItem);
		return hCurItem;
	}
	return nullptr;
}

HTREEITEM CPopupTreeView::AddString(LPCWSTR lpszText, HTREEITEM hRoot) {
	TVINSERTSTRUCT tv = { 0 };
	tv.hParent = hRoot;
	tv.item.mask = TVIF_TEXT | TVIF_HANDLE;
	tv.item.pszText = const_cast<LPWSTR>(lpszText);
	return TreeView_InsertItem(m_hTreeView, &tv);
}

void CPopupTreeView::GetItemText(HTREEITEM hItem, wchar_t* szBuffer, int nCount) {
	TVITEM tvi = { 0 };
	tvi.hItem = hItem;
	tvi.mask = TVIF_TEXT | TVIF_HANDLE;
	tvi.pszText = szBuffer;
	tvi.cchTextMax = nCount;
	TreeView_GetItem(m_hTreeView, &tvi);
}

void CPopupTreeView::GetText(wchar_t* szBuffer, int nCount) {
	GetItemText(GetCurSel(), szBuffer, nCount);
}

BOOL CPopupTreeView::GetItemIndex(HTREEITEM& hCurItem, HTREEITEM hItem, UINT uCode, int& nPos) {
	while (hCurItem) {
		if (hCurItem == hItem)
			return TRUE;
		nPos++;
		if (uCode == TVGN_NEXT_ITEM) {
			HTREEITEM hChildItem = TreeView_GetNextItem(m_hTreeView, hCurItem, TVGN_CHILD);
			if (GetItemIndex(hChildItem, hItem, uCode, nPos))
				return TRUE;
		}
		hCurItem = TreeView_GetNextItem(m_hTreeView, hCurItem, (TVGN_NEXT_ITEM ? TVGN_NEXT : uCode)); 
	}
	return FALSE;
}

// ѕока пусть так будет так...
BOOL CPopupTreeView::GetItemIndex(HTREEITEM& hCurItem, int nFind, UINT uCode, int& nPos) {
	while (hCurItem) {
		if (nFind == nPos)
			return TRUE;
		nPos++;
		if (uCode == TVGN_NEXT_ITEM) {
			HTREEITEM hChildItem = TreeView_GetNextItem(m_hTreeView, hCurItem, TVGN_CHILD );
			if (GetItemIndex(hChildItem, nFind, uCode, nPos)) {
				hCurItem = hChildItem;
				return TRUE;
			}
		}
		hCurItem = TreeView_GetNextItem(m_hTreeView, hCurItem, (TVGN_NEXT_ITEM ? TVGN_NEXT : uCode));
	}
	return FALSE;
}

int CPopupTreeView::GetItemIndex(HTREEITEM hItem, UINT uCode, HTREEITEM hFirst) {
	int nPos = 0;
	if (!hFirst)
		hFirst = TreeView_GetRoot(m_hTreeView);
	if (GetItemIndex(hFirst, hItem, uCode, nPos))
		return nPos;
	return -1;
}

HTREEITEM CPopupTreeView::GetItem(int nIndex, UINT uCode, HTREEITEM hFirst) {
	int nPos = 0;
	if (!hFirst)
		hFirst = TreeView_GetRoot(m_hTreeView);
	if (GetItemIndex(hFirst, nIndex, uCode, nPos))
		return hFirst;
	return nullptr;
}

void CPopupTreeView::Fill(HTREEITEM hRoot, LPCWSTR list, int nSel) {
	for (int nIndex = 0; wcslen(list) != 0; nIndex++, list += wcslen(list) + 1) {
		HTREEITEM hItem = AddString(list, hRoot);
		if (nIndex == nSel)
			SetCurSel(hItem);
	}
}
