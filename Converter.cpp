#include "pch.h"
#include "Converter.h"
using namespace ByteInterpreter;

void CConverter::PrintBytes() {
	int k[8] = { 32, 32, 48, 48, 56, 56, 0, 32 };		// сдвиг байт для !m_pBI->bInvBytesOrder;
	ULONGLONG uLen = (m_data.pTokenizer->GetLength()-m_data.pBI->uOffset)/ByteInterpreter::SZBYTE;
	ULONGLONG uLastPos = m_data.pTokenizer->GetPos();	// Если не вернуть, то зациклися...
	// Получим первое значение
	m_data.pTokenizer->SetPos(m_data.pBI->uOffset);
	uint64_t out = m_data.pTokenizer->Next(sizeof(ULONGLONG) * ByteInterpreter::SZBYTE);
	m_data.pTokenizer->SetPos(uLastPos);
	uint64_t inv = _byteswap_uint64(out);
	if (m_data.pBI->flags.bInvert) {
		memset(k, 0, sizeof(int) * 8);					// Обнулим массив, так как сдвигать не нужно.
		std::swap(out, inv);
	}
	// Обрежем лишнее при длине массива меньше 8 байтов
	_snwprintf_s(m_data.szBuffer, ByteInterpreter::BUFFER_SIZE, L"%016I64X...", inv);
	// *2 так как 1 байт - это 2 символа в текстовом эквиваленте
	if (uLen <= ByteInterpreter::SZBYTE) m_data.szBuffer[uLen*2] = L'\0';
	// Выведем результат
	LPCWSTR fmt = L"%s i32=%I32d\0%s (%I32u),\0%s i16=%hd\0%s (%hu),\0%s i8=%hhd\0%s (%hhu),\0%s d=%.2e,\0";
	_snwprintf_s(m_data.szBuffer, ByteInterpreter::BUFFER_SIZE, L"%s (len=%I64u) →", m_data.szBuffer, uLen);
	for (size_t len = wcslen(fmt), i = 0; len != 0; fmt += len + 1, len = wcslen(fmt), i++)
		_snwprintf_s(m_data.szBuffer, ByteInterpreter::BUFFER_SIZE, fmt, m_data.szBuffer, out >> k[i]);
	out >>= k[7];
	_snwprintf_s(m_data.szBuffer, ByteInterpreter::BUFFER_SIZE, L"%s f=%.2f", m_data.szBuffer, *reinterpret_cast<float*>(&out));
	SetWindowTextW(m_hWndOutputProgress, m_data.szBuffer);
}

void CConverter::UpdateProgressText() {
	m_fProgress = (static_cast<float>(m_data.pTokenizer->GetPos())/m_data.pTokenizer->GetLength())*100;
	_snwprintf_s(m_data.szBuffer, ByteInterpreter::BUFFER_SIZE, L"%.1f%%: %I64u/%I64u", m_fProgress, 
		m_data.pTokenizer->GetPos(), m_data.pTokenizer->GetLength());
	SetWindowTextW(m_hWndOutputProgress, m_data.szBuffer);
}

// ========= ========= ========= ========= ========= ========= ========= =========

CConverter::CConverter(const ByteInterpreter::OPTIONS& bi) 
	: CAbstractByteInterpreter(bi) {
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
	m_szOutputText.reserve(BUFFER_SIZE);

	m_szSeparator[0] = L'\0';
#ifdef BYTEINTERPRETER_BYTE2STRING
	if (m_data.pBI->type != DataType::UNICODEBYTES || m_data.pBI->flags.bExtended)
		wcscpy_s(m_szSeparator, m_data.pBI->fmt.szSeparator);
#endif // END BYTEINTERPRETER_BYTE2STRING
}

void CConverter::OnIteration() {
	m_szOutputText += m_data.szBuffer;
	if (!m_data.pTokenizer->IsEmpty()) {
		m_szOutputText += m_szSeparator;
		UpdateProgressText();
	}
}

void CConverter::OnEndOfBytes() {
	SetWindowTextW(m_hWndOutputText, m_szOutputText.c_str());
	m_szOutputText.clear();
	PrintBytes();
}