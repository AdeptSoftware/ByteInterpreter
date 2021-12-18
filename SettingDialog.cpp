#include "pch.h"
#include "SettingDialog.h"
#include "control_constants.h"
#include "ColorConverter.h"

const wchar_t	CLASS_NAME[]	= L"BISettingDialog";
const wchar_t	FMT_DOUBLE[]	= L"%.3f";
const UINT		OFFSET			= 92;
const UINT		DLG_W			= 372;
const UINT		DLG_H_YUV_MODE	= 125;
const UINT		DLG_H_BKG_MODE	= 27;

const UINT		CB_W			= 100;
const UINT		BTN_W			= 70;
const UINT		EDIT_W			= 38;
const UINT		CTRL_H			= 20;
const UINT		DIST			= 3;
const UINT		GAP				= 73;

const UINT		DLG_BTN_CNT		= 2;
const UINT		CTRL_BKG_CNT	= 3;
const UINT		CTRL_MAT_CNT	= 7;

const UINT		SZSTRUCT		= sizeof(COLOR_CONVERTER_SETTINGS);

// ========= ========= ========= ========= ========= ========= ========= =========

double CheckEdit(HWND hEdit, double dMin = 0.0, double dMax = 0.0, LPCWSTR fmt = L"%.0lf") {
	wchar_t buf[32];
	GetWindowTextW(hEdit, buf, 32);
	double dValue = _wtof(buf);
	if (dMin != dMax) {
		if (dValue < dMin)		dValue = dMin;
		else if (dValue > dMax)	dValue = dMax;
	}
	swprintf_s(buf, fmt, dValue);
	SetWindowTextW(hEdit, buf);
	return dValue;
}

BOOL CSettingDialog::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_COMMAND) {
		UINT uID = LOWORD(wParam);
		UINT uNotify = HIWORD(wParam);
		if ((uID == ID_SD_BTN_CANCEL || uID == ID_SD_BTN_OK) && uNotify == BN_CLICKED)
			OnCancelOK(uID == ID_SD_BTN_OK);
		else if (uNotify == EN_KILLFOCUS) {
			if (uID >= ID_SD_EDIT_BKG_R && uID <= ID_SD_EDIT_BKG_B)
				m_ccs.bkg[uID-ID_SD_EDIT_BKG_R] = static_cast<BYTE>(CheckEdit(m_controls[uID], 0.0, 255.0));
			else if (uID == ID_SD_EDIT_M_ANGLE) {
				m_ccs.dAngle = CheckEdit(m_controls[uID], 0.0, 360.0, L"%.1lf");
				Invalidate(TRUE);
			}
			else if (uID >= ID_SD_EDIT_WR && uID <= ID_SD_EDIT_VMAX) {
				m_ccs.W[uID-ID_SD_EDIT_WR] = CheckEdit(m_controls[uID], 0.0, 0.0, FMT_DOUBLE);
				SendMessageW(m_controls[ID_SD_CB_BTSTANDART], CB_SETCURSEL, 0, NULL);
				Invalidate(TRUE);
			}
		}
		else if (uID == ID_SD_CB_BTSTANDART && uNotify == CBN_SELENDOK) {
			int nIndex = SendMessageW(m_controls[ID_SD_CB_BTSTANDART], CB_GETCURSEL, 0, NULL);
			SetStandard(nIndex);
		}
	}
	else if (uMsg == WM_LBUTTONDOWN)
		::SendMessage(m_data.hWnd, WM_SYSCOMMAND, SC_MOVE|HTCAPTION, 0);
	else if (uMsg == WM_KEYDOWN && (wParam == VK_ESCAPE || wParam == VK_RETURN))
		OnCancelOK(wParam == VK_RETURN);
	return TRUE;
}

