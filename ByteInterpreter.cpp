#include "pch.h"
#include "ByteInterpreter.h"
using namespace ByteInterpreter;

void CAbstractByteInterpreter::ReleaseMemory() {
	if (m_pMethod)			delete m_pMethod;
	if (m_pSubsampling)		delete m_pSubsampling;
	if (m_data.pTokenizer)	delete m_data.pTokenizer;
}

CAbstractByteInterpreter::~CAbstractByteInterpreter() {
	ReleaseMemory();
}

CAbstractByteInterpreter::CAbstractByteInterpreter(const OPTIONS& bi) 
	: m_bi(bi) {
	UseSubsampling(SubsamplingMode::NONE);

	m_data.OnExtract = [this]() {
		m_pMethod->Use(m_data);
		OnIteration();
	};
}

// ========= ========= ========= ========= ========= ========= ========= =========

CAbstractTokenizer* CreateTokenizerObject(const OPTIONS& bi, const UINT& uDataSize) {
	if (bi.type == DataType::COLOR) {
		if (bi.clr.bpp == BPPFormat::RGB565 && 
		    (bi.clr.type == ColorList::RGB || bi.clr.type == ColorList::RGBA))
			return new Tokenizer::CRGB565();
	}
	return new Tokenizer::CDefault(uDataSize);
}

DWORD GetFlags(const OPTIONS& bi) {
	DWORD dwFlags = 0;
	if (bi.clr.bpp == BPPFormat::RGB565)	dwFlags |= CD_FLAG_RGB565;
	if (bi.clr.bARGB)						dwFlags |= CD_FLAG_ARGB;
	if (bi.clr.bShowAsRGB)					dwFlags |= CD_FLAG_SHOWASRGB;
	return dwFlags;
}

std::function<void(ULONGLONG&)> GetIBO(const UINT& uDataSize) {
	if (uDataSize >= 8 * SZBYTE) {
		return [](ULONGLONG& v) {
			v = (((v << 56) | (v >> 8))  & 0xFF000000FF000000) | (((v << 40) | (v >> 24)) & 0x00FF000000FF0000) | 
				(((v << 24) | (v >> 40)) & 0x0000FF000000FF00) | (((v << 8)  | (v >> 56)) & 0x000000FF000000FF);
		};
	}
	else if (uDataSize >= 4*SZBYTE) {
		return [](ULONGLONG& v) {
			v = ((v << 56) | ((v << 40) & 0x00FF000000000000) | ((v << 24) & 0x0000FF0000000000) | ((v << 8) & 0x000000FF00000000)) >> 32;
		};
	}
	else if (uDataSize >= 2 * SZBYTE) {
		return [](ULONGLONG& v) {
			v = ((v << 56) | ((v << 40) & 0x00FF000000000000)) >> 48;
		};
	}
	return nullptr;
}

ISubsampling* CreateSubsamplingObject(SubsamplingMode mode) {
	switch (mode) {
	case SubsamplingMode::AYUV:					return new Subsampling::AYUV();
	case SubsamplingMode::YUV:					return new Subsampling::YUV();
	case SubsamplingMode::YUY2:					return new Subsampling::YUY2();
	case SubsamplingMode::UYVY:					return new Subsampling::UYVY();
	case SubsamplingMode::YUV422p:				return new Subsampling::YUV422p();
	case SubsamplingMode::YU12:					return new Subsampling::YUV420p(TRUE);
	case SubsamplingMode::YV12:					return new Subsampling::YUV420p(FALSE);
	case SubsamplingMode::YUV411:				return new Subsampling::YUV411();
	default:									return new Subsampling::NONE();
	}
}

IConversionMethod* CreateYUVMethod(const OPTIONS& bi) {
	if (bi.clr.type == ColorList::YCbCr) {
		auto pMethod = new ColorMethods::YCBCR();
		pMethod->m_bFullRange = bi.clr.bFullRangeYCbCr;
		return pMethod;
	}
	auto pMethod = new ColorMethods::YUV();
	if (bi.clr.type == ColorList::YIQ)
		pMethod->m_conv.YUV_RotateTransformMatrix(33.0);
	else if (bi.clr.type == ColorList::YDbDr) {
		const double W[] = { 0.299, 0.587, 0.114, 1.333, -1.333 };
		pMethod->m_conv.YUV_CreateTransformMatrix(W);
	}
	return pMethod;
}

