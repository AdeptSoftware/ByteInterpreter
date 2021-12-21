#pragma once

namespace ByteInterpreter {
	// Стандартный колориметрический наблюдатель
	enum class XYZ_OBSERVER : int { CIE1931_2, CIE1964_10 };
	// Стандартный источник света
	enum class XYZ_ILLUMINANT : int { A, B, C, D50, D55, D65, D75, E, F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12 };
	// Метод хроматической адаптации
	enum class XYZ_METHOD : int { LINEAR, BRADFORD, VON_KRIES };
	enum class LMS_CONVERSION : int { LMS2XYZ_D65_2, LMS2XYZ_E_2, LMS2XYZ_F_2, CAT97S_LINEAR, CAT97S_BRADFORD, CAT02, XYB_JPEG_XL };

	// ========= ========= ========= ========= ========= ========= ========= =========

	// SDTV использует BT.470 и BT.601; HDTV - BT.709; UHDTV - BT.2020 || HDR-TV - BT.2100
	// BT.2020 и BT.2100 используют одну и ту же цветовую гамму (вместо BT.2020 можно использовать BT.2100)
	enum class BT_STANDARD : int { CUSTOM = -1, BT470 = 0, BT601, BT709, BT2100 };

namespace Colors {

	struct _REFERENCE_WHITE_COMPONENTS {
		// Параметры
		XYZ_OBSERVER	stdObserver = XYZ_OBSERVER::CIE1931_2;	// Опорный угол
		XYZ_ILLUMINANT	illuminant = XYZ_ILLUMINANT::D65;		// Освещение
	};

#define IMPLEMENT_COLOR(N, n1, n2, n3)	  struct N { double n1 = 0.0, n2 = 0.0, n3 = 0.0; }
#define IMPLEMENT_COLOR_RW(N, n1, n2, n3) struct N : _REFERENCE_WHITE_COMPONENTS { double n1 = 0.0, n2 = 0.0, n3 = 0.0; }
	// Значения HSL в диапазоне [0.0..1.0], что соответ. Hue [0.0..360.0)deg; Saturation и Lightness [0.0..100.0]%
	IMPLEMENT_COLOR(HSL, H, S, L);
	// Значения HSV в диапазоне [0.0..1.0], что соответ. Hue [0.0..360.0)deg; Saturation и Value/Brightness [0.0..100.0]%
	IMPLEMENT_COLOR(HSV, H, S, V);
	// Значения HSI в диапазоне [0.0..1.0], что соответ. Hue [0.0..360.0)deg; Saturation и Intensity [0.0..100.0]%
	IMPLEMENT_COLOR(HSI, H, S, I);
	// Значения HWB в диапазоне [0.0..1.0], что соответ. Hue [0.0..360.0)deg; Whiteness и Blackness [0.0..100.0]%
	IMPLEMENT_COLOR(HWB, H, W, B);

	// Y в диапазоне [0.0..1.0]. Значения UV зависят от используемого стандарта (см.пояснения к YUV_ConversionData)
	IMPLEMENT_COLOR(YUV, Y, U, V);
	// Не относится к YUV!
	// Значения YCoCg в диапазоне [0.0..1.0] [-max..max] [-max..max], где max = 0.5 (для R-варианта = 1.0)
	IMPLEMENT_COLOR(YCOCG, Y, Co, Cg);
	// Значения должны быть в диапазоне от [0.0..1.0]
	IMPLEMENT_COLOR(SRGB, R, G, B);
	// Значения CMY в диапазоне [0.0..1.0]
	IMPLEMENT_COLOR(CMY, C, M, Y);

	// Значения RGBA в диапазоне [0.0..1.0]
	struct RGBA : SRGB { double A = 0.0; };
	// Значения HSLA в диапазоне [0.0..1.0]
	struct HSLA : HSL { double A = 0.0; };
	// Значения CMYK в диапазоне [0.0..1.0]
	struct CMYK : CMY { double K = 0.0; };

