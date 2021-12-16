#include "pch.h"
#include "resource.h"
#include "PluginDialog.h"
#include "control_constants.h"

const LPCWSTR	CLASS_NAME			= L"MainPluginDialog";
const LONG		MIN_W				= 650;
const LONG		MIN_H				= 133;
const UINT		NOMOVE_CTRL_COUNT	= 3;

const UINT		BUFFER_SIZE			= 32;

const UINT		WM_CHANGE_VALUE		= WM_USER+1;

// ========= ========= ========= ========= ========= ========= ========= =========

WNDPROC g_lpfnDefaultEditProc = nullptr;

LRESULT CALLBACK EditProc(HWND hEdit, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_KEYDOWN) {
		if (wParam == VK_DOWN || wParam == VK_UP) {
			wchar_t szText[BUFFER_SIZE];
			GetWindowTextW(hEdit, szText, BUFFER_SIZE);
			int nValue = _wtoi(szText);
			int nKoef = GetAsyncKeyState(VK_SHIFT) ? 1 : 8;
			if (wParam == VK_DOWN)		nValue -= nKoef;
			else if (wParam == VK_UP)	nValue += nKoef;
			if (nValue >= 0) {
				_itow_s(nValue, szText, 10);
				SetWindowTextW(hEdit, szText);
				SendMessageW(GetParent(hEdit), WM_CHANGE_VALUE, GetDlgCtrlID(hEdit), nValue);
			}
		} //Активируем родительское окно и установим 0 в текстовое поле, если там пусто
		else if (wParam == VK_RETURN) {
			wchar_t szText[BUFFER_SIZE];
			GetWindowTextW(hEdit, szText, BUFFER_SIZE);
			if (wcslen(szText) == 0)
				SetWindowTextW(hEdit, L"0");
			HWND hWndParent = GetParent(hEdit);
			SendMessageW(hWndParent, WM_ACTIVATE, WA_ACTIVE, NULL);
			SendMessageW(hWndParent, WM_CHANGE_VALUE, GetDlgCtrlID(hEdit), _wtoi(szText));
		}
	}
	return CallWindowProc(g_lpfnDefaultEditProc, hEdit, uMsg, wParam, lParam);
}

// ========= ========= ========= ========= ========= ========= ========= =========

LRESULT CALLBACK DialogProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	CPluginDialog* pWnd = reinterpret_cast<CPluginDialog*>(CWnd::FindWnd(hWnd));
	if (pWnd) {
		if (uMsg == WM_PAINT) {
			RECT rcWindow;
			PAINTSTRUCT ps;
			GetClientRect(hWnd, &rcWindow);
			HDC hDC = BeginPaint(hWnd, &ps);
			pWnd->OnPaint(hDC, rcWindow);
			EndPaint(hWnd, &ps);
		}
		else if (uMsg >= WM_CTLCOLORMSGBOX && uMsg <= WM_CTLCOLORSTATIC)
			return reinterpret_cast<LRESULT>(GetStockObject(WHITE_BRUSH));
		else if (uMsg == WM_GETMINMAXINFO) {
			MINMAXINFO* mm = (MINMAXINFO*)lParam;
			mm->ptMinTrackSize.x = MIN_W;
			mm->ptMinTrackSize.y = MIN_H;
		}
		else if (!pWnd->WndProc(hWnd, uMsg, wParam, lParam))
			return 0;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

BOOL CPluginDialog::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	m_cb_model.OnMessage(uMsg, wParam, lParam);
	if (uMsg == WM_CLOSE)
		return !ShowWindow(SW_HIDE);
	else if (uMsg == WM_SIZE)
		OnSizing(LOWORD(lParam), HIWORD(lParam));
	else if (uMsg == WM_KEYDOWN)
		OnKeyDown(wParam, lParam);
	else if (uMsg == WM_COMMAND)
		OnCommand(lParam, LOWORD(wParam), HIWORD(wParam));
	else if (uMsg == WM_CHANGE_VALUE)
		OnChangeValue(wParam, lParam);
	return TRUE;
}

