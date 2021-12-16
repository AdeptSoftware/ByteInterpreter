#pragma once
#include "Window.h"
#include "ControlEx.h"
#include <CommCtrl.h>

// Перебирает рекурсивно все элементы
const UINT  TVGN_NEXT_ITEM = 0xFFFF;

// WM_COMMAND: WPARAM = MAKEWPARAM(UINT_CTRL_ID, UINT_MESSAGE_CODE)
//			   MESSAGE			 CODE				LPARAM
const UINT  TVN_EX_KEYDOWN	= WM_USER+125; // Код клавиши VK_XXXXXX


class CPopupTreeView : public CHWNDWrapper {
public:
	virtual HWND Create(int x, int y, int w, int h, HWND hWndParent, DWORD dwStyle, UINT uTVID);
	virtual nullptr_t DestroyWindow();

	void SetTreeViewFont(HFONT hFont);

	virtual HTREEITEM SetCurSel(int nIndex);
	virtual void SetCurSel(HTREEITEM hItem);
	HTREEITEM	 GetCurSel();
	HTREEITEM 	 GetItem(int nIndex, UINT uCode = TVGN_NEXT_ITEM, HTREEITEM hFirst = nullptr);
	int		  	 GetItemIndex(HTREEITEM hItem, UINT uCode = TVGN_NEXT_ITEM, HTREEITEM hFirst = nullptr);
	void	  	 GetText(wchar_t* szBuffer, int nCount);
	void	  	 GetItemText(HTREEITEM hItem, wchar_t* szBuffer, int nCount);
	HTREEITEM 	 AddString(LPCWSTR lpszText, HTREEITEM hRoot = nullptr);

	// nSel - Номер текущего элемента (-1 = не выбрано)
	// hRoot - Корневой элемент дерева (nullptr = вставка в корень дерева)
	// list - список элементов. Пример: list = L"Element1\0Element2\0Element3\0"; 
	void Fill(HTREEITEM hRoot, LPCWSTR list, int nSel = -1);

protected:
	HWND m_hTreeView = nullptr;
	CControlEx m_cex; // Настройка и отлов сообщений от TreeView

	BOOL GetItemIndex(HTREEITEM& hCurItem, HTREEITEM hItem, UINT uCode, int& nPos);
	BOOL GetItemIndex(HTREEITEM& hCurItem, int nFind, UINT uCode, int& nPos);
};

