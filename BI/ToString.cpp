#include "pch.h"
#include <math.h>
#include <wchar.h>
#include "ToString.h"
using namespace ByteInterpreter;

#ifdef BYTEINTERPRETER_BYTE2STRING

enum ExtendedViewMode : int {
	DEFAULT				= 0,
	BINARY				= 1,
	REAL				= 2,
	RGB_HEX				= 3,
	RGBA_HEX			= 4,
	CLR3_REAL			= 5,
	CLR4_REAL			= 6,
	CLR3_INTEGER		= 7,
	CLR4_INTEGER		= 8,
	HSLA				= 9,
	HXX					= 10,
};

int ByteInterpreter::GetViewModeIndex(const OPTIONS* pBI) {
	switch (pBI->type) {
	case DataType::BIN:									return ExtendedViewMode::BINARY;
	case DataType::FLOAT:								return ExtendedViewMode::REAL;
	case DataType::COLOR:
		if (pBI->flags.bShowAsRGB ||
			pBI->clr.type == ColorList::RGB ||
			pBI->clr.type == ColorList::GrayScale) {
			switch (pBI->clr.viewmode) {
			case ColorViewMode::HEX:					return ExtendedViewMode::RGB_HEX;
			case ColorViewMode::REAL:					return ExtendedViewMode::CLR3_REAL;
			default:									return ExtendedViewMode::CLR3_INTEGER;
			}
		}
		switch (pBI->clr.type) {
		case ColorList::RGBA:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::HEX:					return ExtendedViewMode::RGBA_HEX;
			case ColorViewMode::REAL:					return ExtendedViewMode::CLR4_REAL;
			default:									return ExtendedViewMode::CLR4_INTEGER;
			}
		case ColorList::CMY:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::REAL:					return ExtendedViewMode::CLR3_REAL;
			default:									return ExtendedViewMode::CLR3_INTEGER;
			}
		case ColorList::CMYK:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::REAL:					return ExtendedViewMode::CLR4_REAL;
			default:									return ExtendedViewMode::CLR4_INTEGER;
			}
		case ColorList::HSLA:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::REAL:					return ExtendedViewMode::CLR4_REAL;
			default:									return ExtendedViewMode::HSLA;
			}
		case ColorList::HSI:
		case ColorList::HSL:
		case ColorList::HSV:
		case ColorList::HWB:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::REAL:					return ExtendedViewMode::CLR3_REAL;
			default:									return ExtendedViewMode::HXX;
			}
		default:										return ExtendedViewMode::CLR3_REAL;
		}
	default:											return ExtendedViewMode::DEFAULT;
	}
}

// ========= ========= ========= ========= ========= ========= ========= =========

void ToString::ANY::Call() {
	switch (m_pData->nViewModeIndex) {
	case ExtendedViewMode::REAL:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat, m_pData->out[0].f);
		break;
	case ExtendedViewMode::CLR3_INTEGER:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat,
			round(BYTEMAX*m_pData->out[0].d), round(BYTEMAX*m_pData->out[1].d),
			round(BYTEMAX*m_pData->out[2].d));
		break;
	case ExtendedViewMode::CLR4_INTEGER:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat,
			round(BYTEMAX*m_pData->out[0].d), round(BYTEMAX*m_pData->out[1].d),
			round(BYTEMAX*m_pData->out[2].d), round(BYTEMAX*m_pData->out[3].d));
		break;
	case ExtendedViewMode::CLR3_REAL:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat,
					 m_pData->out[0].d, m_pData->out[1].d, m_pData->out[2].d);
		break;
	case ExtendedViewMode::CLR4_REAL:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat,
					 m_pData->out[0].d, m_pData->out[1].d,
					 m_pData->out[2].d, m_pData->out[3].d);
		break;
	case ExtendedViewMode::RGB_HEX:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat,
			static_cast<UINT>(round(BYTEMAX*m_pData->out[0].d)),
			static_cast<UINT>(round(BYTEMAX*m_pData->out[1].d)),
			static_cast<UINT>(round(BYTEMAX*m_pData->out[2].d)));
		break;
	case ExtendedViewMode::RGBA_HEX:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat,
			static_cast<UINT>(round(BYTEMAX*m_pData->out[0].d)),
			static_cast<UINT>(round(BYTEMAX*m_pData->out[1].d)),
			static_cast<UINT>(round(BYTEMAX*m_pData->out[2].d)),
			static_cast<UINT>(round(BYTEMAX*m_pData->out[3].d)));
		break;
	case ExtendedViewMode::HXX:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat,
					 m_pData->out[0].d*360, m_pData->out[1].d*100,
					 m_pData->out[2].d*100);
		break;
	case ExtendedViewMode::HSLA:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat,
					 m_pData->out[0].d*360, m_pData->out[1].d*100,
					 m_pData->out[2].d*100, m_pData->out[3].d);
		break;
	case ExtendedViewMode::BINARY:
		m_pData->szBuffer[m_pData->uValueSize] = L'\0';
		for (int i = m_pData->uValueSize-1; i >= 0; i--, m_pData->out[0].u >>= 1)
			m_pData->szBuffer[i] = L'0'+(m_pData->out[0].u & 1);
		break;
	default:
		_snwprintf_s(m_pData->szBuffer, BUFFER_SIZE, m_pData->szFormat, m_pData->out[0].u);
	}
}