void CSettingDialog::OnPaint(HDC hDC, RECT& rc) {
	HFONT hOldFont = (HFONT)SelectObject(hDC, m_controls.GetFont());
	COLORREF clrOldBackground = SetBkColor(hDC, RGB(255, 255, 255));
	if (m_mode == SettingMode::BKG_COLOR)
		TextOutW(hDC, DIST, DIST+2,	L"Background color:", 17);
	else if (m_mode == SettingMode::YUV_CONVERSION_MATRIX) {
		TextOutW(hDC, DIST, DIST+3,	L"BT standard:", 12);
		TextOutW(hDC, DIST, 50,		L"YUV2RGB Matrix:", 15);
		TextOutW(hDC, 175,  DIST+3,	L"Rotate matrix, deg:", 19);
		wchar_t text[5][32] = { L"└ Wr:", L"    Wg:", L"    Wb:", L"Umax:", L"Vmax:" };
		for (UINT i = 0; i < CTRL_MAT_CNT-2; i++)
			TextOutW(hDC, DIST+(i*GAP),	(DIST*3)+CTRL_H, text[i], wcslen(text[i]));

		int nMinusAlign = 0;
		int nCenter = (DLG_W-(55*3)-(2*DIST))/2;
		wchar_t buf[16]{ 0 };
		double out[3][3]{ 0.0 };
		ByteInterpreter::Colors::CColorConverter conv;
		conv.YUV_CreateTransformMatrix(m_ccs.W);
		conv.YUV_RotateTransformMatrix(m_ccs.dAngle);
		conv.YUV_GetTransformMatrix(reinterpret_cast<double*>(out), TRUE);
		for (UINT i = 0; i < 3; i++) {
			for (UINT j = 0; j < 3; j++) {
				nMinusAlign = (out[i][j] < 0.0) ? 0 : 4;
				swprintf_s(buf, L"%.5f", out[i][j]);
				TextOutW(hDC, nCenter+(55*i)+nMinusAlign, 35+(15*(j+1)), buf, wcslen(buf));
			}
		}

	}
	SetBkColor(hDC, clrOldBackground);
	SelectObject(hDC, hOldFont);
}

void CSettingDialog::OnCancelOK(BOOL bOK) {
	EnableWindow(m_hWndParent, TRUE);
	ShowWindow(SW_HIDE);
	if (bOK) {
		::SendMessage(m_hWndParent, WM_SETTING_UPDATE, 0, reinterpret_cast<LPARAM>(&m_ccs));
		memcpy_s(&m_backup, SZSTRUCT, &m_ccs, SZSTRUCT);
	}
	else {
		memcpy_s(&m_ccs, SZSTRUCT, &m_backup, SZSTRUCT);
		RefreshEditControls();
	}
}

// ========= ========= ========= ========= ========= ========= ========= =========

CSettingDialog::CSettingDialog(HFONT* hFontPtr) :
	m_wrBISettingDialog(CLASS_NAME, DLGWINDOWEXTRA),
	m_controls(&m_data, ID_SD_BTN_CANCEL, hFontPtr) {
	UINT i = 0;
	for (UINT j = 0; j < DLG_BTN_CNT;  j++, i++) m_ctrl_type[i] = SettingMode::NONE;
	for (UINT j = 0; j < CTRL_BKG_CNT; j++, i++) m_ctrl_type[i] = SettingMode::BKG_COLOR;
	for (UINT j = 0; j < CTRL_MAT_CNT; j++, i++) m_ctrl_type[i] = SettingMode::YUV_CONVERSION_MATRIX;

	m_ccs.bkg[0]	= m_ccs.bkg[1] = m_ccs.bkg[2] = 255;
	m_mode		= SettingMode::NONE;
	m_ccs.dAngle	= 0.0;
}

void CSettingDialog::Show(SettingMode mode) {
	EnableWindow(m_hWndParent, FALSE);
	if (m_mode != mode) {
		m_mode = mode;
		UINT h = DLG_H_BKG_MODE;
		if (mode == SettingMode::YUV_CONVERSION_MATRIX)
			h = DLG_H_YUV_MODE;

		RECT rc;
		GetWindowRect(m_hWndParent, &rc);
		UINT uFlags = SWP_NOSIZE | SWP_NOZORDER;
		SetWindowPos(m_data.hWnd, HWND_TOPMOST, rc.left+OFFSET, rc.top+OFFSET, DLG_W, h, SWP_NOZORDER | SWP_SHOWWINDOW);
		SetWindowPos(m_controls[ID_SD_BTN_CANCEL], nullptr, DLG_W-((BTN_W+DIST)*2), h-CTRL_H-DIST-1, 0, 0, uFlags);
		SetWindowPos(m_controls[ID_SD_BTN_OK],	   nullptr, DLG_W-BTN_W-DIST,		h-CTRL_H-DIST-1, 0, 0, uFlags);
		for (UINT i = 0; i < m_controls.GetCount(); i++) {
			BOOL bShow = (m_ctrl_type[i] == SettingMode::NONE || m_ctrl_type[i] == mode);
			::ShowWindow(m_controls[ID_SD_BTN_CANCEL+i], bShow ? SW_SHOW : SW_HIDE);
		}
	}
	else
		::ShowWindow(m_data.hWnd, SW_SHOW);
}

