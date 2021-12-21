#include "pch.h"
#include "Subsampling.h"
using namespace ByteInterpreter::Subsampling;
using namespace ByteInterpreter;


void NONE::Call() {
	for (UINT i = 0; i < m_pData->uCount; i++)
		m_pData->out[i].u = m_pData->pTokenizer->Get();
	m_pData->pManager->OnExtract();
}

void COPY::Call() {
	m_pData->out[0].u = m_pData->pTokenizer->Get();
	m_pData->out[2] = m_pData->out[1] = m_pData->out[0];
	m_pData->pManager->OnExtract();
}

void AYUV::Call() {
	m_pData->out[2].u = m_pData->pTokenizer->Get();	// V
	m_pData->out[1].u = m_pData->pTokenizer->Get();	// U
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y
	m_pData->out[3].u = m_pData->pTokenizer->Get();	// A
	m_pData->pManager->OnExtract();
}

void YUV::Call() {
	m_pData->out[2].u = m_pData->pTokenizer->Get();	// V
	m_pData->out[1].u = m_pData->pTokenizer->Get();	// U
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y
	m_pData->pManager->OnExtract();
}

void YUY2::Call() {
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y0
	m_pData->out[1].u = m_pData->pTokenizer->Get();	// U0
	m_pData->out[3].u = m_pData->pTokenizer->Get();	// Y1
	m_pData->out[2].u = m_pData->pTokenizer->Get();	// V0
	m_pData->pManager->OnExtract();
	m_pData->out[0] = m_pData->out[3];
	m_pData->pManager->OnExtract();
}

void UYVY::Call() {
	m_pData->out[1].u = m_pData->pTokenizer->Get();	// U0
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y0
	m_pData->out[2].u = m_pData->pTokenizer->Get();	// V0
	m_pData->pManager->OnExtract();
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y1
	m_pData->pManager->OnExtract();
}

void YUV422p::Call() { // Не совсе так
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y0
	m_pData->out[3].u = m_pData->pTokenizer->Get();	// Y1
	m_pData->out[1].u = m_pData->pTokenizer->Get();	// U0
	m_pData->out[2].u = m_pData->pTokenizer->Get();	// V0
	m_pData->pManager->OnExtract();
	m_pData->out[0] = m_pData->out[3];
	m_pData->pManager->OnExtract();
}

void YUV411::Call() {
	m_pData->out[1].u = m_pData->pTokenizer->Get();	// U
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y1
	m_pData->out[3].u = m_pData->pTokenizer->Get();	// Y2
	m_pData->out[2].u = m_pData->pTokenizer->Get();	// V
	m_pData->pManager->OnExtract();
	m_pData->out[0] = m_pData->out[3];
	m_pData->pManager->OnExtract();
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y3
	m_pData->pManager->OnExtract();
	m_pData->out[0].u = m_pData->pTokenizer->Get();	// Y4
	m_pData->pManager->OnExtract();
}

// ========= ========= ========= ========= ========= ========= ========= =========

void YUV420p::Call() {}
void YUV410::Call()  {}
void YUV421::Call()  {}
void YUV311::Call()  {}

// ========= ========= ========= ========= ========= ========= ========= =========

UINT ByteInterpreter::GetColorComponentCount(const OPTIONS* pBI) {
	if (pBI->type == DataType::COLOR) {
		switch (pBI->clr.type) {
		case ColorList::CMYK:
		case ColorList::HSLA:
		case ColorList::RGBA:						return 4;
		default:									return 3;
		}
	}
	else											return 1;
}

CAbstractHandler* ByteInterpreter::CreateSubsamplingObject(const OPTIONS* pBI) {
	switch (pBI->clr.subsampling) {
	case SubsamplingMode::AYUV:						return new Subsampling::AYUV;
	case SubsamplingMode::YUV:						return new Subsampling::YUV;
	case SubsamplingMode::YUY2:						return new Subsampling::YUY2;
	case SubsamplingMode::UYVY:						return new Subsampling::UYVY;
	case SubsamplingMode::YUV422p:					return new Subsampling::YUV422p;
	//case SubsamplingMode::YU12:					return new Subsampling::YUV420p(TRUE);
	//case SubsamplingMode::YV12:					return new Subsampling::YUV420p(FALSE);
	case SubsamplingMode::YUV411:					return new Subsampling::YUV411;
	default:
		if (pBI->clr.type == ColorList::GrayScale)	return new Subsampling::COPY;
		else										return new Subsampling::NONE;
	}
}