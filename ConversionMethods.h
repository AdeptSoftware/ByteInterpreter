#pragma once // Не хорошо юзать define, но так хотя бы код предельно понятен...
#include "pch.h"
#include <wchar.h>
#include "ColorConverter.h"
#include "ConversionData.h"

namespace ByteInterpreter {
	const size_t BUFFER_SIZE = 512;
	enum class ColorModeView : int { DEFAULT = 0, HEX, INTEGER, REAL };

	struct TOSTRING_DATA {
		TOSTRING_DATA(const UINT& arg) : uDataSize(arg) {}

		wchar_t			szBuffer[BUFFER_SIZE]{ 0 };
		wchar_t			szFormat[BUFFER_SIZE]{ 0 };

		const UINT&		uDataSize;
		BOOL			bShowAsRGB = FALSE;
		ColorModeView	viewmode = ColorModeView::DEFAULT;
	};

	class IConversionMethod {
	public:
		virtual void Use(CONVERSION_DATA& data) = 0;
		virtual void ToString(TOSTRING_DATA& data) = 0;
	};

	class IColorMethod : public IConversionMethod {
	public:
		virtual COLORREF GetRGB() = 0;
	protected:
		virtual void ToRGB() = 0;
	};

	// ========= ========= ========= ========= ========= ========= ========= =========

namespace DataTypeMethods {
#define IMPLEMENT_DATATYPE_METHOD(N)																	\
	class N : public IConversionMethod {																\
		ULONGLONG m_uValue;																				\
	public:																								\
		virtual void Use(CONVERSION_DATA& data) {														\
			m_uValue = data.raw[0];																		\
			if (data.InvertBytesOrder)																	\
				data.InvertBytesOrder(m_uValue);														\
		}																								\
		virtual void ToString(TOSTRING_DATA& data);														\
	}

	IMPLEMENT_DATATYPE_METHOD(DEFAULT);
	IMPLEMENT_DATATYPE_METHOD(BINARY);
	IMPLEMENT_DATATYPE_METHOD(FLOAT);

} // END namespace ByteInterpreter::TypeMethods

	// ========= ========= ========= ========= ========= ========= ========= =========

// Какой же это колхоз... как б сделать красиво, но без defines...
namespace ColorMethods {
	struct FnLibrary {
		static void RGB2String(TOSTRING_DATA& data, const Colors::SRGB& clr);
	};

#define IMPLEMENT_COLOR_METHOD(N, M, USE, GETRGB, TOSTRING, FIELDS)											\
	class N : public IColorMethod, public M {																\
		FIELDS																								\
	public:																									\
		GETRGB																								\
		TOSTRING																							\
		USE																									\
	protected:																								\
		virtual void ToRGB();																				\
	}
#define IMPL_GETRGB_R		virtual COLORREF GetRGB();
#define IMPL_GETRGB_C		virtual COLORREF GetRGB() { return RGB(m_out.R, m_out.G, m_out.B); }
#define IMPL_TOSTRING_R		virtual void ToString(TOSTRING_DATA& data);
#define IMPL_TOSTRING_C(n1, n2, n3)																			\
	virtual void ToString(TOSTRING_DATA& data) {															\
		if (data.bShowAsRGB)																				\
			FnLibrary::RGB2String(data, m_out);																\
		else																								\
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, n1, n2, n3);					\
	}
#define IMPL_TOSTRING_H(n1, n2, n3)																			\
	virtual void ToString(TOSTRING_DATA& data) {															\
		if (data.bShowAsRGB)																				\
			FnLibrary::RGB2String(data, m_out);																\
		else {																								\
			if (data.viewmode == ColorModeView::REAL)														\
				_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, n1, n2, n3);				\
			else																							\
				_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, n1*360, n2*100, n3*100);	\
		}																									\
	}
#define IMPL_USE_DEF	virtual void Use(CONVERSION_DATA& data);
#define IMPL_USE(a1, a3)																					\
	virtual void Use(CONVERSION_DATA& data) {																\
		if (data.uDataSize == SZFLOAT)			data.CastF(&a1);											\
		else									data.CastD(&a1);											\
		if (!data.InvertBytesOrder)				std::swap(a1, a3);											\
		if (data.dwFlags & CD_FLAG_SHOWASRGB)	ToRGB();													\
	}
#define IMPL_USE4(a1, a2, a3, a4)																		\
	virtual void Use(CONVERSION_DATA& data) {																\
		if (data.uDataSize == SZFLOAT)				data.CastF(&a1);										\
		else										data.CastD(&a1);										\
		if (!data.InvertBytesOrder) {				std::swap(a1, a4); std::swap(a2, a3); }					\
		if (data.dwFlags & CD_FLAG_SHOWASRGB)		ToRGB();												\
	}
