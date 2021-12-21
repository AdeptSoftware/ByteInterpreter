#include "pch.h"
#include "ByteInterpreter.h"
#include "Subsampling.h"
#include "Tokenizers.h"
#include "Adapters.h"
#include "Swappers.h"
#include "ToRGB.h"
#ifdef BYTEINTERPRETER_BYTE2STRING
#include "ToString.h"
#endif // END BYTEINTERPRETER_BYTE2STRING

using namespace ByteInterpreter;


void CAbstractByteInterpreter::ReleaseMemory() {
	if (m_data.pTokenizer)	delete m_data.pTokenizer;
	if (m_pSubsampling)		delete m_pSubsampling;
	if (m_pAdapter)			delete m_pAdapter;
	if (m_pSwapper)			delete m_pSwapper;
	if (m_p2RGB)			delete m_p2RGB;
#ifdef BYTEINTERPRETER_BYTE2STRING
	if (m_p2String)			delete m_p2String;
	m_p2String				= nullptr;
#endif // END BYTEINTERPRETER_BYTE2STRING
	m_data.pTokenizer		= nullptr;
	m_pSubsampling			= nullptr;
	m_pAdapter				= nullptr;
	m_pSwapper				= nullptr;
	m_p2RGB					= nullptr;
}

CAbstractByteInterpreter::~CAbstractByteInterpreter() {
	ReleaseMemory();
}

CAbstractByteInterpreter::CAbstractByteInterpreter(const OPTIONS& bi) {
	m_data.pBI		= &bi;
	m_data.pManager = this;
}

// ========= ========= ========= ========= ========= ========= ========= =========
// ========= ========= ========= ========= ========= ========= ========= =========

void CAbstractByteInterpreter::OnExtract() {
	if (m_pAdapter)		m_pAdapter->Call();
	if (m_pSwapper)		m_pSwapper->Call();
	if (m_p2RGB)		m_p2RGB->Call();
#ifdef BYTEINTERPRETER_BYTE2STRING
	if (m_p2String)		m_p2String->Call();
#endif // END BYTEINTERPRETER_BYTE2STRING
	OnIteration();
}

void CAbstractByteInterpreter::Read() {
	if (m_data.pBI->bytes) {
		m_data.pTokenizer->SetBytes(m_data.pBI->bytes, m_data.pBI->uLength);
		m_data.pTokenizer->SetPos(m_data.pBI->uOffset);
		while (!m_data.pTokenizer->IsEmpty()) {
			m_pSubsampling->Call();
			if (m_data.pBI->uStride != 0)
				m_data.pTokenizer->Seek(m_data.pBI->uStride);
		}
		OnEndOfBytes();
	}
}

void CAbstractByteInterpreter::ChangeMethods() {
	ReleaseMemory();
	m_data.uValueSize		= GetValueSize(m_data.pBI);
	m_data.uCount			= GetColorComponentCount(m_data.pBI);

	m_data.pTokenizer		= CreateTokenizer(m_data);
	m_pSubsampling			= CreateSubsamplingObject(m_data.pBI);
	m_pSwapper				= CreateSwapper(m_data.pBI);
	m_pAdapter				= CreateAdapter(m_data.pBI);
	m_p2RGB					= CreateToRGBConverter(m_data.pBI);

	if (m_pSubsampling)		m_pSubsampling->SetConversionData(&m_data);
	if (m_pSwapper)			m_pSwapper->SetConversionData(&m_data);
	if (m_p2RGB)			m_p2RGB->SetConversionData(&m_data);
	if (m_pAdapter)			m_pAdapter->SetConversionData(&m_data);

#ifdef BYTEINTERPRETER_BYTE2STRING
	m_p2String				= new ToString::ANY;
	if (m_p2String)			m_p2String->SetConversionData(&m_data);

	m_data.nViewModeIndex = GetViewModeIndex(m_data.pBI);

	wcscpy_s(m_data.szFormat, m_data.pBI->type == DataType::COLOR ? L"[" : m_data.pBI->fmt.szPrefix);
	wcscat_s(m_data.szFormat, GetFormatString(m_data.pBI));
	wcscat_s(m_data.szFormat, m_data.pBI->type == DataType::COLOR ? L"]" : m_data.pBI->fmt.szSuffix);
#endif // END BYTEINTERPRETER_BYTE2STRING
	OnInitialize();
}