void CPluginDialog::OnPaint(HDC hDC, RECT& rc) {
	//int nWidth = m_img.GetDrawRect().right;
	int nWidth = rc.right;
	HFONT hOldFont = (HFONT)SelectObject(hDC, m_hFont);
	COLORREF clrOldBackground = SetBkColor(hDC, RGB(255, 255, 255));
	if(::IsWindowVisible(m_controls[ID_CB_COLOR_MODELS])) {
		TextOutW(hDC, nWidth-175, -1, L"Width", 5);
		TextOutW(hDC, nWidth-141, -1, L"Height", 6);
	}
	TextOutW(hDC, nWidth-107, -1, L"Stride", 6);
	TextOutW(hDC, nWidth-73,  -1, L"Offset", 6);
	TextOutW(hDC, nWidth-28,  -1, L"Skip",   4);
	SetBkColor(hDC, clrOldBackground);
	SelectObject(hDC, hOldFont);
	// Составим изображение
	//m_img.Draw(hDC);
}

// ========= ========= ========= ========= ========= ========= ========= =========

CPluginDialog::CPluginDialog(HINSTANCE hInstDLL)
	: m_wrPluginDialog(DialogProc, CLASS_NAME, DLGWINDOWEXTRA),
	  m_controls(&m_data, ID_CB_DATA_TYPES, &m_hFont),
	  m_converter(m_bi) {
	m_hMenu		 = LoadMenu(hInstDLL, MAKEINTRESOURCE(IDR_PLUGIN_MENU));
	m_data.hInst = GetModuleHandle(nullptr);
	m_hFont		 = MakeFont(L"Arial", 8);
}

CPluginDialog::~CPluginDialog() {
	if (m_hFont)	DeleteObject(m_hFont);
	if (m_bi.bytes)	delete[] m_bi.bytes;
	if (m_hMenu)	DestroyMenu(m_hMenu);
	m_bi.bytes	= nullptr;
	m_hFont		= nullptr;
	m_hMenu		= nullptr;
}

POINT CPluginDialog::CalcWindowRect(LONG w, LONG h, LONG offsetRT) {
	RECT rc;
	GetWindowRect(GetDesktopWindow(), &rc);
	return { rc.right-w-offsetRT, offsetRT };
}

HWND CPluginDialog::Create(HWND hWndParent, HICON hIcon) {
	POINT p = CalcWindowRect(MIN_W, MIN_H, 40); // Ширина, высота окна и отступ от верхнего правого угла экрана
	DWORD dwStyle = WS_SYSMENU | WS_BORDER | WS_THICKFRAME | WS_CAPTION; 
	m_data.hWnd = ::CreateWindowW(CLASS_NAME, L"", dwStyle, p.x, p.y, MIN_W, MIN_H, hWndParent, m_hMenu, m_data.hInst, nullptr);
	if (!m_data.hWnd || !m_hMenu) {
		MessageBox(nullptr, L"Window wasn't created!", CLASS_NAME, MB_OK);
		return FALSE;
	}
	SendMessageW(m_data.hWnd, WM_SETICON, ICON_BIG, reinterpret_cast<LPARAM>(hIcon));
	if (!CreateControls())
		DestroyWindow();
	return m_data.hWnd;
}