	// X,Y,Z зависят от Reference White.
	// Для D65/2: X,Y,Z = [0.0..0.95047] [0.0..1.0] [0.0..1.08883] - соответ. R,G,B = [0.0..1.0]
	IMPLEMENT_COLOR_RW(XYZ, X, Y, Z);
	// Значения Hunter L,a,b в диапазоне = [0.0..1.0] ~[-0.7..1.1] ~[-2.0..0.56]
	IMPLEMENT_COLOR_RW(HLAB, L, a, b);
	// Значения CIE L*a*b в диапазоне = [0.0..100.0] ~[-90.0..100.0] ~[-115.0..100.0]
	IMPLEMENT_COLOR_RW(LAB, L, a, b);
	// Значения CIE L*CH в диапазоне [0.0..100.0] [0.0..~180.0] [0.0..360.0]
	IMPLEMENT_COLOR_RW(LCH, L, C, H);
	// Значения CIE L*uv в диапазоне [0.0..100.0] ~[-85.0..177.0] ~[-135.0..110.0]
	IMPLEMENT_COLOR_RW(LUV, L, u, v);
	// Значения xyY в диапазоне ~[ 0.17..1.0] [~0.06..1.0] [0.0..1.0]
	IMPLEMENT_COLOR_RW(XYY, x, y, Y);
	// Значения	LMS в диапазоне зависят от используемой LMS_CONVERSION...
	IMPLEMENT_COLOR_RW(LMS, L, M, S);

} // END namespace ByteInterpreter::Colors

// ========= ========= ========= ========= ========= ========= ========= =========

class CColorConverter {
protected:
	// Поля с текущии настройками для преобразований в/из цветового пространства XYZ
	double			m_TransformMatrixXYZtoRGB[3][3];
	double			m_TransformMatrixRGBtoXYZ[3][3];
	XYZ_METHOD		m_xyzChromaticAdaptationMethod;
	XYZ_ILLUMINANT	m_xyzIlluminant;
	XYZ_OBSERVER	m_xyzObserer;

	// YUV, YIQ, ...
	double			m_TransformMatrixYUVtoRGB[3][3];
	double			m_TransformMatrixRGBtoYUV[3][3];
	BT_STANDARD     m_yuvStandardBT = BT_STANDARD::BT470;

	// Для конвертации RGBA
	Colors::SRGB   m_clrBackground = { 1.0, 1.0, 1.0 };

public:
	CColorConverter();

	// ========= ========= ========= ========= ========= ========= ========= =========

	void XYZ_UseAdaptationMethod(XYZ_METHOD method);
	const double* XYZ_GetReferenceWhite(const XYZ_OBSERVER& stdObserver, const XYZ_ILLUMINANT& illuminant);

	// В out предварительно нужно настроить "опорный угол" и "освещение"
	void XYZ_Convert(Colors::XYZ& out, const Colors::XYZ& in);
	// Конвертирует XYZ D65/2 в RGB
	void XYZD65toRGB(Colors::SRGB& out, const Colors::XYZ& in);
	// Конвертирует RGB в XYZ D65/2
	void RGBtoXYZD65(Colors::XYZ& out, const Colors::SRGB& in);

	// ========= ========= ========= ========= ========= ========= ========= =========

	void xyY2XYZ(Colors::XYZ& out, const Colors::XYY& in);
	void XYZ2xyY(Colors::XYY& out, const Colors::XYZ& in);
	void XYZ2HunterLab(Colors::HLAB& out, const Colors::XYZ& in);
	void HunterLab2XYZ(Colors::XYZ& out, const Colors::HLAB& in);
	void XYZ2CIELAB(Colors::LAB& out, const Colors::XYZ& in);
	void CIELAB2XYZ(Colors::XYZ& out, const Colors::LAB& in);
	void XYZ2CIELUV(Colors::LUV& out, const Colors::XYZ& in);
	void CIELUV2XYZ(Colors::XYZ& out, const Colors::LUV& in);

	void CIELAB2CIELCHab(Colors::LCH& out, const Colors::LAB& in);
	void CIELCHab2CIELAB(Colors::LAB& out, const Colors::LCH& in);
	void CIELUV2CIELCHuv(Colors::LCH& out, const Colors::LUV& in);
	void CIELCHuv2CIELUV(Colors::LUV& out, const Colors::LCH& in);

	void XYZD65toLMS(Colors::LMS& out, const Colors::XYZ& in, const LMS_CONVERSION c);
	void LMStoXYZ(Colors::XYZ& out, const Colors::LMS& in, const LMS_CONVERSION c);

	// ========= ========= ========= ========= ========= ========= ========= =========

