#include "pch.h"
#include "ConversionMethods.h"
using namespace ByteInterpreter;
using namespace ByteInterpreter::DataTypeMethods;
using namespace ByteInterpreter::ColorMethods;

const UINT		G6_MAX_VALUE			= 63;	// (2^6)-1;
const double	KOEF_YCBCR_NOFULLRANGE	= 16.0/255.0;

#define ROUND(V)		round(V*BYTEMAX)
#define PACK3(R, G, B)	static_cast<BYTE>(ROUND(R)) | (static_cast<WORD>(ROUND(G)) << 8) | (static_cast<DWORD>(ROUND(B)) << 16)

// ========= ========= ========= ========= ========= ========= ========= =========

void DEFAULT::ToString(TOSTRING_DATA& data) {
	_snwprintf_s(data.szBuffer, BUFFER_SIZE, data.szFormat, m_uValue);
}

void DataTypeMethods::FLOAT::ToString(TOSTRING_DATA& data) {
	_snwprintf_s(data.szBuffer, BUFFER_SIZE, data.szFormat, *reinterpret_cast<float*>(&m_uValue));
}

void BINARY::ToString(TOSTRING_DATA& data) {
	data.szBuffer[data.uDataSize] = L'\0';
	for (int i = data.uDataSize-1; i >= 0; i--, m_uValue >>= 1)
		data.szBuffer[i] = L'0' + (m_uValue & 1);
}

// ========= ========= ========= ========= ========= ========= ========= =========

void GrayScale::Use(CONVERSION_DATA& data) {
	if (data.uDataSize == SZFLOAT)
		data.CastF(&R);
	else
		R = static_cast<double>(data.raw[0])/data.pTokenizer->GetMaxValue();
	G = B = R;
}

void SRGB::Use(CONVERSION_DATA& data) {
	if (data.uDataSize == SZFLOAT)
		data.CastF(&R);
	else{
		R = static_cast<double>(data.raw[0])/data.pTokenizer->GetMaxValue();
		B = static_cast<double>(data.raw[2])/data.pTokenizer->GetMaxValue();
		if (data.dwFlags & CD_FLAG_RGB565)
			G = static_cast<double>(data.raw[1])/G6_MAX_VALUE;
		else
			G = static_cast<double>(data.raw[1])/data.pTokenizer->GetMaxValue();
	}
	if (!data.InvertBytesOrder)
		std::swap(R, B);
}

void RGBA::Use(CONVERSION_DATA& data) {
	if (data.uDataSize == SZFLOAT)		data.CastF(&R);
	else {
		A = static_cast<double>(data.raw[3])/data.pTokenizer->GetMaxValue();
		R = static_cast<double>(data.raw[0])/data.pTokenizer->GetMaxValue();
		B = static_cast<double>(data.raw[2])/data.pTokenizer->GetMaxValue();
		if (data.dwFlags & CD_FLAG_RGB565)
			G = static_cast<double>(data.raw[1])/G6_MAX_VALUE;
		else
			G = static_cast<double>(data.raw[1])/data.pTokenizer->GetMaxValue();
	}
	if (data.dwFlags & CD_FLAG_ARGB)
		std::swap(R, B);		// bIsARGB: 0123 -> 3012;
	if ((data.InvertBytesOrder && (data.dwFlags & CD_FLAG_ARGB)) || (!data.InvertBytesOrder && !(data.dwFlags & CD_FLAG_ARGB))) {
		std::swap(R, A);
		std::swap(G, B);		// Этот код смещать вверх нельзя, так как важно для корректного отображения, чтобы G конвертировалось раньше
	}							// bIBO: 0123 -> 3210 || 3012 -> 2103
	if (data.dwFlags & CD_FLAG_SHOWASRGB)
		ToRGB();
}

// ========= ========= ========= ========= ========= ========= ========= =========

COLORREF SRGB::GetRGB() { return PACK3(R, G, B); }

// ========= ========= ========= ========= ========= ========= ========= =========

void FnLibrary::RGB2String(TOSTRING_DATA& data, const Colors::SRGB& clr) {
	if (data.viewmode == ColorModeView::HEX)
		_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, PACK3(clr.R, clr.G, clr.B));
	else if (data.viewmode == ColorModeView::INTEGER || data.viewmode == ColorModeView::DEFAULT)
		_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, ROUND(clr.R), ROUND(clr.G), ROUND(clr.B));
	else
		_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, clr.R, clr.G, clr.B);
}

void CMY::ToString(TOSTRING_DATA& data) {
	if (data.bShowAsRGB)
		FnLibrary::RGB2String(data, m_out);
	else {
		if (data.viewmode == ColorModeView::REAL)
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, C, M, Y);
		else
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, ROUND(C), ROUND(M), ROUND(Y));
	}
}