BOOL CPluginDialog::CreateControls() {
	HWND hColorModel = m_cb_model.Create(0, 0, 150, 200, m_data.hWnd, TVS_COMBOBOX_LIST | TVS_HASBUTTONS, ID_CB_COLOR_MODELS, ID_TREE_VIEW_CB);
	m_controls.Create(  5, 7, 155, 200, WC_COMBOBOX, WS_VISIBLE | WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST);							// ID_CB_DATA_TYPES
	m_controls.Create(  0, 0,   0,   0, WC_EDIT,	 WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL);	// ID_EDIT_OUTPUT
	m_controls.Create(163, 8,  65,  22, WC_BUTTON,	 WS_CHILD   | BS_AUTOCHECKBOX, L"unsigned");										// ID_BTN_EXTENDED
	m_controls.Append(hColorModel);																										// ID_CB_COLOR_MODELS
	m_controls.Create(  0, 0,  35,  18, WC_EDIT,	 WS_BORDER  | WS_CHILD | ES_CENTER | ES_NUMBER | WS_DISABLED, L"0", EditProc);		// ID_EDIT_WIDTH
	m_controls.Create(  0, 0,  35,  18, WC_EDIT,	 WS_BORDER  | WS_CHILD | ES_CENTER | ES_NUMBER | WS_DISABLED, L"0", EditProc);		// ID_EDIT_HEIGHT
	m_controls.Create(  0, 0,  20,  18, WC_EDIT,	 WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER | WS_BORDER,   L"0", EditProc);		// ID_EDIT_STRIDE
	m_controls.Create(  0, 0,  50,  18, WC_EDIT,	 WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER | WS_BORDER,   L"0", EditProc);		// ID_EDIT_OFFSET
	m_controls.Create(  0, 0,  28,  18, WC_EDIT,	 WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER | WS_BORDER,   L"0", EditProc);		// ID_EDIT_SKIP
	if (!m_controls.IsOK() || !hColorModel)
		return FALSE;
	// Определим дефолтовый WndProc для модифицированных Edit
	g_lpfnDefaultEditProc = reinterpret_cast<WNDPROC>(GetWindowLong(m_controls[ID_EDIT_OUTPUT], GWLP_WNDPROC));
	// Заполним выпадающие списки (массив "types" должен соотвествовать enum class DataType) // └ elem
	m_controls.FillComboBox(ID_CB_DATA_TYPES, L"Int64\0Int32\0Int16\0Int8\0HEX\0OCT\0BIN\0Float\0Double\0Color\0Unicode\0", e_cast(m_bi.type));
	m_cb_model.Fill(nullptr, L"RGB\0CMYK\0HSV\0CIE XYZ\0YUV\0YCoCg\0");
	m_cb_model.Fill(m_cb_model.GetItem(4), L"YCbCr\0YIQ\0YDbDr\0");
	m_cb_model.Fill(m_cb_model.GetItem(3), L"L*a*b*\0xyY\0Luv\0LCH(ab)\0LCH(uv)\0LMS\0Hunter L,a,b\0");
	m_cb_model.Fill(m_cb_model.GetItem(2), L"HSL\0HSLA\0HSI\0HWB\0");
	m_cb_model.Fill(m_cb_model.GetItem(1), L"CMY\0");
	m_cb_model.Fill(m_cb_model.GetItem(0), L"RGBA\0GrayScale\0");
	m_cb_model.SetCurSel(e_cast(m_bi.clr.type));
	// Обновим текстовые поля
	m_controls.FillNumberEdit(ID_EDIT_WIDTH,  m_bi.img.uWidth);
	m_controls.FillNumberEdit(ID_EDIT_HEIGHT, m_bi.img.uHeight);
	m_controls.FillNumberEdit(ID_EDIT_STRIDE, m_bi.uStride);
	m_controls.FillNumberEdit(ID_EDIT_OFFSET, m_bi.uOffset);
	m_controls.FillNumberEdit(ID_EDIT_SKIP,   m_bi.uSkip);
	// Установим вывод для конвертера
	m_converter.SetOutputText(m_controls[ID_EDIT_OUTPUT]);
	m_converter.SetOutputProgress(m_data.hWnd);
	// Рассчитаем позиции элементов управления и отобрази нужные
	OnSizing(MIN_W, MIN_H);
	ShowColorControls();
	ShowDataControls();
	return TRUE;
}

void CPluginDialog::ShowDataControls() {
	BOOL bShow = FALSE;
	if (m_bi.type == BI::DataType::DOUBLE)
		bShow = ::SetWindowTextW(m_controls[ID_BTN_EXTENDED], L"exp"); 
	else if (m_bi.type == BI::DataType::UNICODEBYTES)
		bShow = ::SetWindowTextW(m_controls[ID_BTN_EXTENDED], L"bytes");
	else if (m_bi.type >= BI::DataType::INT64 && m_bi.type <= BI::DataType::INT8)
		bShow = ::SetWindowTextW(m_controls[ID_BTN_EXTENDED], L"unsigned");
	::ShowWindow(m_controls[ID_BTN_EXTENDED], bShow ? SW_SHOW : SW_HIDE);
	Invalidate(TRUE);
}

void CPluginDialog::ShowColorControls() {
	BOOL bShow = (m_bi.type == BI::DataType::COLOR ? SW_SHOW : SW_HIDE);
	::ShowWindow(m_controls[ID_EDIT_HEIGHT], bShow);
	::ShowWindow(m_controls[ID_EDIT_WIDTH], bShow);
	m_cb_model.ShowWindow(bShow);
}

