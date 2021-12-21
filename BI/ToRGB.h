#pragma once
#include "ConversionData.h"
#include "ColorConverter.h"
#undef CMYK
#undef RGB

namespace ByteInterpreter {
	namespace ToRGB {
		class CAbstractColor : public CAbstractHandler {
		public:
			CColorConverter m_conv;
		};

#define IMPLEMENT_CLR_HANDLER(N, F) class N : public CAbstractColor { F; public: virtual void Call(); }
		IMPLEMENT_CLR_HANDLER(RGBA,			Colors::SRGB m_out);
		IMPLEMENT_CLR_HANDLER(CMY,			Colors::SRGB m_out);
		IMPLEMENT_CLR_HANDLER(CMYK,			Colors::SRGB m_out);
		IMPLEMENT_CLR_HANDLER(YUV,			Colors::SRGB m_out);
		IMPLEMENT_CLR_HANDLER(YCbCr,		Colors::SRGB m_out);
		IMPLEMENT_CLR_HANDLER(HSLA,			Colors::HSL  m_temp);
		IMPLEMENT_CLR_HANDLER(HSL,			Colors::SRGB m_out);
		IMPLEMENT_CLR_HANDLER(HSV,			Colors::SRGB m_out);
		IMPLEMENT_CLR_HANDLER(HSI,			Colors::SRGB m_out);
		IMPLEMENT_CLR_HANDLER(HWB,			Colors::HSV  m_temp);
		IMPLEMENT_CLR_HANDLER(XYZ,			Colors::XYZ  m_in);
		IMPLEMENT_CLR_HANDLER(xyY,			Colors::XYZ  m_temp;	Colors::XYY  m_in);
		IMPLEMENT_CLR_HANDLER(HunterLab,	Colors::XYZ  m_temp;	Colors::HLAB m_in);
		IMPLEMENT_CLR_HANDLER(Lab,			Colors::XYZ  m_temp;	Colors::LAB  m_in);
		IMPLEMENT_CLR_HANDLER(Luv,			Colors::XYZ  m_temp;	Colors::LUV  m_in);
		IMPLEMENT_CLR_HANDLER(LCHab,		Colors::XYZ  m_temp;	Colors::LAB  m_temp_lab;	Colors::LCH m_in);
		IMPLEMENT_CLR_HANDLER(LCHuv,		Colors::XYZ  m_temp;	Colors::LUV  m_temp_luv; 	Colors::LCH m_in);
		IMPLEMENT_CLR_HANDLER(LMS,			Colors::XYZ  m_temp;	Colors::LMS	 m_in);
		IMPLEMENT_CLR_HANDLER(YCoCg,		Colors::SRGB m_out);
	} // END namespace ByteInterpreter::ToRGB

	ToRGB::CAbstractColor* CreateToRGBConverter(const OPTIONS* pBI);
} // END namespace ByteInterpreter