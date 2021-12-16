#pragma once
#include "pch.h"
#include <type_traits>

// �������������� enum class E : typename T � �������� ���� T
template <typename E> constexpr auto e_cast(E e) noexcept {
	return static_cast<std::underlying_type_t<E>>(e);
}

namespace ByteInterpreter {
	const double BYTEMAX = 255.0;
	// ����������� ����������������� �����������
	enum class XYZ_OBSERVER : int { CIE1931_2, CIE1964_10 };
	// ����������� �������� �����
	enum class XYZ_ILLUMINANT : int { A, B, C, D50, D55, D65, D75, E, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12 };

namespace Colors {
	// ����� ������������� ���������
	enum class XYZ_METHOD : int { LINEAR, BRADFORD, VON_KRIES };
	enum class LMS_CONVERSION : int { LMS2XYZ_D65_2, LMS2XYZ_E_2, LMS2XYZ_F_2, CAT97S_LINEAR, CAT97S_BRADFORD, CAT02, XYB_JPEG_XL };

	struct _REFERENCE_WHITE_COMPONENTS {
		// ���������
		XYZ_OBSERVER	stdObserver = XYZ_OBSERVER::CIE1931_2;	// ������� ����
		XYZ_ILLUMINANT	illuminant = XYZ_ILLUMINANT::D65;		// ���������
	};

#define IMPLEMENT_COLOR(N, n1, n2, n3)	  struct N { double n1 = 0.0, n2 = 0.0, n3 = 0.0; }
#define IMPLEMENT_COLOR_RW(N, n1, n2, n3) struct N : _REFERENCE_WHITE_COMPONENTS { double n1 = 0.0, n2 = 0.0, n3 = 0.0; }
	// �������� HSL � ��������� [0.0..1.0], ��� �������. Hue [0.0..360.0)deg; Saturation � Lightness [0.0..100.0]%
	IMPLEMENT_COLOR(HSL, H, S, L);
	// �������� HSV � ��������� [0.0..1.0], ��� �������. Hue [0.0..360.0)deg; Saturation � Value/Brightness [0.0..100.0]%
	IMPLEMENT_COLOR(HSV, H, S, V);
	// �������� HSI � ��������� [0.0..1.0], ��� �������. Hue [0.0..360.0)deg; Saturation � Intensity [0.0..100.0]%
	IMPLEMENT_COLOR(HSI, H, S, I);
	// �������� HWB � ��������� [0.0..1.0], ��� �������. Hue [0.0..360.0)deg; Whiteness � Blackness [0.0..100.0]%
	IMPLEMENT_COLOR(HWB, H, W, B);

	// Y � ��������� [0.0..1.0]. �������� UV ������� �� ������������� ��������� (��.��������� � YUV_ConversionData)
	IMPLEMENT_COLOR(YUV, Y, U, V);
	// �� ��������� � YUV!
	// �������� YCoCg � ��������� [0.0..1.0] [-max..max] [-max..max], ��� max = 0.5 (��� R-�������� = 1.0)
	IMPLEMENT_COLOR(YCOCG, Y, Co, Cg);
	// �������� ������ ���� � ��������� �� [0.0..1.0]
	IMPLEMENT_COLOR(SRGB, R, G, B);
	// �������� CMY � ��������� [0.0..1.0]
	IMPLEMENT_COLOR(CMY, C, M, Y);

	// �������� RGBA � ��������� [0.0..1.0]
	struct RGBA : SRGB { double A = 0.0; };
	// �������� HSLA � ��������� [0.0..1.0]
	struct HSLA : HSL { double A = 0.0; };
	// �������� CMYK � ��������� [0.0..1.0]
	struct CMYK : CMY { double K = 0.0; };

