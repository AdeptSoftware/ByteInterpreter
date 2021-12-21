#pragma once
#include "ConversionData.h"

namespace ByteInterpreter {
	UINT GetColorComponentCount(const OPTIONS* pBI);
	CAbstractHandler* CreateSubsamplingObject(const OPTIONS* pBI);

	namespace Subsampling {
		IMPLEMENT_HANDLER(NONE);
		IMPLEMENT_HANDLER(COPY);
		IMPLEMENT_HANDLER(AYUV);		// 4:4:4 (32 bpp)
		IMPLEMENT_HANDLER(YUV);			// 4:4:4 (24 bpp)
		IMPLEMENT_HANDLER(YUY2);		// 4:2:2 (16 bpp)
		IMPLEMENT_HANDLER(UYVY);		// 4:2:2 (16 bpp)
		IMPLEMENT_HANDLER(YUV422p);		// 4:2:2 (?? bpp)

		IMPLEMENT_HANDLER(YUV420p);		// 4:2:0 (?? bpp) [YU12/YV12]
		IMPLEMENT_HANDLER(YUV411);		// 4:1:1 (?? bpp)
		IMPLEMENT_HANDLER(YUV410);
		IMPLEMENT_HANDLER(YUV421);
		IMPLEMENT_HANDLER(YUV311);
	} // END namespace ByteInterpreter::Subsampling
} // END namespace ByteInterpreter