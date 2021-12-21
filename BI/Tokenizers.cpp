#include "pch.h"
#include <stdint.h>
#include "Tokenizers.h"
using namespace ByteInterpreter::Tokenizer;
using namespace ByteInterpreter;

const UINT RGB565_TABLE[4] = { 5, 6, 5, 5 };

RGB565::RGB565(UINT uColorComponentCount) {
	m_uColorComponentCount = uColorComponentCount;
}

ULONGLONG RGB565::Get() {
	if (m_uIndex == m_uColorComponentCount)
		m_uIndex = 0;
	return CBitTokenizer::Next(RGB565_TABLE[m_uIndex++]);
}

// ========== ========== ========== ========== ========== ========== ========== ==========

DEFAULT::DEFAULT(UINT uReadBytes)	{
	m_uReadBytes = uReadBytes;
}

ULONGLONG DEFAULT::Get() {
	return CBitTokenizer::Next(m_uReadBytes);
}

// ========== ========== ========== ========== ========== ========== ========== ==========

UINT ByteInterpreter::GetValueSize(const OPTIONS* pBI) {
	switch (pBI->type) {
	case DataType::DOUBLE:						return SZBYTE*sizeof(double);
	case DataType::INT64:						return SZBYTE*sizeof(int64_t);
	case DataType::INT32:						return SZBYTE*sizeof(int32_t);
	case DataType::FLOAT:						return SZFLOAT;
	case DataType::INT16:						return SZBYTE*sizeof(int16_t);
	case DataType::UNICODEBYTES:				return SZBYTE*sizeof(wchar_t);
	case DataType::COLOR:
		switch (pBI->clr.bpp) {
		case BPPFormat::FLOATS:					return SZFLOAT;
		case BPPFormat::RGB565:					// В коде +1 будем добавлять для G
		case BPPFormat::RGB555:					return 5;
		case BPPFormat::RGB444:					return 4;
		}
	default:									return SZBYTE*sizeof(int8_t);
	}
}

ITokenizer* ByteInterpreter::CreateTokenizer(const CONVERSION_DATA& data) {
	if (data.pBI->type == DataType::COLOR && data.pBI->clr.bpp == BPPFormat::RGB565)
		return new RGB565(data.uCount);
	return new DEFAULT(data.uValueSize);
}
