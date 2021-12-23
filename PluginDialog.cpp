#include "pch.h"
#include "resource.h"
#include "PluginDialog.h"
#include "control_constants.h"

const LPCWSTR	CLASS_NAME			= L"MainPluginDialog";
const LONG		DLG_W				= 650;
const LONG		DLG_H				= 140;
const LONG		CTRL_H_OFFSET		= 59;	// Смещение от верха окна до ближайшего контрола
const UINT		NOMOVE_CTRL_COUNT	= 3;
const int		GAP					= 3;

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

BOOL CPluginDialog::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_GETMINMAXINFO) {
		MINMAXINFO* mm = (MINMAXINFO*)lParam;
		mm->ptMinTrackSize.x = DLG_W;
		mm->ptMinTrackSize.y = DLG_H;
		return TRUE;
	}
	m_cb_model.OnMessage(uMsg, wParam, lParam);
	m_viewer.OnMessage(hWnd, uMsg, wParam, lParam);
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
	else if (uMsg == WM_SETTING_UPDATE)
		OnUpdateSettings(reinterpret_cast<COLOR_CONVERTER_SETTINGS*>(lParam));
	return TRUE;
}

void CPluginDialog::OnPaint(HDC hDC, RECT& rc) {
	int nWidth = rc.right;
	HFONT hOldFont = (HFONT)SelectObject(hDC, m_hFont);
	COLORREF clrOldBackground = SetBkColor(hDC, RGB(255, 255, 255));
	if(::IsWindowVisible(m_controls[ID_CB_COLOR_MODELS])) {
		TextOutW(hDC, nWidth-GAP-172, -1, L"Width", 5);
		TextOutW(hDC, nWidth-GAP-139, -1, L"Height", 6);
	}
	TextOutW(hDC, nWidth-GAP-105, -1, L"Stride", 6);
	TextOutW(hDC, nWidth-GAP-70,  -1, L"Offset", 6);
	TextOutW(hDC, nWidth-GAP-25,  -1, L"Skip",   4);
	SetBkColor(hDC, clrOldBackground);
	SelectObject(hDC, hOldFont);
	// Составим изображение
	m_viewer.Draw(hDC);
}

// ========= ========= ========= ========= ========= ========= ========= =========

