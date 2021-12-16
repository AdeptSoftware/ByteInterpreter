#pragma once
#include "PopupTreeView.h"

const DWORD	TVS_COMBOBOX_LIST = TVS_FULLROWSELECT | TVS_SHOWSELALWAYS | LVS_SINGLESEL;

// WM_COMMAND: WPARAM = MAKEWPARAM(UINT_CTRL_ID, UINT_MESSAGE_CODE)
//			   MESSAGE			 CODE				LPARAM
const UINT  TVN_EX_SELENDOK = WM_USER+126; // Не используется


class CComboBoxTree : public CPopupTreeView {
public:
	~CComboBoxTree();
	void ShowDropDownList(int nCmdShow);

	virtual nullptr_t DestroyWindow();
	virtual BOOL ShowWindow(int nCmdShow);
	virtual HWND Create(int x, int y, int w, int h, HWND hWndParent, DWORD dwStyle, UINT uCBID, UINT uTVID);

	virtual void SetCurSel(HTREEITEM hItem);
	virtual HTREEITEM SetCurSel(int nIndex);

	void OnMessage(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
	HFONT m_hTreeViewFont = nullptr;
	HWND  m_hWndParent	  = nullptr;
	HWND  m_hPopupWnd	  = nullptr;

	void OnCloseUp();
	void SetComboBoxText(LPCWSTR lpszText);
	void SetComboBoxText(HTREEITEM hItem);
};

