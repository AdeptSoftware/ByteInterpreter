#pragma once
#include "ByteInterpreter.h"
#include <string>


class CConverter : public BI::CAbstractByteInterpreter {
public:
	CConverter(const BI::OPTIONS& bi);

	// hWnd - окно куда будет выводиться текст прогресса
	void SetOutputText(HWND hWnd);
	void SetOutputProgress(HWND hWnd);

protected:
	virtual void OnIteration();
	virtual void OnInitialize();

	std::wstring		m_szOutputText;
	HWND				m_hWndOutputText		= nullptr;
	HWND				m_hWndOutputProgress	= nullptr;

private:
	float				m_fProgress = 0.0;
	wchar_t				m_szSeparator[BI::BUFFER_SIZE];
	BI::TOSTRING_DATA	m_buf;

	void UpdateProgressText();
	void PrintBytes();
};