// ========= ========= ========= ========= ========= ========= ========= =========

LPCWSTR ByteInterpreter::GetFormatString(const OPTIONS* pBI) {
	switch (pBI->type) {
	case DataType::INT64:						return (pBI->flags.bExtended ? L"%I64u" : L"%I64d");
	case DataType::INT32:						return (pBI->flags.bExtended ? L"%I32u" : L"%I32d");
	case DataType::INT16:						return (pBI->flags.bExtended ? L"%d" : L"%hd");
	case DataType::INT8:						return (pBI->flags.bExtended ? L"%hhu" : L"%hhd");
	case DataType::HEX:							return L"%02X";
	case DataType::OCT:							return L"%o";
	case DataType::FLOAT:						return L"%.5f";
	case DataType::DOUBLE:						return (pBI->flags.bExtended ? L"%e" : L"%.5lf");
	case DataType::UNICODEBYTES:				return (pBI->flags.bExtended ? L"%04X" : L"%llc");
	case DataType::COLOR:
		if (pBI->flags.bShowAsRGB ||
			pBI->clr.type == ColorList::RGB || pBI->clr.type == ColorList::GrayScale) {
			switch (pBI->clr.viewmode) {
			case ColorViewMode::HEX:			return L"#%02X%02X%02X";
			case ColorViewMode::DEFAULT:
			case ColorViewMode::INTEGER:		return L"%.0lf %.0lf %.0lf";
			default:							return L"%.3lf %.3lf %.3lf";
			}
		}

		switch (pBI->clr.type) {
		case ColorList::CMYK:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::DEFAULT:
			case ColorViewMode::INTEGER:		return L"%.0lf %.0lf %.0lf %.0lf";
			default:							return L"%.3lf %.3lf %.3lf %.3lf";
			}
		case ColorList::CMY:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::DEFAULT:
			case ColorViewMode::INTEGER:		return L"%.0lf %.0lf %.0lf";
			default:							return L"%.3lf %.3lf %.3lf";
			}
		case ColorList::HSLA:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::REAL:			return L"%.3lf %.3lf %.3lf %.3lf";
			default:							return L"%.1lf° %.1lf%% %.1lf%% %.3lf";
			}
		case ColorList::HSL:
		case ColorList::HSV:
		case ColorList::HSI:
		case ColorList::HWB:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::REAL:			return L"%.3lf %.3lf %.3lf";
			default:							return L"%.1lf° %.1lf%% %.1lf%%";
			}
		case ColorList::RGBA:
			switch (pBI->clr.viewmode) {
			case ColorViewMode::HEX:			return L"#%02X%02X%02X%02X";
			case ColorViewMode::DEFAULT:
			case ColorViewMode::INTEGER:		return L"%.0lf %.0lf %.0lf %.0lf";
			default:							return L"%.3lf %.3lf %.3lf %.3lf";
			}
		default:								return L"%.4lf %.4lf %.4lf";
		}
	default:									return L"";
	}
}

#endif // END BYTEINTERPRETER_BYTE2STRING