CPluginDialog::CPluginDialog(HINSTANCE hInstDLL)
	: m_wrPluginDialog(CLASS_NAME, DLGWINDOWEXTRA),
	  m_controls(&m_data, ID_CB_DATA_TYPES, &m_hFont),
	  m_settings(&m_hFont),
	  m_converter(m_bi) {
	m_hMenu		 = LoadMenu(hInstDLL, MAKEINTRESOURCE(IDR_PLUGIN_MENU));
	m_data.hInst = GetModuleHandle(nullptr);
	m_hFont		 = MakeFont(L"Arial", 8);
	m_converter.ChangeMethods();
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
	POINT p = CalcWindowRect(DLG_W, DLG_H, 40); // Ширина, высота окна и отступ от верхнего правого угла экрана
	DWORD dwStyle = WS_SYSMENU | WS_BORDER | WS_THICKFRAME | WS_CAPTION | WS_MAXIMIZEBOX; 
	m_data.hWnd = ::CreateWindowW(CLASS_NAME, L"", dwStyle, p.x, p.y, DLG_W, DLG_H, hWndParent, m_hMenu, m_data.hInst, nullptr);
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
	m_controls.Create(GAP, 7, 155, 200, WC_COMBOBOX, WS_VISIBLE | WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST);							// ID_CB_DATA_TYPES
	m_controls.Create(  0, 0,   0,   0, WC_EDIT,	 WS_VISIBLE | WS_CHILD | WS_BORDER | ES_AUTOVSCROLL | ES_MULTILINE | WS_VSCROLL);	// ID_EDIT_OUTPUT
	m_controls.Create(163, 8,  65,  22, WC_BUTTON,	 WS_CHILD   | BS_AUTOCHECKBOX, L"unsigned");										// ID_BTN_EXTENDED
	m_controls.Append(hColorModel);																										// ID_CB_COLOR_MODELS
	m_controls.Create(  0, 0,  35,  18, WC_EDIT,	 WS_BORDER  | WS_CHILD | ES_CENTER | ES_NUMBER | WS_DISABLED, L"0", EditProc);		// ID_EDIT_WIDTH
	m_controls.Create(  0, 0,  35,  18, WC_EDIT,	 WS_BORDER  | WS_CHILD | ES_CENTER | ES_NUMBER | WS_DISABLED, L"0", EditProc);		// ID_EDIT_HEIGHT
	m_controls.Create(  0, 0,  20,  18, WC_EDIT,	 WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER | WS_BORDER,   L"0", EditProc);		// ID_EDIT_STRIDE
	m_controls.Create(  0, 0,  50,  18, WC_EDIT,	 WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER | WS_BORDER,   L"0", EditProc);		// ID_EDIT_OFFSET
	m_controls.Create(  0, 0,  28,  18, WC_EDIT,	 WS_VISIBLE | WS_CHILD | ES_CENTER | ES_NUMBER | WS_BORDER,   L"0", EditProc);		// ID_EDIT_SKIP
	if (!m_controls.IsOK() || !hColorModel || !m_settings.Init(m_data.hWnd))
		return FALSE;
	// Определим дефолтовый WndProc для модифицированных Edit
	g_lpfnDefaultEditProc = reinterpret_cast<WNDPROC>(GetWindowLong(m_controls[ID_EDIT_OUTPUT], GWLP_WNDPROC));
	// Заполним выпадающие списки (массив "types" должен соотвествовать enum class DataType) // └ elem
	m_controls.FillComboBox(ID_CB_DATA_TYPES, L"Int64\0Int32\0Int16\0Int8\0HEX\0OCT\0BIN\0Float\0Double\0Color\0Unicode\0", static_cast<int>(m_bi.type));
	m_cb_model.Fill(nullptr, L"RGB\0CMYK\0HSV\0CIE XYZ\0YUV\0YCoCg\0");
	m_cb_model.Fill(m_cb_model.GetItem(4), L"YCbCr\0YIQ\0YDbDr\0");
	m_cb_model.Fill(m_cb_model.GetItem(3), L"L*a*b*\0xyY\0Luv\0LCH(ab)\0LCH(uv)\0LMS\0Hunter L,a,b\0");
	m_cb_model.Fill(m_cb_model.GetItem(2), L"HSL\0HSLA\0HSI\0HWB\0");
	m_cb_model.Fill(m_cb_model.GetItem(1), L"CMY\0");
	m_cb_model.Fill(m_cb_model.GetItem(0), L"RGBA\0GrayScale\0");
	m_cb_model.SetCurSel(static_cast<int>(m_bi.clr.type));
	// Обновим текстовые поля
	m_controls.FillNumberEdit(ID_EDIT_WIDTH,  L"%i", m_bi.img.uWidth);
	m_controls.FillNumberEdit(ID_EDIT_HEIGHT, L"%i", m_bi.img.uHeight);
	m_controls.FillNumberEdit(ID_EDIT_STRIDE, L"%i", m_bi.uStride);
	m_controls.FillNumberEdit(ID_EDIT_OFFSET, L"%i", m_bi.uOffset);
	m_controls.FillNumberEdit(ID_EDIT_SKIP,	  L"%i", m_bi.uSkip);
	// Установим вывод для конвертера
	m_converter.SetOutputText(m_controls[ID_EDIT_OUTPUT]);
	m_converter.SetOutputProgress(m_data.hWnd);
	// Рассчитаем позиции элементов управления и отобрази нужные
	OnSizing(DLG_W, DLG_H);
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

void CPluginDialog::ShowImage(BOOL bShow) {
	if (!m_bi.flags.bShowAsRGB)
		return;
	if (bShow)
		m_converter.SetOutputViewer(&m_viewer);
	else {
		m_viewer.Clear();
		m_converter.SetOutputViewer(nullptr);
	}

	RECT rc;
	GetWindowRect(m_data.hWnd, &rc);
	int nHeight = DLG_H;
	if (bShow) {
		int h = m_viewer.GetViewerRect().Height;
		if (h <= 0)
			h = DLG_H;
		nHeight += h+GAP;
	}
	::SetWindowPos(m_data.hWnd, nullptr, 0, 0, rc.right-rc.left, nHeight, SWP_NOMOVE|SWP_NOZORDER);

	if (bShow)
		UpdateOutput(FALSE, FALSE);
}

void CPluginDialog::OnSizing(int w, int h) {
	UINT uFlags = SWP_NOACTIVATE | SWP_NOZORDER;
	int x[] = { w-GAP-329, w+GAP-182, w-GAP-140, w-GAP-100, w-GAP-79, w-GAP-28 };
	for (UINT i = 0; i < m_controls.GetCount()-NOMOVE_CTRL_COUNT; i++)
		SetWindowPos(m_controls.GetAt(i+NOMOVE_CTRL_COUNT), nullptr, x[i], (i == 0 ? 7 : 11), 0, 0, uFlags | SWP_NOSIZE);
	if (m_converter.HasViewer()) {
		int y = DLG_H-CTRL_H_OFFSET;
		m_viewer.SetViewerRect(GAP, y, w-GAP, h-y-GAP, ImageAlignment::CENTER);
		h = y;
	}
	SetWindowPos(m_controls[ID_EDIT_OUTPUT], nullptr, GAP, 31, w-GAP-3, h-GAP-31, uFlags);
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
		m_bi.flags.bExtended = (SendMessage(reinterpret_cast<HWND>(lParam), BM_GETCHECK, NULL, NULL) == BST_CHECKED);
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

void CPluginDialog::OnUpdateSettings(COLOR_CONVERTER_SETTINGS* pCSS) {
	//m_bi.clr.converter.RGBA_SetBackgroundColor(reinterpret_cast<>(pCSS->bkg))
}

void CPluginDialog::UpdateOutput(BOOL bCheckKeyState, BOOL bChangeMethod) {
	if (bCheckKeyState && GetAsyncKeyState(VK_CONTROL))
		return;

	if (bChangeMethod)
		m_converter.ChangeMethods();
	m_converter.Read();
	if (m_viewer.HasImage())
		m_viewer.Invalidate(m_data.hWnd);
}

// Алгоритм всегда обеспечивает w*h >= cnt, где (w*h)-cnt = minimum
void CPluginDialog::CalcImageSize(const ULONGLONG& cnt, UINT& w, UINT& h) {
	double px = sqrt(ceil(cnt/m_converter.GetBytesPerColor()));
	h = static_cast<size_t>(px);
	if (fmod(px, 1.0) != 0.0)	w = static_cast<size_t>(round(px)+1.0);
	else						w = h;
}

void CPluginDialog::SetBytes(BYTE* bytes, ULONG length, BOOL bIsWideChar) {
	if (m_bi.bytes)
		delete[] m_bi.bytes;
	// Конвертируем строку в MultiByte
	if (bIsWideChar) {
		char* buf = new char[length];
		wchar_t* ptr = reinterpret_cast<wchar_t*>(bytes); 
		WideCharToMultiByte(CP_ACP, 0, ptr, length, buf, length, NULL, NULL);
		m_bi.bytes = reinterpret_cast<BYTE*>(buf);
	}
	else
		m_bi.bytes = bytes;
	m_bi.uLength = length;
	// Дополнительные настройки
	if (m_bResetOffset) {
		m_bi.uOffset = 0;
		SetWindowText(m_controls[ID_EDIT_OFFSET], L"0");
	}
	if (m_bi.img.bAutoSize) {
		CalcImageSize(m_bi.uLength, m_bi.img.uWidth, m_bi.img.uHeight);
		m_controls.FillNumberEdit(ID_EDIT_WIDTH,  L"%u", m_bi.img.uWidth);
		m_controls.FillNumberEdit(ID_EDIT_HEIGHT, L"%u", m_bi.img.uHeight);
	}
	// Преобразуем байты
	UpdateOutput(FALSE, FALSE);
}

#include "PluginDialog_Menu.inl"