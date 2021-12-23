#pragma once					// ��� �������������� � ����� � ����� "pch.h" ��������:
#include "ConversionData.h"		// #define BYTEINTERPRETER_BYTE2STRING


namespace ByteInterpreter {
	class CAbstractByteInterpreter : public IManager {
	public:
		~CAbstractByteInterpreter();
		CAbstractByteInterpreter(const OPTIONS& bi);

		void Read();
		void ChangeMethods();

		virtual void OnExtract();

	protected:
		void ReleaseMemory();

		virtual void OnInitialize()			= 0;
		virtual void OnFirstIteration()		= 0;
		virtual void OnIteration()			= 0;
		virtual void OnEndOfBytes()			= 0;

	protected:
		CONVERSION_DATA		m_data;
		CAbstractHandler*	m_pSubsampling	= nullptr;	// ���������� ����� � ��������� ���
		CAbstractHandler*	m_pAdapter		= nullptr;	// ���������� �������� (�����) ��� ������������ ������
		CAbstractHandler*	m_pSwapper		= nullptr;	// ����������� ������� ����
		CAbstractHandler*	m_p2RGB			= nullptr;	// ����������� ������������ ���� � RGB

#ifdef BYTEINTERPRETER_BYTE2STRING
		CAbstractHandler*	m_p2String		= nullptr;	// ����������� ����� � �����
#endif // END BYTEINTERPRETER_BYTE2STRING
	};
} // END namespace ByteInterpreter