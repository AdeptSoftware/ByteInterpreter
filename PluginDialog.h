#pragma once
#include "exports.h"
#include "Converter.h"
#include "ComboBoxTree.h"
#include "SettingDialog.h"


class CPluginDialog : public CWnd, IPluginDialog {
public:
	HWND Create(HWND hWndParent, HICON hIcon);
	CPluginDialog(HINSTANCE hInstDLL);
	~CPluginDialog();

	virtual BOOL WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	virtual void OnPaint(HDC hDC, RECT& rc);

	// bytes - должны быть динамически выделены!
	void SetBytes(BYTE* bytes, ULONG length, BOOL bIsWideChar = TRUE);
	void UpdateOutput(BOOL bCheckKeyState = TRUE, BOOL bChangeMethod = TRUE);

private:
	// Расчет позиции окна, cмещенного от верхнего правого угла экрана
	POINT CalcWindowRect(LONG w, LONG h, LONG offsetRT);
	BOOL CreateControls();
	// w, h - ширина и высота окна!
	void OnSizing(int w, int h);
	void OnCommand(LPARAM lParam, UINT uCtrlID, UINT uNotify, void* pData = nullptr);
	void OnMenuCommand(UINT uID, BOOL arg = TRUE);
	void OnKeyDown(WPARAM code, LPARAM cnt);

	void OnChangeValue(UINT uCtrlID, UINT uValue);
	void OnUpdateSettings(COLOR_CONVERTER_SETTINGS* pCSS);

	void ShowDataControls();
	void ShowColorControls();

	void ShowImage(BOOL bShow);

	// Алгоритм всегда обеспечивает w*h >= cnt, где (w*h)-cnt = minimum
	void CalcImageSize(const ULONGLONG& cnt, UINT& w, UINT& h);

private:
	BI::OPTIONS		m_bi;
	CSettingDialog	m_settings;
	CConverter		m_converter;
	CImageViewer	m_viewer;

	CWndRegistrator m_wrPluginDialog;
	CControlManager m_controls;
	CComboBoxTree	m_cb_model;

	HMENU			m_hMenu			= nullptr;
	HFONT			m_hFont			= nullptr;

	BOOL			m_bHideWindow	= TRUE;
	BOOL			m_bResetOffset	= TRUE;
};

