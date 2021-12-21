#pragma once
#include "ConversionData.h"


namespace ByteInterpreter {
	CAbstractHandler* CreateSwapper(const OPTIONS* pBI);

	namespace Swapper {
		IMPLEMENT_HANDLER(TYPE64);
		IMPLEMENT_HANDLER(TYPE32);
		IMPLEMENT_HANDLER(TYPE16);
		IMPLEMENT_HANDLER(CLR3);
		IMPLEMENT_HANDLER(CLR4);
		IMPLEMENT_HANDLER(RGBA);
	} // END namespace ByteInterpreter::Swapper
} // END namespace ByteInterpreter