void CPluginDialog::OnSizing(int w, int h) {
	UINT uFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	int x[] = { w-332, w-179, w-143, w-103, w-82, w-31 };
	for (UINT i = 0; i < m_controls.GetCount()-NOMOVE_CTRL_COUNT; i++)
		SetWindowPos(m_controls.GetAt(i+NOMOVE_CTRL_COUNT), nullptr, x[i], (i == 0 ? 7 : 11), 0, 0, uFlags | SWP_NOSIZE);
	SetWindowPos(m_controls[ID_EDIT_OUTPUT], nullptr, 5, 31, w-8, h-34, uFlags);
	Invalidate(TRUE);
}

void CPluginDialog::OnCommand(LPARAM lParam, UINT uCtrlID, UINT uNotify, void* pData) {
	// При выборе типа данных
	if (uCtrlID == ID_CB_DATA_TYPES && uNotify == CBN_SELENDOK) {
		int nIndex = SendMessageW(reinterpret_cast<HWND>(lParam), CB_GETCURSEL, NULL, NULL);
		if (nIndex != CB_ERR) {
			BI::DataType type = static_cast<BI::DataType>(nIndex);
			if (m_bi.type != type) {
				m_bi.type = type;
				ShowDataControls();
				ShowColorControls();
				UpdateOutput();
			}
		}
	}
	// При выборе модели цвета
	else if (uCtrlID == ID_CB_COLOR_MODELS && uNotify == TVN_EX_SELENDOK) {
		HTREEITEM hItem = m_cb_model.GetCurSel();
		int nIndex = m_cb_model.GetItemIndex(hItem, TVGN_NEXT_ITEM);
		if (nIndex != CB_ERR) {
			BI::ColorList clr = static_cast<BI::ColorList>(nIndex);
			if (m_bi.clr.type != clr) {
				m_bi.clr.type = clr;
				UpdateOutput();
			}
		}
	}
	// Реакция на изменение CheckBox'а
	else if (uCtrlID == ID_BTN_EXTENDED && uNotify == BN_CLICKED) {
		m_bi.bExtended = (SendMessage(reinterpret_cast<HWND>(lParam), BM_GETCHECK, NULL, NULL) == BST_CHECKED);
		UpdateOutput();
	}
	else if (lParam == NULL)
		OnMenuCommand(uCtrlID);
}

void CPluginDialog::OnChangeValue(UINT uCtrlID, UINT uValue) {
	if (uCtrlID == ID_EDIT_WIDTH)		m_bi.img.uWidth		= uValue;
	else if (uCtrlID == ID_EDIT_HEIGHT)	m_bi.img.uHeight	= uValue;
	else if (uCtrlID == ID_EDIT_STRIDE)	m_bi.uStride		= uValue;
	else if (uCtrlID == ID_EDIT_OFFSET)	m_bi.uOffset		= uValue;
	else if (uCtrlID == ID_EDIT_SKIP)	m_bi.uSkip			= uValue;
	else return;
	if (!GetAsyncKeyState(VK_CONTROL)) {
		if (uCtrlID != ID_EDIT_WIDTH && uCtrlID != ID_EDIT_HEIGHT)
			UpdateOutput(FALSE);
		//if (m_cb_model.IsWindowVisible()) m_img.Redraw(); 
	}
}

void CPluginDialog::UpdateOutput(BOOL bCheckKeyState) {
	if (bCheckKeyState && GetAsyncKeyState(VK_CONTROL))
		return;
	m_converter.Extract();
}

void CPluginDialog::SetBytes(BYTE* bytes, ULONG length, BOOL bIsWideChar) {
	if (m_bi.bytes)
		delete[] m_bi.bytes;

	if (bIsWideChar) {
		char* buf = new char[length];
		wchar_t* ptr = reinterpret_cast<wchar_t*>(bytes); 
		WideCharToMultiByte(CP_ACP, 0, ptr, length, buf, length, NULL, NULL);
		m_bi.bytes = reinterpret_cast<BYTE*>(buf);
	}
	else
		m_bi.bytes = bytes;
	m_bi.uLength = length;
	// Преобразуем байты
	if (m_bResetOffset) {
		m_bi.uOffset = 0;
		SetWindowText(m_controls[ID_EDIT_OFFSET], L"0");
	}
	UpdateOutput(FALSE);
}

#include "PluginDialog_Menu.inl"