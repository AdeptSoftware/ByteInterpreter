#include "pch.h"
#include "Converter.h"
using namespace ByteInterpreter;

LPCWSTR GetFormat(const OPTIONS& bi) {
	switch (bi.type) {
	case DataType::INT64:						return (bi.bExtended ? L"%I64u" : L"%I64d");
	case DataType::INT32:						return (bi.bExtended ? L"%I32u" : L"%I32d");
	case DataType::INT16:						return (bi.bExtended ? L"%d"    : L"%hd");
	case DataType::INT8:						return (bi.bExtended ? L"%hhu"  : L"%hhd");
	case DataType::HEX:							return L"%02X";
	case DataType::OCT:							return L"%o";
	case DataType::FLOAT:						return L"%.5f";
	case DataType::DOUBLE:						return (bi.bExtended ? L"%e" : L"%.5lf");
	case DataType::UNICODEBYTES:				return (bi.bExtended ? L"%04X" : L"%llc");
	case DataType::COLOR:
		if (bi.clr.bShowAsRGB ||
			bi.clr.type == ColorList::RGB || bi.clr.type == ColorList::GrayScale) {
			switch (bi.clr.view_mode) {
			case ColorModeView::HEX:			return L"#%06X";
			case ColorModeView::DEFAULT:
			case ColorModeView::INTEGER:		return L"%.0lf %.0lf %.0lf";
			default:							return L"%.3lf %.3lf %.3lf";
			}
		}

		switch (bi.clr.type) {
		case ColorList::CMYK:
			switch (bi.clr.view_mode) {
			case ColorModeView::DEFAULT:
			case ColorModeView::INTEGER:		return L"%.0lf %.0lf %.0lf %.0lf";
			default:							return L"%.3lf %.3lf %.3lf %.3lf";
			}
		case ColorList::CMY:
			switch (bi.clr.view_mode) {
			case ColorModeView::DEFAULT:
			case ColorModeView::INTEGER:		return L"%.0lf %.0lf %.0lf";
			default:							return L"%.3lf %.3lf %.3lf";
			}
		case ColorList::HSLA:
			switch (bi.clr.view_mode) {
			case ColorModeView::REAL:			return L"%.3lf %.3lf %.3lf %.3lf";
			default:							return L"%.1lf° %.1lf%% %.1lf%% %.3lf";
			}
		case ColorList::HSL:
		case ColorList::HSV:
		case ColorList::HSI:
		case ColorList::HWB:
			switch (bi.clr.view_mode) {
			case ColorModeView::REAL:			return L"%.3lf %.3lf %.3lf";
			default:							return L"%.1lf° %.1lf%% %.1lf%%";
			}
		case ColorList::RGBA:
			switch (bi.clr.view_mode) {
			case ColorModeView::HEX:			return L"#%08X";
			case ColorModeView::DEFAULT:
			case ColorModeView::INTEGER:		return L"%.0lf %.0lf %.0lf %.0lf";
			default:							return L"%.3lf %.3lf %.3lf %.3lf";
			}
		default:								return L"%.4lf %.4lf %.4lf";
		}
	default:									return L"";
	}
}

// ========= ========= ========= ========= ========= ========= ========= =========