	// X,Y,Z ������� �� Reference White.
	// ��� D65/2: X,Y,Z = [0.0..0.95047] [0.0..1.0] [0.0..1.08883] - �������. R,G,B = [0.0..1.0]
	IMPLEMENT_COLOR_RW(XYZ, X, Y, Z);
	// �������� Hunter L,a,b � ��������� = [0.0..1.0] ~[-0.7..1.1] ~[-2.0..0.56]
	IMPLEMENT_COLOR_RW(HLAB, L, a, b);
	// �������� CIE L*a*b � ��������� = [0.0..100.0] ~[-90.0..100.0] ~[-115.0..100.0]
	IMPLEMENT_COLOR_RW(LAB, L, a, b);
	// �������� CIE L*CH � ��������� [0.0..100.0] [0.0..~180.0] [0.0..360.0]
	IMPLEMENT_COLOR_RW(LCH, L, C, H);
	// �������� CIE L*uv � ��������� [0.0..100.0] ~[-85.0..177.0] ~[-135.0..110.0]
	IMPLEMENT_COLOR_RW(LUV, L, u, v);
	// �������� xyY � ��������� ~[ 0.17..1.0] [~0.06..1.0] [0.0..1.0]
	IMPLEMENT_COLOR_RW(XYY, x, y, Y);
	// ��������	LMS � ��������� ������� �� ������������ LMS_CONVERSION...
	IMPLEMENT_COLOR_RW(LMS, L, M, S);

// ========= ========= ========= ========= ========= ========= ========= =========

// SDTV ���������� BT.470 � BT.601; HDTV - BT.709; UHDTV - BT.2020 || HDR-TV - BT.2100
// BT.2020 � BT.2100 ���������� ���� � �� �� �������� ����� (������ BT.2020 ����� ������������ BT.2100)
enum class BT_STANDARD : int { CUSTOM = -1, BT470 = 0, BT601, BT709, BT2100 };

// ========= ========= ========= ========= ========= ========= ========= =========

class CColorConverter {
protected:
	// ���� � ������� ����������� ��� �������������� �/�� ��������� ������������ XYZ
	double			m_TransformMatrixXYZtoRGB[3][3];
	double			m_TransformMatrixRGBtoXYZ[3][3];
	XYZ_METHOD		m_xyzChromaticAdaptationMethod;
	XYZ_ILLUMINANT	m_xyzIlluminant;
	XYZ_OBSERVER	m_xyzObserer;

	// YUV, YIQ, ...
	double			m_TransformMatrixYUVtoRGB[3][3];
	double			m_TransformMatrixRGBtoYUV[3][3];
	BT_STANDARD     m_yuvStandardBT = BT_STANDARD::BT470;

	// ��� ����������� RGBA
	SRGB             m_clrBackground = { 1.0, 1.0, 1.0 };

public:
	CColorConverter();

	// ========= ========= ========= ========= ========= ========= ========= =========

	void XYZ_UseAdaptationMethod(XYZ_METHOD method);
	const double* XYZ_GetReferenceWhite(const XYZ_OBSERVER& stdObserver, const XYZ_ILLUMINANT& illuminant);

	// � out �������������� ����� ��������� "������� ����" � "���������"
	void XYZ_Convert(XYZ& out, const XYZ& in);
	// ������������ XYZ D65/2 � RGB
	void XYZD65toRGB(SRGB& out, const XYZ& in);
	// ������������ RGB � XYZ D65/2
	void RGBtoXYZD65(XYZ& out, const SRGB& in);

	// ========= ========= ========= ========= ========= ========= ========= =========

	void xyY2XYZ(XYZ& out, const XYY& in);
	void XYZ2xyY(XYY& out, const XYZ& in);
	void XYZ2HunterLab(HLAB& out, const XYZ& in);
	void HunterLab2XYZ(XYZ& out, const HLAB& in);
	void XYZ2CIELAB(LAB& out, const XYZ& in);
	void CIELAB2XYZ(XYZ& out, const LAB& in);
	void XYZ2CIELUV(LUV& out, const XYZ& in);
	void CIELUV2XYZ(XYZ& out, const LUV& in);

	void CIELAB2CIELCHab(LCH& out, const LAB& in);
	void CIELCHab2CIELAB(LAB& out, const LCH& in);
	void CIELUV2CIELCHuv(LCH& out, const LUV& in);
	void CIELCHuv2CIELUV(LUV& out, const LCH& in);

	void XYZD65toLMS(LMS& out, const XYZ& in, const LMS_CONVERSION c);
	void LMStoXYZ(XYZ& out, const LMS& in, const LMS_CONVERSION c);

	// ========= ========= ========= ========= ========= ========= ========= =========

