#include "pch.h"
#include <utility>
#include "Adapters.h"
#define BI ByteInterpreter
using namespace BI::Adapter;

const double U5_MAX = 31.0;	// (2^5)-1
const double U6_MAX = 63.0;	// (2^6)-1

void DEFAULT::SetConversionData(CONVERSION_DATA* pData) {
	m_pData = pData;
	if (pData->uValueSize < 64)	m_uMaxValue = (2ull << (pData->uValueSize-1u))-1ull;
	else						m_uMaxValue = 1ull;
}

void DEFAULT::Call() {
	for (UINT i = 0; i < m_pData->uCount; i++)
		m_pData->out[i].d = static_cast<double>(m_pData->out[i].u)/m_uMaxValue;
}

// ========== ========== ========== ========== ========== ========== ========== ==========

void RGB565::Call() {
	if (m_pData->uCount > 3)
		m_pData->out[3].d = static_cast<double>(m_pData->out[3].u)/U5_MAX;
	m_pData->out[0].d = static_cast<double>(m_pData->out[0].u)/U5_MAX;
	m_pData->out[2].d = static_cast<double>(m_pData->out[2].u)/U5_MAX;
	m_pData->out[1].d = static_cast<double>(m_pData->out[1].u)/U6_MAX;
}

// ========== ========== ========== ========== ========== ========== ========== ==========

void BI::Adapter::FLOAT::Call() {
	for (UINT i = 0; i < m_pData->uCount; i++) {
		m_pData->out[i].u = _byteswap_ulong(static_cast<unsigned long>(m_pData->out[i].u));
		m_pData->out[i].d = *reinterpret_cast<float*>(&m_pData->out[i].u);
	}
}

// ========== ========== ========== ========== ========== ========== ========== ==========

BI::CAbstractHandler* BI::CreateAdapter(const OPTIONS* pBI) {
	if (pBI->type != DataType::COLOR)		return nullptr;
	if (pBI->clr.bpp == BPPFormat::FLOATS)	return new Adapter::FLOAT;
	if (pBI->clr.bpp == BPPFormat::RGB565)	return new Adapter::RGB565;
	return new Adapter::DEFAULT;
}