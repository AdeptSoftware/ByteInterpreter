#pragma once
#include "Window.h"
#include "ControlManager.h"

const UINT WM_SETTING_UPDATE = WM_USER+10;
enum class SettingMode : int { NONE = -1, YUV_CONVERSION_MATRIX = 0, BKG_COLOR };

struct COLOR_CONVERTER_SETTINGS {
	int			nStandard;
	BYTE		bkg[3];
	double		dAngle;
	double		W[5];
};

class CSettingDialog : public CWnd {
	COLOR_CONVERTER_SETTINGS	m_backup, m_ccs;
	CWndRegistrator				m_wrBISettingDialog;
	CControlManager				m_controls;
	HWND						m_hWndParent		= nullptr;
	SettingMode					m_ctrl_type[12];
	SettingMode					m_mode;
	
public:
	CSettingDialog(HFONT* hFontPtr);
	virtual BOOL WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnPaint(HDC hDC, RECT& rc);

	BOOL Init(HWND hWndParent);
	void Show(SettingMode mode);

protected:
	void OnCancelOK(BOOL bOK = TRUE);
	void SetStandard(int nIndex);
	void RefreshEditControls();
};