void CConverter::PrintBytes() {
	int k[8] = { 32, 32, 48, 48, 56, 56, 0, 32 };		// сдвиг байт для !m_pBI->bInvBytesOrder;
	ULONGLONG uLen = (m_data.pTokenizer->GetLength() - m_bi.uOffset) / ByteInterpreter::SZBYTE;
	ULONGLONG uLastPos = m_data.pTokenizer->GetPos();	// Если не вернуть, то зациклися...
	// Получим первое значение
	m_data.pTokenizer->Seek(m_bi.uOffset, ByteInterpreter::CBitTokenizer::POSITION::begin);
	uint64_t out = m_data.pTokenizer->Next(sizeof(ULONGLONG) * ByteInterpreter::SZBYTE);
	m_data.pTokenizer->Seek(uLastPos, ByteInterpreter::CBitTokenizer::POSITION::begin);
	uint64_t inv = _byteswap_uint64(out);
	if (m_bi.bInvBytesOrder) {
		memset(k, 0, sizeof(int) * 8);					// Обнулим массив, так как сдвигать не нужно.
		std::swap(out, inv);
	}
	// Обрежем лишнее при длине массива меньше 8 байтов
	_snwprintf_s(m_buf.szBuffer, ByteInterpreter::BUFFER_SIZE, L"%016I64X...", inv);
	// *2 так как 1 байт - это 2 символа в текстовом эквиваленте
	if (uLen <= ByteInterpreter::SZBYTE) m_buf.szBuffer[uLen * 2] = L'\0';
	// Выведем результат
	LPCWSTR fmt = L"%s i32=%I32d\0%s (%I32u),\0%s i16=%hd\0%s (%hu),\0%s i8=%hhd\0%s (%hhu),\0%s d=%.2e,\0";
	_snwprintf_s(m_buf.szBuffer, ByteInterpreter::BUFFER_SIZE, L"%s (len=%I64u) →", m_buf.szBuffer, uLen);
	for (size_t len = wcslen(fmt), i = 0; len != 0; fmt += len + 1, len = wcslen(fmt), i++)
		_snwprintf_s(m_buf.szBuffer, ByteInterpreter::BUFFER_SIZE, fmt, m_buf.szBuffer, out >> k[i]);
	out >>= k[7];
	_snwprintf_s(m_buf.szBuffer, ByteInterpreter::BUFFER_SIZE, L"%s f=%.2f", m_buf.szBuffer, *reinterpret_cast<float*>(&out));
	SetWindowTextW(m_hWndOutputProgress, m_buf.szBuffer);
}

void CConverter::UpdateProgressText() {
	m_fProgress = (static_cast<float>(m_data.pTokenizer->GetPos()) / m_data.pTokenizer->GetLength()) * 100;
	_snwprintf_s(m_buf.szBuffer, ByteInterpreter::BUFFER_SIZE, L"%.1f%%: %u/%u", m_fProgress,
		static_cast<int>(m_data.pTokenizer->GetPos()), static_cast<int>(m_data.pTokenizer->GetLength()));
	SetWindowTextW(m_hWndOutputProgress, m_buf.szBuffer);
}

// ========= ========= ========= ========= ========= ========= ========= =========

CConverter::CConverter(const BI::OPTIONS& bi) 
	: CAbstractByteInterpreter(bi), m_buf(m_data.uDataSize) {
	m_szSeparator[0] = L'\0';
}

void CConverter::SetOutputText(HWND hWnd)		{ m_hWndOutputText	   = hWnd; }
void CConverter::SetOutputProgress(HWND hWnd)	{ m_hWndOutputProgress = hWnd; }

void CConverter::OnInitialize() {
	if (m_data.pTokenizer->IsEmpty()) {
		SetWindowTextW(m_hWndOutputText, L"");
		SetWindowTextW(m_hWndOutputProgress, L"");
	}
	m_szOutputText.clear();
	m_szOutputText.reserve(ByteInterpreter::BUFFER_SIZE);

	m_buf.viewmode   = m_bi.clr.view_mode;
	m_buf.bShowAsRGB = m_bi.clr.bShowAsRGB;
	wcscpy_s(m_buf.szFormat, m_bi.type == DataType::COLOR ? L"[" : m_bi.fmt.lpszPrefix);
	wcscat_s(m_buf.szFormat, GetFormat(m_bi));
	wcscat_s(m_buf.szFormat, m_bi.type == DataType::COLOR ? L"]" : m_bi.fmt.lpszSuffix);

	m_szSeparator[0] = L'\0';
	if (m_bi.type != DataType::UNICODEBYTES || m_bi.bExtended)
		wcscpy_s(m_szSeparator, m_bi.fmt.lpszSeparator);
}

void CConverter::OnIteration() {
	m_pMethod->ToString(m_buf);
	m_szOutputText += m_buf.szBuffer;

	if (!m_data.pTokenizer->IsEmpty()) {
		m_szOutputText += m_szSeparator;
		UpdateProgressText();
	}
	else {
		SetWindowTextW(m_hWndOutputText, m_szOutputText.c_str());
		m_szOutputText.clear();
		PrintBytes();
	}
}
