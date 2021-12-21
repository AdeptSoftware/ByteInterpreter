#pragma once
#include "ByteTokenizer.h"
#include "ConversionData.h"

namespace ByteInterpreter {
	UINT		GetValueSize(const OPTIONS* pBI);
	ITokenizer* CreateTokenizer(const CONVERSION_DATA& data);

	namespace Tokenizer {
		class DEFAULT : public CBitTokenizer {
			UINT m_uReadBytes;
		public:
			DEFAULT(UINT uCount);
			virtual ULONGLONG Get();
		};

		class RGB565 : public CBitTokenizer {
			UINT m_uColorComponentCount = 0;
			UINT m_uIndex				= 0;
		public:
			RGB565(UINT nColorComponentCount);
			virtual ULONGLONG Get();
		};

	} // END namespace ByteInterpreter::Tokenizer
} // END namespace ByteInterpreter