	// Алгоритм не оптиизирован
	// Альфа канал (A) - имеет диапазон [0.0..1.0]
	void HSLA2HSL(Colors::HSL& out, const Colors::HSLA& in);
	// Альфа канал (A) - имеет диапазон [0.0..1.0]
	void RGBA2RGB(Colors::SRGB& out, const Colors::RGBA& in);
	// Альфа канал (A) - имеет диапазон [0.0..1.0]
	COLORREF RGBA2RGB(const Colors::RGBA& in);
	// Диапазон lightness [0.0..1.0]
	static void GrayScale2RGB(Colors::SRGB& out, const double lightness);

	void RGBA_SetBackgroundColor(const Colors::SRGB& clr);

	// ========= ========= ========= ========= ========= ========= ========= =========

	static void HSV2RGB(Colors::SRGB& out, const Colors::HSV& in);
	static void HSL2RGB(Colors::SRGB& out, const Colors::HSL& in);
	static void HSI2RGB(Colors::SRGB& out, const Colors::HSI& in);

	static void RGB2HSV(Colors::HSV& out, const Colors::SRGB& in);
	static void RGB2HSL(Colors::HSL& out, const Colors::SRGB& in);
	static void RGB2HSI(Colors::HSI& out, const Colors::SRGB& in);

	static void HSV2HWB(Colors::HWB& out, const Colors::HSV& in);
	static void HWB2HSV(Colors::HSV& out, const Colors::HWB& in);

	// ========= ========= ========= ========= ========= ========= ========= =========

	static void RGB2CMY(Colors::CMY& out, const Colors::SRGB& in);
	static void CMY2RGB(Colors::SRGB& out, const Colors::CMY& in);
	static void CMYK2RGB(Colors::SRGB& out, const Colors::CMYK& in);
	static void CMYK2CMY(Colors::CMY& out, const Colors::CMYK& in);
	static void CMY2CMYK(Colors::CMYK& out, const Colors::CMY& in);
	
	// ========= ========= ========= ========= ========= ========= ========= =========

	static void YCoCg2RGB(Colors::SRGB& out, const Colors::YCOCG& in, BOOL bYCoCgR);
	static void RGB2YCoCg(Colors::YCOCG& out, const Colors::SRGB& in, BOOL bYCoCgR);

	// ========= ========= ========= ========= ========= ========= ========= =========

	void YUV_UseBTStandard(BT_STANDARD bt);
	void YUV_GetTransformMatrix(double* m3x3, BOOL b2RGB);
	static void YUV_GetConversionData(double* W5, BT_STANDARD standard);

	// W: Wr, Wb - константы (?эталонного цвета?); Umax и Vmax - макс. значение цветоразностных компонентов
	// YDbDr (SECAM): W { 0.299, 0.587, 0.114, 1.333, -1.333 }; -> Y = [0.0..1.0]; Dr, Db = [+/-1.333]
	void YUV_CreateTransformMatrix(const double W[5]);
	
	// Возможно Umax = |m_TransformMatrixRGBtoYUV[2][1]|; Vmax = |m_TransformMatrixRGBtoYUV[1][0]|
	// YIQ: 33 deg -> Y = [0.0..1.0]; I = [+/-0.5226]; Q = [+/-0.5957]
	void YUV_RotateTransformMatrix(const double deg);

	// Пример для YCbCr: YUV2RGB(Y - (bFullRange ? 0.0 : 16.0/255), Cb-0.5, Cr-0.5)
	void YUV2RGB(Colors::SRGB& out, const Colors::YUV& in);

	// Пример для YCbCr: RGB2YUV(R, G, B) и	out += { (bFullRange ? 0.0 : 16.0/255), 0.5, 0.5 }
	void RGB2YUV(Colors::YUV& out, const Colors::SRGB& in);

private:
	// XYZ D65/2 deg (RGB Working Space: sRGB)
	void XYZ_CreateTransformationMatrix();
};
} // END namespace ByteInterpreter

/*
* YUV = YPbPr = сигналы Y'CbCr до нормирования и смещения сигналов для перевода сигналов в цифровую форму
* YPbPr - для аналогового компонентного видео
* YCbCr - для аналогового/цифрового компонентного видео
* YDbDr - аналоговое (SECAM)
* YIQ - цветное
*/
