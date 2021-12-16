#include "pch.h"
#include "Tokenizer.h"
using namespace ByteInterpreter::Tokenizer;

const UINT RGB_VCOUNT	= 3;
const UINT RGB565[RGB_VCOUNT] = { 5, 6, 5 };

const ULONGLONG& ByteInterpreter::CAbstractTokenizer::GetMaxValue() {
	return m_uMaxValue;
}

// ========== ========== ========== ========== ========== ========== ========== ==========

CDefault::CDefault(UINT uCount) {
	m_uCount = uCount;
	if (uCount < 64)	m_uMaxValue	= (2ull << (uCount-1u))-1ull;
	else				m_uMaxValue = 1ull;
}

ULONGLONG CDefault::Get() {
	return Next(m_uCount);
}

// ========== ========== ========== ========== ========== ========== ========== ==========

ULONGLONG CRGB565::Get() {
	if (m_nIndex == RGB_VCOUNT)
		m_nIndex = 0;
	return Next(RGB565[m_nIndex++]);
}

CRGB565::CRGB565() {
	m_uCount	= 5;
	m_uMaxValue = 31; // (2^5)-1;
}

// ========== ========== ========== ========== ========== ========== ========== ==========