	// �������� �� ������������
	// ����� ����� (A) - ����� �������� [0.0..1.0]
	void HSLA2HSL(HSL& out, const HSLA& in);
	// ����� ����� (A) - ����� �������� [0.0..1.0]
	void RGBA2RGB(SRGB& out, const RGBA& in);
	// ����� ����� (A) - ����� �������� [0.0..1.0]
	COLORREF RGBA2RGB(const RGBA& in);
	// �������� lightness [0.0..1.0]
	static void GrayScale2RGB(SRGB& out, const double lightness);

	void RGBA_SetBackgroundColor(const SRGB& clr);

	// ========= ========= ========= ========= ========= ========= ========= =========

	static void HSV2RGB(SRGB& out, const HSV& in);
	static void HSL2RGB(SRGB& out, const HSL& in);
	static void HSI2RGB(SRGB& out, const HSI& in);

	static void RGB2HSV(HSV& out, const SRGB& in);
	static void RGB2HSL(HSL& out, const SRGB& in);
	static void RGB2HSI(HSI& out, const SRGB& in);

	static void HSV2HWB(HWB& out, const HSV& in);
	static void HWB2HSV(HSV& out, const HWB& in);

	// ========= ========= ========= ========= ========= ========= ========= =========

	static void RGB2CMY(CMY& out, const SRGB& in);
	static void CMY2RGB(SRGB& out, const CMY& in);
	static void CMYK2RGB(SRGB& out, const CMYK& in);
	static void CMYK2CMY(CMY& out, const CMYK& in);
	static void CMY2CMYK(CMYK& out, const CMY& in);
	
	// ========= ========= ========= ========= ========= ========= ========= =========

	static void YCoCg2RGB(SRGB& out, const YCOCG& in, BOOL bYCoCgR);
	static void RGB2YCoCg(YCOCG& out, const SRGB& in, BOOL bYCoCgR);

	// ========= ========= ========= ========= ========= ========= ========= =========

	void YUV_UseBTStandard(BT_STANDARD bt);

	// W: Wr, Wb - ��������� (?���������� �����?); Umax � Vmax - ����. �������� ��������������� �����������
	// YDbDr (SECAM): W { 0.299, 0.587, 0.114, 1.333, -1.333 }; -> Y = [0.0..1.0]; Dr, Db = [+/-1.333]
	void YUV_CreateTransformMatrix(const double W[5]);
	
	// �������� Umax = |m_TransformMatrixRGBtoYUV[2][1]|; Vmax = |m_TransformMatrixRGBtoYUV[1][0]|
	// YIQ: 33 deg -> Y = [0.0..1.0]; I = [+/-0.5226]; Q = [+/-0.5957]
	void YUV_RotateTransformMatrix(const double deg);

	// ������ ��� YCbCr: YUV2RGB(Y - (bFullRange ? 0.0 : 16.0/255), Cb-0.5, Cr-0.5)
	void YUV2RGB(SRGB& out, const YUV& in);

	// ������ ��� YCbCr: RGB2YUV(R, G, B) �	out += { (bFullRange ? 0.0 : 16.0/255), 0.5, 0.5 }
	void RGB2YUV(YUV& out, const SRGB& in);

private:
	// XYZ D65/2 deg (RGB Working Space: sRGB)
	void XYZ_CreateTransformationMatrix();
};

} // END namespace ByteInterpreter::Colors
} // END namespace ByteInterpreter

/*
* ������������� YUV � �� ���������
* Y'UV444	RGB <- YUV2RGB()	 <- Bytes[03] = { Y, U, V }
* Y'UV422	RGB <- YUV2RGB() x02 <- Bytes[04] = { U, Y1, V, Y2 }
* Y'UV411	RGB <- YUV2RGB() x04 <- Bytes[06] = { U, Y1, Y2, V, Y3, Y4 }
* Y'UV420p  RGB <- YUV2RGB() x24 <- Bytes[36] = { Y1..Y24, U1..U6, V1..V6 }
* Y'UV420sp // en.wikipedia.org/wiki/YUV#Y%E2%80%B2UV420sp_(NV21)_to_RGB_conversion_(Android)
*/

/*
* YUV = YPbPr = ������� Y'CbCr �� ������������ � �������� �������� ��� �������� �������� � �������� �����
* YPbPr - ��� ����������� ������������� �����
* YCbCr - ��� �����������/��������� ������������� �����
* YDbDr - ���������� (SECAM)
* YIQ - �������
*/
