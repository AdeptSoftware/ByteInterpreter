#pragma once					// Для преобразования в текст в файле "pch.h" добавьте:
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
		CAbstractHandler*	m_pSubsampling	= nullptr;	// Декодирует байты в привычный вид
		CAbstractHandler*	m_pAdapter		= nullptr;	// Адаптирует значения (цвета) под определенный формат
		CAbstractHandler*	m_pSwapper		= nullptr;	// Инвертирует порядок байт
		CAbstractHandler*	m_p2RGB			= nullptr;	// Преобразует оригинальный цвет в RGB

#ifdef BYTEINTERPRETER_BYTE2STRING
		CAbstractHandler*	m_p2String		= nullptr;	// Преобразует байты в текст
#endif // END BYTEINTERPRETER_BYTE2STRING
	};
} // END namespace ByteInterpreter