void SCMYK::ToString(TOSTRING_DATA& data) {
	if (data.bShowAsRGB)
		FnLibrary::RGB2String(data, m_out);
	else {
		if (data.viewmode == ColorModeView::REAL)
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, C, M, Y, K);
		else
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, ROUND(C), ROUND(M), ROUND(Y), ROUND(K));
	}
}

void RGBA::ToString(TOSTRING_DATA& data) {
	if (data.bShowAsRGB)
		FnLibrary::RGB2String(data, m_out);
	else {
		if (data.viewmode == ColorModeView::HEX)
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, PACK3(R, G, B) | (static_cast<DWORD>(ROUND(A)) << 24));
		else if (data.viewmode == ColorModeView::INTEGER || data.viewmode == ColorModeView::DEFAULT)
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, ROUND(R), ROUND(G), ROUND(B), ROUND(A));
		else
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, R, G, B, A);
	}
}

void SRGB::ToString(TOSTRING_DATA& data) {
	FnLibrary::RGB2String(data, *this);
}

void HSLA::ToString(TOSTRING_DATA& data) {
	if (data.bShowAsRGB)
		FnLibrary::RGB2String(data, m_out);
	else {
		if (data.viewmode == ColorModeView::REAL)
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, H, S, L, A);
		else
			_snwprintf_s(data.szBuffer, BUFFER_SIZE, _TRUNCATE, data.szFormat, H*360, S*100, L*100, A);
	}
}


// ========= ========= ========= ========= ========= ========= ========= =========

void HWB::ToRGB() {
	Colors::CColorConverter::HWB2HSV(m_temp, *this);
	Colors::CColorConverter::HSV2RGB(m_out, m_temp);
}

// ========= ========= ========= ========= ========= ========= ========= =========

void SRGB::ToRGB()	{ }
void HSL::ToRGB()	{ Colors::CColorConverter::HSL2RGB(m_out, *this); }
void HSV::ToRGB()	{ Colors::CColorConverter::HSV2RGB(m_out, *this); }
void HSI::ToRGB()	{ Colors::CColorConverter::HSI2RGB(m_out, *this); }
void CMY::ToRGB()	{ Colors::CColorConverter::CMY2RGB(m_out, *this); }
void SCMYK::ToRGB() { Colors::CColorConverter::CMYK2RGB(m_out, *this); }
void YCOCG::ToRGB() { Colors::CColorConverter::YCoCg2RGB(m_out, *this, FALSE); }

void YUV::ToRGB()	{ m_conv.YUV2RGB(m_out, *this); }
void RGBA::ToRGB()	{ m_conv.RGBA2RGB(m_out, *this); }


void YCBCR::ToRGB() {
	Y -= (m_bFullRange ? 0.0 : KOEF_YCBCR_NOFULLRANGE);
	/*Cb*/ U -= 0.5;
	/*Cr*/ V -= 0.5;
	m_conv.YUV2RGB(m_out, *this);
}

// ========= ========= ========= ========= ========= ========= ========= =========

void HSLA::ToRGB() {
	m_conv.HSLA2HSL(m_temp_hsl, *this);
	Colors::CColorConverter::HSL2RGB(m_out, m_temp_hsl);
}

// ========= ========= ========= ========= ========= ========= ========= =========

void HLAB::ToRGB() {
	if (L > 1.0f) {
		L /= 100.0;
		a /= 100.0;
		b /= 100.0;
	}
	m_conv.HunterLab2XYZ(m_temp, *this);
	m_conv.XYZD65toRGB(m_out, m_temp);
}


void XYZ::ToRGB() {
	m_conv.XYZD65toRGB(m_out, *this);
}

void LAB::ToRGB() {
	m_conv.CIELAB2XYZ(m_temp, *this);
	m_conv.XYZD65toRGB(m_out, m_temp);
}

void LCHab::ToRGB() {
	m_conv.CIELCHab2CIELAB(m_temp_lab, *this);
	m_conv.CIELAB2XYZ(m_temp, m_temp_lab);
	m_conv.XYZD65toRGB(m_out, m_temp);
}

void LCHuv::ToRGB() {
	m_conv.CIELCHuv2CIELUV(m_temp_luv, *this);
	m_conv.CIELUV2XYZ(m_temp, m_temp_luv);
	m_conv.XYZD65toRGB(m_out, m_temp);
}

void LUV::ToRGB() {
	m_conv.CIELUV2XYZ(m_temp, *this);
	m_conv.XYZD65toRGB(m_out, m_temp);
}

void XYY::ToRGB() {
	m_conv.xyY2XYZ(m_temp, *this);
	m_conv.XYZD65toRGB(m_out, m_temp);
}

void LMS::ToRGB() {
	m_conv.LMStoXYZ(m_temp, *this, Colors::LMS_CONVERSION::LMS2XYZ_D65_2);
	m_conv.XYZD65toRGB(m_out, m_temp);
}