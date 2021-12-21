#pragma once
#include "ConversionData.h"
#ifdef BYTEINTERPRETER_BYTE2STRING

#undef CMYK
#undef RGB

namespace ByteInterpreter {
	LPCWSTR GetFormatString(const OPTIONS* pBI);
	int GetViewModeIndex(const OPTIONS* pBI);

	namespace ToString {
		IMPLEMENT_HANDLER(ANY);
	} // END namespace ByteInterpreter::ToString
} // END namespace ByteInterpreter

#endif // END BYTEINTERPRETER_BYTE2STRING