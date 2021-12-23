#include "pch.h"
#include <math.h>
#include <Windowsx.h>
#include "ImageViewer.h"
#include <gdiplusinit.h>
using namespace Gdiplus;
#pragma comment(lib, "Gdiplus.lib")

// ========== ========== ========== ========== ========== ========== ========== ==========

CImageViewer::CImageViewer(BOOL bFrameMode) 
	: m_bFrameMode(bFrameMode) {
	GdiplusStartupInput si;
	GdiplusStartup(&m_gdipToken, &si, nullptr);
}

CImageViewer::~CImageViewer() {
	GdiplusShutdown(m_gdipToken);
}

void CImageViewer::Clear() {
	m_pImage = nullptr;
}

void CImageViewer::SetImage(Image* pImage) {
	m_pImage = pImage;
	if (pImage) {
		m_ptOffset = { 0, 0 };
		m_rcImage  = { 0, 0, static_cast<INT>(pImage->GetWidth()), static_cast<INT>(pImage->GetHeight()) };
		ReCalculate();
	}
}

void CImageViewer::SetViewerRect(INT x, INT y, INT w, INT h, ImageAlignment align) {
	m_rcViewer = { x, y, w, h };
	m_align = align;
	ReCalculate();
}

INT CImageViewer::GetPixelSize() {
	INT maximum = max(m_rcImage.Width, m_rcImage.Height);
	return static_cast<INT>(ceil(static_cast<double>(m_rcViewer.Height)/maximum));
}

void CImageViewer::ReCalculate() {
	if (m_rcImage.Height == 0) {
		m_rcPaint = { 0, 0, 0, 0 };
		return;
	}
	m_rcPaint = m_rcViewer;
	m_rcPaint.Width = m_rcImage.Width*GetPixelSize();
	if (!m_bFrameMode)
		m_rcPaint.Y = 0;

	switch (m_align) {
	case ImageAlignment::CENTER:
		m_rcPaint.X = (m_rcViewer.Width-m_rcViewer.Height)/2;
		break;
	case ImageAlignment::RIGHT:
		m_rcPaint.X = m_rcViewer.Width-m_rcPaint.Width;
		break;
	default:
		m_rcPaint.X = m_rcViewer.X;
	}
}

const Rect& CImageViewer::GetViewerRect() const {
	return m_rcViewer;
}

const Rect& CImageViewer::GetImageRect() const {
	return m_rcImage;
}

BOOL CImageViewer::HasImage() const {
	return (m_pImage != nullptr);
}

void CImageViewer::Draw(HDC hDC) {
	if (m_pImage) {
		Rect rc;
		Graphics g(hDC);
		SolidBrush brush(Color::White);
		if (m_bFrameMode) rc = { 0, 0, m_rcPaint.Width,  m_rcPaint.Height };
		else			  rc = { 0, 0, m_rcViewer.Width, m_rcViewer.Height };
		Bitmap* pImage = new Bitmap(rc.Width, rc.Height, m_pImage->GetPixelFormat());
		Graphics* pMem = Graphics::FromImage(pImage);
		pMem->SetPixelOffsetMode(PixelOffsetMode::PixelOffsetModeHalf);
		pMem->SetInterpolationMode(InterpolationMode::InterpolationModeNearestNeighbor);
		pMem->FillRectangle(&brush, rc);
		if (m_bFrameMode) {
			rc.X = m_ptOffset.X;
			rc.Y = m_ptOffset.Y;
			pMem->DrawImage(m_pImage, rc, m_rcImage.X, m_rcImage.Y, m_rcImage.Width, m_rcImage.Height, Unit::UnitPixel);
			g.DrawImage(pImage, m_rcPaint);
		}
		else {
			rc = m_rcPaint;
			rc.X += m_ptOffset.X;
			rc.Y += m_ptOffset.Y;
			pMem->DrawImage(m_pImage, rc, m_rcImage.X, m_rcImage.Y, m_rcImage.Width, m_rcImage.Height, Unit::UnitPixel);
			g.DrawImage(pImage, m_rcViewer.X, m_rcViewer.Y);
		}
		delete pMem;
		delete pImage;
	}
}

void CImageViewer::Invalidate(HWND hWndParent) {
	RECT rc = { m_rcViewer.X, m_rcViewer.Y, m_rcViewer.GetRight(), m_rcViewer.GetBottom() };
	InvalidateRect(hWndParent, &rc, FALSE);
}

void CImageViewer::SetOffset(INT x, INT y) {
	m_ptOffset.X -= x;
	m_ptOffset.Y -= y;
}

void CImageViewer::SetZoom(INT nPixels) {
	Rect& rc = (m_bFrameMode ? m_rcImage : m_rcPaint);
	if (!m_bFrameMode)
		nPixels *= GetPixelSize();
	if (rc.Width-(nPixels*2) < 1)
		nPixels = (rc.Width-1)/2;
	if (rc.Height-(nPixels*2) < 1)
		nPixels = (rc.Height-1)/2;
	rc.X       += nPixels;			// на px смещается с края
	rc.Y       += nPixels;
	rc.Width   -= nPixels*2;		// на 2 уменьшается ширина
	rc.Height  -= nPixels*2;
}

BOOL CImageViewer::OnMessage(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_MOUSEWHEEL) {
		int zDelta = GET_WHEEL_DELTA_WPARAM(wParam)/120;
		SetZoom(zDelta);
		Invalidate(hWnd);
	}
	else if (uMsg == WM_KEYDOWN) {
		if (wParam >= VK_LEFT && wParam <= VK_DOWN) {
			INT px = GetPixelSize();
			SetOffset((wParam == VK_LEFT ? -px : (wParam == VK_RIGHT ? px : 0)),
					  (wParam == VK_DOWN ? -px : (wParam == VK_UP    ? px : 0)));
			Invalidate(hWnd);
		}
	}
	else if (uMsg == WM_LBUTTONDOWN) {
		m_ptDrag = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		if ((!m_bFrameMode && m_rcViewer.Contains(m_ptDrag)) ||
			 (m_bFrameMode && m_rcPaint.Contains(m_ptDrag)))
			SetCapture(hWnd);
		else
			m_ptDrag.X = 0;
	}
	else if (uMsg == WM_MOUSEMOVE && m_ptDrag.X) {
		INT x = GET_X_LPARAM(lParam);
		INT y = GET_Y_LPARAM(lParam);
		SetOffset(m_ptDrag.X-x, m_ptDrag.Y-y);
		m_ptDrag = { x, y };
		Invalidate(hWnd);
	}
	else if (uMsg == WM_LBUTTONUP && m_ptDrag.X) {
		m_ptDrag.X = 0;
		ReleaseCapture();
	}
	return TRUE;
}