#define IMPL_FIELD_NONE
#define IMPL_FIELD_RGB		protected: Colors::SRGB m_out;
#define IMPL_FIELD_CONV		public: Colors::CColorConverter m_conv; IMPL_FIELD_RGB
#define IMPL_FIELD_YCBCR	public: BOOL m_bFullRange; IMPL_FIELD_CONV
#define IMPL_FIELD_HSLA		IMPL_FIELD_CONV	Colors::HSL m_temp_hsl;
#define IMPL_FIELD_HWB		IMPL_FIELD_RGB  Colors::HSV m_temp;
#define IMPL_FIELD_XYZ		IMPL_FIELD_CONV Colors::XYZ m_temp;
#define IMPL_FIELD_LCHAB	IMPL_FIELD_XYZ  Colors::LAB m_temp_lab;
#define IMPL_FIELD_LCHUV	IMPL_FIELD_XYZ  Colors::LUV m_temp_luv;
	
	IMPLEMENT_COLOR_METHOD(SRGB,  Colors::SRGB,	 IMPL_USE_DEF,		 IMPL_GETRGB_R, IMPL_TOSTRING_R,			IMPL_FIELD_NONE);
	IMPLEMENT_COLOR_METHOD(RGBA,  Colors::RGBA,	 IMPL_USE_DEF,		 IMPL_GETRGB_C, IMPL_TOSTRING_R,			IMPL_FIELD_CONV);

	IMPLEMENT_COLOR_METHOD(SCMYK, Colors::CMYK,  IMPL_USE4(C,M,Y,K), IMPL_GETRGB_C, IMPL_TOSTRING_R,			IMPL_FIELD_RGB);
	IMPLEMENT_COLOR_METHOD(CMY,	  Colors::CMY,	 IMPL_USE(C, Y),	 IMPL_GETRGB_C, IMPL_TOSTRING_R,			IMPL_FIELD_RGB);

	IMPLEMENT_COLOR_METHOD(YUV,	  Colors::YUV,	 IMPL_USE(Y, V),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(Y, U, V),   IMPL_FIELD_CONV);
	IMPLEMENT_COLOR_METHOD(YCBCR, Colors::YUV,	 IMPL_USE(Y, V),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(Y, U, V),   IMPL_FIELD_YCBCR);

	IMPLEMENT_COLOR_METHOD(HSLA,  Colors::HSLA,	 IMPL_USE4(H,S,L,A), IMPL_GETRGB_C, IMPL_TOSTRING_R,			IMPL_FIELD_HSLA);
	IMPLEMENT_COLOR_METHOD(HSL,	  Colors::HSL,	 IMPL_USE(H, L),	 IMPL_GETRGB_C, IMPL_TOSTRING_H(H, S, L),   IMPL_FIELD_RGB);
	IMPLEMENT_COLOR_METHOD(HSV,   Colors::HSV,	 IMPL_USE(H, V),	 IMPL_GETRGB_C, IMPL_TOSTRING_H(H, S, V),   IMPL_FIELD_RGB);
	IMPLEMENT_COLOR_METHOD(HSI,   Colors::HSI,	 IMPL_USE(H, I),	 IMPL_GETRGB_C, IMPL_TOSTRING_H(H, S, I),   IMPL_FIELD_RGB);
	IMPLEMENT_COLOR_METHOD(HWB,	  Colors::HWB,	 IMPL_USE(H, B),	 IMPL_GETRGB_C, IMPL_TOSTRING_H(H, W, B),   IMPL_FIELD_HWB);

	IMPLEMENT_COLOR_METHOD(XYZ,	  Colors::XYZ,	 IMPL_USE(X, Z),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(X, Y, Z),   IMPL_FIELD_XYZ);
	IMPLEMENT_COLOR_METHOD(XYY,	  Colors::XYY,	 IMPL_USE(x, Y),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(x, y, Y),   IMPL_FIELD_XYZ);
	IMPLEMENT_COLOR_METHOD(HLAB,  Colors::HLAB,	 IMPL_USE(L, b),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(L, a, b),   IMPL_FIELD_XYZ);
	IMPLEMENT_COLOR_METHOD(LAB,	  Colors::LAB,	 IMPL_USE(L, b),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(L, a, b),   IMPL_FIELD_XYZ);
	IMPLEMENT_COLOR_METHOD(LUV,	  Colors::LUV,	 IMPL_USE(L, v),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(L, u, v),   IMPL_FIELD_XYZ);
	IMPLEMENT_COLOR_METHOD(LCHab, Colors::LCH,	 IMPL_USE(L, H),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(L, C, H),   IMPL_FIELD_LCHAB);
	IMPLEMENT_COLOR_METHOD(LCHuv, Colors::LCH,	 IMPL_USE(L, H),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(L, C, H),   IMPL_FIELD_LCHUV);
	IMPLEMENT_COLOR_METHOD(LMS,	  Colors::LMS,	 IMPL_USE(L, S),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(L, M, S),   IMPL_FIELD_XYZ);

	IMPLEMENT_COLOR_METHOD(YCOCG, Colors::YCOCG, IMPL_USE(Y, Cg),	 IMPL_GETRGB_C, IMPL_TOSTRING_C(Y, Co, Cg), IMPL_FIELD_RGB);

#define IMPLEMENT_COLOR_METHOD_FROM_RGB(N) class N : public SRGB { protected: virtual void Use(CONVERSION_DATA& data) override; };
	IMPLEMENT_COLOR_METHOD_FROM_RGB(GrayScale);

} // END namespace ByteInterpreter::ColorMethods
} // END namespace ByteInterpreter