// ========= ========= ========= ========= ========= ========= ========= =========

BOOL CSettingDialog::Init(HWND hWndParent) {
	DWORD dwStyle = WS_BORDER | WS_POPUP;
	m_data.hInst  = GetModuleHandle(nullptr);
	m_data.hWnd   = ::CreateWindowW(CLASS_NAME, L"", dwStyle, 0, 0, 0, 0, hWndParent, nullptr, m_data.hInst, nullptr);
	m_hWndParent  = hWndParent;
	if (!m_data.hWnd)
		return FALSE;
	
	dwStyle = WS_CHILD | WS_BORDER | WS_TABSTOP;
	m_controls.Create(0, 0, BTN_W, CTRL_H, L"BUTTON", dwStyle, L"Отмена");
	m_controls.Create(0, 0, BTN_W, CTRL_H, L"BUTTON", dwStyle, L"ОК");
	dwStyle |= ES_NUMBER | ES_CENTER | ES_UPPERCASE;
	for (UINT i = 0; i < CTRL_BKG_CNT; i++)
		m_controls.Create(OFFSET+DIST+(i*(EDIT_W+DIST)), DIST, EDIT_W, CTRL_H, L"EDIT", dwStyle);
	m_controls.Create(268, DIST, EDIT_W, CTRL_H, L"EDIT", dwStyle);
	dwStyle ^= ES_NUMBER;
	for (UINT i = 0; i < CTRL_MAT_CNT-2; i++)
		m_controls.Create(35+(i*GAP), (DIST*2)+CTRL_H, EDIT_W, CTRL_H, L"EDIT", dwStyle);
	dwStyle = WS_CHILD | WS_VSCROLL | CBS_DROPDOWNLIST;
	m_controls.Create(70, DIST, CB_W, CTRL_H*5, L"COMBOBOX", dwStyle);
	if (!m_controls.IsOK())
		return FALSE;
	m_controls.FillComboBox(ID_SD_CB_BTSTANDART, L"Custom\0BT.470\0BT.601\0BT.709\0BT.2100\0", 1);
	SetStandard(e_cast(ByteInterpreter::BT_STANDARD::BT470)+1);
	memcpy_s(&m_backup, SZSTRUCT, &m_ccs, SZSTRUCT);
	return TRUE;
}

// ========= ========= ========= ========= ========= ========= ========= =========

void CSettingDialog::RefreshEditControls() {
	for (UINT i = 0; i < CTRL_BKG_CNT; i++)
		m_controls.FillNumberEdit(ID_SD_EDIT_BKG_R+i, L"%hhu", m_ccs.bkg[i]);

	m_controls.FillNumberEdit(ID_SD_EDIT_M_ANGLE, L"%.1lf", 0.0);
	SendMessageW(m_controls[ID_SD_CB_BTSTANDART], CB_SETCURSEL, m_ccs.nStandard, NULL);
	for (UINT i = 0; i < CTRL_MAT_CNT-2; i++)
		m_controls.FillNumberEdit(ID_SD_EDIT_WR+i, L"%.3lf", m_ccs.W[i]);
}

void CSettingDialog::SetStandard(int nIndex) {
	if (nIndex == 0) { // Был стандарт не Custom, а стал Custom. Надо отменить изменения
		SendMessageW(m_controls[ID_SD_CB_BTSTANDART], CB_SETCURSEL, m_ccs.nStandard, NULL);
		return;
	}

	m_ccs.nStandard = nIndex;
	auto bt = static_cast<ByteInterpreter::BT_STANDARD>(nIndex-1);
	ByteInterpreter::Colors::CColorConverter::YUV_GetConversionData(m_ccs.W, bt);
	RefreshEditControls();
	Invalidate(TRUE);
}