IConversionMethod* CreateMethod(const OPTIONS& bi) {
	switch (bi.type) {
	case DataType::BIN:							return new DataTypeMethods::BINARY();
	case DataType::FLOAT:						return new DataTypeMethods::FLOAT();
	case DataType::COLOR:
		switch (bi.clr.type) {
		case ColorList::GrayScale:				return new ColorMethods::GrayScale();
		case ColorList::RGB:					return new ColorMethods::SRGB();
		case ColorList::RGBA:					return new ColorMethods::RGBA();
		case ColorList::CMYK:					return new ColorMethods::SCMYK();
		case ColorList::CMY:					return new ColorMethods::CMY();
		case ColorList::HSV:					return new ColorMethods::HSV();
		case ColorList::HSL:					return new ColorMethods::HSL();
		case ColorList::HSLA:					return new ColorMethods::HSLA();
		case ColorList::HSI:					return new ColorMethods::HSI();
		case ColorList::XYZ:					return new ColorMethods::XYZ();
		case ColorList::xyY:					return new ColorMethods::XYY();
		case ColorList::LAB:					return new ColorMethods::LAB();
		case ColorList::LUV:					return new ColorMethods::LUV();
		case ColorList::LCH_AB:					return new ColorMethods::LCHab();
		case ColorList::LCH_UV:					return new ColorMethods::LCHuv();
		case ColorList::HLAB:					return new ColorMethods::HLAB();
		case ColorList::YUV:
		// Не проверялись на реальных данных (или через онлайн-конвертеры):
		case ColorList::YIQ:
		case ColorList::YCbCr:
		case ColorList::YDbDr:					return CreateYUVMethod(bi);
		case ColorList::LMS:					return new ColorMethods::LMS();
		case ColorList::HWB:					return new ColorMethods::HWB();
		case ColorList::YCoCg:					return new ColorMethods::YCOCG();
		}
	default:									return new DataTypeMethods::DEFAULT();
	}
}

UINT GetSize(const OPTIONS& bi) {
	switch (bi.type) {
	case DataType::DOUBLE:						return SZBYTE*sizeof(double);
	case DataType::INT64:						return SZBYTE*sizeof(int64_t);
	case DataType::INT32:						return SZBYTE*sizeof(int32_t);
	case DataType::FLOAT:						return SZFLOAT;
	case DataType::INT16:						return SZBYTE*sizeof(int16_t);
	case DataType::UNICODEBYTES:				return SZBYTE*sizeof(wchar_t);
	case DataType::COLOR:
		if (bi.clr.bpp == BPPFormat::FLOATS)	return SZFLOAT;
		switch (bi.clr.type) {
		case ColorList::RGB:
		case ColorList::RGBA:
			switch (bi.clr.bpp) {		
			case BPPFormat::RGB565:				// В коде +1 будем добавлять для G
			case BPPFormat::RGB555:				return 5;
			case BPPFormat::RGB444:				return 4;
			}
		}
	default:									return SZBYTE*sizeof(int8_t);
	}
}

BOOL GetRepeatCount(const OPTIONS& bi) {
	if (bi.type == DataType::COLOR) {
		switch (bi.clr.type) {
		case ColorList::GrayScale:				return 1;
		case ColorList::CMYK:
		case ColorList::HSLA:
		case ColorList::RGBA:					return 4;
		default:								return 3;
		}
	}
	else										return 1;
}

// ========= ========= ========= ========= ========= ========= ========= =========

void CAbstractByteInterpreter::UseSubsampling(SubsamplingMode mode) {
	m_curSubsampling = mode;
}

void CAbstractByteInterpreter::Prepare() {
	ReleaseMemory();
	m_pMethod				= CreateMethod(m_bi);
	m_pSubsampling			= CreateSubsamplingObject(m_curSubsampling);

	m_data.uDataSize		= GetSize(m_bi);
	m_data.dwFlags			= GetFlags(m_bi);
	m_data.uCount			= GetRepeatCount(m_bi);
	m_data.pTokenizer		= CreateTokenizerObject(m_bi, m_data.uDataSize);

	m_data.pTokenizer->SetBytes(m_bi.bytes, m_bi.uLength);
	m_data.pTokenizer->Seek(m_bi.uOffset);

	if (m_bi.bInvBytesOrder) {
		if (m_bi.type == DataType::COLOR) m_data.InvertBytesOrder = GetIBO(SZFLOAT); // Любой подойдет для индикации
		else							  m_data.InvertBytesOrder = GetIBO(m_data.uDataSize);
	}
	else								  m_data.InvertBytesOrder = nullptr;

	OnInitialize();
}

void CAbstractByteInterpreter::Extract() {
	Prepare();
	while (!m_data.pTokenizer->IsEmpty()) {
		m_pSubsampling->Extract(m_data);
		if (m_bi.uStride != 0)
			m_data.pTokenizer->Stride(m_bi.uStride);
	}
}