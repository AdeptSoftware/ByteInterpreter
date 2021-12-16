#pragma once
#include "Window.h"
#include "exports.h"
#include "ComboBoxTree.h"
#include "ControlManager.h"
#include "Converter.h"


class CPluginDialog : public CWnd, IPluginDialog {
public:
	HWND Create(HWND hWndParent, HICON hIcon);
	CPluginDialog(HINSTANCE hInstDLL);
	~CPluginDialog();

	virtual BOOL WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	void OnPaint(HDC hDC, RECT& rc);

	// bytes - должны быть динамически выделены!
	void SetBytes(BYTE* bytes, ULONG length, BOOL bIsWideChar = TRUE);
	void UpdateOutput(BOOL bCheckKeyState = TRUE);

private:
	// Расчет позиции окна, cмещенного от верхнего правого угла экрана
	POINT CalcWindowRect(LONG w, LONG h, LONG offsetRT);
	BOOL CreateControls();
	// w, h - ширина и высота окна!
	void OnSizing(int w, int h);
	void OnChangeValue(UINT uCtrlID, UINT uValue);
	void OnCommand(LPARAM lParam, UINT uCtrlID, UINT uNotify, void* pData = nullptr);
	void OnMenuCommand(UINT uID, BOOL arg = TRUE);
	void OnKeyDown(WPARAM code, LPARAM cnt);

	void ShowDataControls();
	void ShowColorControls();

private:
	BI::OPTIONS		m_bi;
	CConverter		m_converter;

	CWndRegistrator m_wrPluginDialog;
	CControlManager m_controls;
	CComboBoxTree	m_cb_model;

	HMENU			m_hMenu			= nullptr;
	HFONT			m_hFont			= nullptr;

	BOOL			m_bHideWindow	= TRUE;
	BOOL			m_bResetOffset	= TRUE;
};

