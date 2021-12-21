#include "pch.h"
#include "ToRGB.h"
#define BI ByteInterpreter
using namespace BI::ToRGB;


const UINT		SZCLR3					= sizeof(double)*3;
const double	KOEF_YCBCR_NOFULLRANGE	= 16.0/255.0;

#define CAST(T)	 *reinterpret_cast<T*>(&m_pData->out[0].d)

// ========= ========= ========= ========= ========= ========= ========= =========

void RGBA::Call() {
	m_conv.RGBA2RGB(m_out, CAST(Colors::RGBA));
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

void CMY::Call() {
	m_conv.CMY2RGB(m_out, CAST(Colors::CMY));
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

void CMYK::Call() {
	m_conv.CMYK2RGB(m_out, CAST(Colors::CMYK));
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

void YUV::Call() {
	m_conv.YUV2RGB(m_out, CAST(Colors::YUV));
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

void YCbCr::Call() {
	m_pData->out[0].d -= (m_pData->pBI->flags.bFullRangeYCbCr ? 0.0 : KOEF_YCBCR_NOFULLRANGE);
	m_pData->out[1].d -= 0.5;	// Cb
	m_pData->out[2].d -= 0.5;	// Cr
	m_conv.YUV2RGB(m_out, CAST(Colors::YUV));
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

void HSLA::Call() {
	m_conv.HSLA2HSL(m_temp, CAST(Colors::HSLA));
	m_conv.HSL2RGB(CAST(Colors::SRGB), m_temp);
}

void HSL::Call() {
	m_conv.HSL2RGB(m_out, CAST(Colors::HSL));
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

void HSV::Call() {
	m_conv.HSV2RGB(m_out, CAST(Colors::HSV));
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

void HSI::Call() {
	m_conv.HSI2RGB(m_out, CAST(Colors::HSI));
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

void HWB::Call() {
	m_conv.HWB2HSV(m_temp, CAST(Colors::HWB));
	m_conv.HSV2RGB(CAST(Colors::SRGB), m_temp);
}

void XYZ::Call() {
	memcpy_s(&m_in.X, SZCLR3, m_pData->out, SZCLR3);
	m_conv.XYZD65toRGB(CAST(Colors::SRGB), m_in);
}

void xyY::Call() {
	memcpy_s(&m_in.x, SZCLR3, m_pData->out, SZCLR3);
	m_conv.xyY2XYZ(m_temp, m_in);
	m_conv.XYZD65toRGB(CAST(Colors::SRGB), m_temp);
}

void HunterLab::Call() {
	if (m_pData->out[0].d > 1.0f) {
		m_pData->out[0].d /= 100.0;
		m_pData->out[1].d /= 100.0;
		m_pData->out[2].d /= 100.0;
	}
	memcpy_s(&m_in.L, SZCLR3, m_pData->out, SZCLR3);
	m_conv.HunterLab2XYZ(m_temp, m_in);
	m_conv.XYZD65toRGB(CAST(Colors::SRGB), m_temp);
}

void Lab::Call() {
	memcpy_s(&m_in.L, SZCLR3, m_pData->out, SZCLR3);
	m_conv.CIELAB2XYZ(m_temp, m_in);
	m_conv.XYZD65toRGB(CAST(Colors::SRGB), m_temp);
}

void Luv::Call() {
	memcpy_s(&m_in.L, SZCLR3, m_pData->out, SZCLR3);
	m_conv.CIELUV2XYZ(m_temp, m_in);
	m_conv.XYZD65toRGB(CAST(Colors::SRGB), m_temp);
}

void LCHab::Call() {
	memcpy_s(&m_in.L, SZCLR3, m_pData->out, SZCLR3);
	m_conv.CIELCHab2CIELAB(m_temp_lab, m_in);
	m_conv.CIELAB2XYZ(m_temp, m_temp_lab);
	m_conv.XYZD65toRGB(CAST(Colors::SRGB), m_temp);
}

void LCHuv::Call() {
	memcpy_s(&m_in.L, SZCLR3, m_pData->out, SZCLR3);
	m_conv.CIELCHuv2CIELUV(m_temp_luv, m_in);
	m_conv.CIELUV2XYZ(m_temp, m_temp_luv);
	m_conv.XYZD65toRGB(CAST(Colors::SRGB), m_temp);
}

void LMS::Call() {
	memcpy_s(&m_in.L, SZCLR3, m_pData->out, SZCLR3);
	m_conv.LMStoXYZ(m_temp, m_in, LMS_CONVERSION::LMS2XYZ_D65_2);
	m_conv.XYZD65toRGB(CAST(Colors::SRGB), m_temp);
}

void YCoCg::Call() {
	m_conv.YCoCg2RGB(m_out, CAST(Colors::YCOCG), m_pData->pBI->flags.bYCoCgR);
	memcpy_s(m_pData->out, SZCLR3, &m_out, SZCLR3);
}

// ========= ========= ========= ========= ========= ========= ========= =========

CAbstractColor* CreateYUVMethod(const BI::OPTIONS* pBI) {
	auto pMethod = new YUV;
	if (pBI->clr.type == BI::ColorList::YIQ)
		pMethod->m_conv.YUV_RotateTransformMatrix(33.0);
	else if (pBI->clr.type == BI::ColorList::YDbDr) {
		const double W[] = { 0.299, 0.587, 0.114, 1.333, -1.333 };
		pMethod->m_conv.YUV_CreateTransformMatrix(W);
	}
	return pMethod;
}

CAbstractColor* ByteInterpreter::CreateToRGBConverter(const OPTIONS* pBI) {
	if (pBI->type != BI::DataType::COLOR)	return nullptr;
	if (!pBI->flags.bShowAsRGB)				return nullptr;
	switch (pBI->clr.type) {
		case BI::ColorList::RGBA:			return new RGBA;
		case BI::ColorList::CMYK:			return new CMYK;
		case BI::ColorList::CMY:			return new CMY;
		case BI::ColorList::HSV:			return new HSV;
		case BI::ColorList::HSL:			return new HSL;
		case BI::ColorList::HSLA:			return new HSLA;
		case BI::ColorList::HSI:			return new HSI;
		case BI::ColorList::XYZ:			return new XYZ;
		case BI::ColorList::xyY:			return new xyY;
		case BI::ColorList::LAB:			return new Lab;
		case BI::ColorList::LUV:			return new Luv;
		case BI::ColorList::LCH_AB:			return new LCHab;
		case BI::ColorList::LCH_UV:			return new LCHuv;
		case BI::ColorList::HLAB:			return new HunterLab;
		// Не проверялись на реальных данных (или через онлайн-конвертеры):
		case BI::ColorList::YUV:
		case BI::ColorList::YIQ:
		case BI::ColorList::YDbDr:			return CreateYUVMethod(pBI);
		case BI::ColorList::YCbCr:			return new YCbCr;
		case BI::ColorList::LMS:			return new LMS;
		case BI::ColorList::HWB:			return new HWB;
		case BI::ColorList::YCoCg:			return new YCoCg;
	}
	return nullptr;
}
