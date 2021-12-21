#include "pch.h"
#include "Swappers.h"
#include <utility>
using namespace ByteInterpreter::Swapper;
using namespace ByteInterpreter;

#define V		m_pData->out[0].u

void TYPE64::Call() {
	V = (((V << 56) | (V >> 8)) & 0xFF000000FF000000) | (((V << 40) | (V >> 24)) & 0x00FF000000FF0000) |
		(((V << 24) | (V >> 40)) & 0x0000FF000000FF00) | (((V << 8) | (V >> 56)) & 0x000000FF000000FF);
}

void TYPE32::Call() {
	V = ((V << 56) | ((V << 40) & 0x00FF000000000000) | ((V << 24) & 0x0000FF0000000000) | ((V << 8) & 0x000000FF00000000)) >> 32;
}

void TYPE16::Call() {
	V = ((V << 56) | ((V << 40) & 0x00FF000000000000)) >> 48;
}

// ========= ========= ========= ========= ========= ========= ========= =========

void CLR3::Call() {
	std::swap(m_pData->out[0], m_pData->out[2]);	// R <-> B
}

void CLR4::Call() {
	std::swap(m_pData->out[0], m_pData->out[3]);	// R <-> A
	std::swap(m_pData->out[1], m_pData->out[2]);	// G <-> B
}

void RGBA::Call() {
	if (m_pData->pBI->flags.bARGB)
		std::swap(m_pData->out[0], m_pData->out[2]);	// R <-> B
	if (m_pData->pBI->flags.bARGB == m_pData->pBI->flags.bInvert) {
		std::swap(m_pData->out[0], m_pData->out[3]);	// R <-> A
		std::swap(m_pData->out[1], m_pData->out[2]);	// G <-> B
	}
}

// ========= ========= ========= ========= ========= ========= ========= =========

CAbstractHandler* ByteInterpreter::CreateSwapper(const OPTIONS* pBI) {
	if (pBI->type == DataType::COLOR) {
		if (pBI->clr.type == ColorList::RGBA)			return new RGBA;
		if (pBI->flags.bInvert)							return nullptr;
		switch (pBI->clr.type) {
		case ColorList::GrayScale:						return nullptr;
		case ColorList::CMYK:
		case ColorList::HSLA:							return new CLR4;
		default:										return new CLR3;
		}
	}
	else {
		if (!pBI->flags.bInvert)						return nullptr;
		switch (pBI->type) {
		case DataType::INT64:
		case DataType::DOUBLE:							return new TYPE64;
		case DataType::INT32:
		case DataType::FLOAT:							return new TYPE32;
		case DataType::INT16:
		case DataType::UNICODEBYTES:					return new TYPE16;
		}
	}
	return nullptr;
}