#pragma once
#include <ObjIdl.h>
#include <gdiplus.h>

enum class ImageAlignment : int { LEFT, CENTER, RIGHT };

class CImageViewer {
	BOOL			m_bFrameMode;
	ULONG_PTR		m_gdipToken;
	ImageAlignment	m_align			= ImageAlignment::LEFT;	// ¬ыравнивание изображени€ в области m_rcViewer

	Gdiplus::Point  m_ptOffset		= { 0, 0 };
	Gdiplus::Point  m_ptDrag		= { 0, 0 };
	Gdiplus::Image* m_pImage		= nullptr;
	Gdiplus::Rect	m_rcImage, m_rcPaint, m_rcViewer;

public:
	CImageViewer(BOOL bFrameMode = FALSE);
	~CImageViewer();

	BOOL HasImage() const;

	const Gdiplus::Rect& GetImageRect() const;
	const Gdiplus::Rect& GetViewerRect() const;

	void Clear();
	// ¬ысвобождение пам€ти класс не управл€ет!
	void SetImage(Gdiplus::Image* pImage);
	void SetViewerRect(INT x, INT y, INT w, INT h, ImageAlignment align = ImageAlignment::LEFT);

	void SetOffset(INT x, INT y);
	void SetZoom(INT nPixels);

	void Invalidate(HWND hWndParent);

	void Draw(HDC hDC);
	BOOL OnMessage(HWND hWnd, UINT uMsg, WPARAM lParam, LPARAM wParam);
private:
	void ReCalculate();
	INT  GetPixelSize();
};

