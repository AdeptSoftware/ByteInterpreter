#pragma once
#include "ByteTokenizer.h"


namespace ByteInterpreter {
	class CAbstractTokenizer : public CBitTokenizer {
	protected:
		UINT m_uCount;
		ULONGLONG m_uMaxValue;
	public:
		virtual ULONGLONG Get() = 0;
		const ULONGLONG& GetMaxValue();
	};

	namespace Tokenizer {
		class CDefault : public CAbstractTokenizer {
		public:
			CDefault(UINT uCount);
			virtual ULONGLONG Get();
		};

		class CRGB565 : public CAbstractTokenizer {
			int m_nIndex = 0;
		public:
			CRGB565();
			virtual ULONGLONG Get();
		};
		
	} // END namespace ByteInterpreter::Tokenizer
} // END namespace ByteInterpreter