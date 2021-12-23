#pragma once
#include <string>
#include "ImageViewer.h"
#include "BI\ByteInterpreter.h"
#define BI ByteInterpreter


class CConverter : public BI::CAbstractByteInterpreter {
public:
	CConverter(const BI::OPTIONS& bi);
	~CConverter();

	// hWnd - окно куда будет выводиться текст прогресса
	void SetOutputText(HWND hWnd);
	void SetOutputProgress(HWND hWnd);
	void SetOutputViewer(CImageViewer* pViewer);

	BOOL HasViewer() const;
	double GetBytesPerColor();

protected:
	virtual void OnInitialize();
	virtual void OnFirstIteration();
	virtual void OnIteration();
	virtual void OnEndOfBytes();

	void UpdateImage();
	void UpdateProgressText();
	void PrintBytes();

private:
	std::wstring	 m_szOutputText;
	HWND			 m_hWndOutputText		= nullptr;
	HWND			 m_hWndOutputProgress	= nullptr;
	
	CImageViewer*	 m_pOutputViewer		= nullptr;
	Gdiplus::Bitmap* m_pImage				= nullptr;
	POINT			 m_pos					= { 0, 0 };

	float			 m_fProgress			= 0.0;
	wchar_t			 m_szSeparator[BI::BUFFER_SIZE]{ 0 };
};