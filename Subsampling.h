#pragma once
#include "ConversionData.h"


namespace ByteInterpreter {
	enum class SubsamplingMode : int { NONE = -1, AYUV = 0, YUV, YUY2, UYVY, YUV422p, YU12, YV12, YUV411 };

	class ISubsampling {
	public:
		virtual void Extract(CONVERSION_DATA&) = 0;
	};
	
	namespace Subsampling {
#define IMPLEMENT_SUBSAMPLING_CLASS(N)				\
	class N : public ISubsampling {					\
	public:											\
		virtual void Extract(CONVERSION_DATA& c);	\
	}

		IMPLEMENT_SUBSAMPLING_CLASS(NONE);
		IMPLEMENT_SUBSAMPLING_CLASS(AYUV);			// 4:4:4 (32 bpp)
		IMPLEMENT_SUBSAMPLING_CLASS(YUV);			// 4:4:4 (24 bpp)
		IMPLEMENT_SUBSAMPLING_CLASS(YUY2);			// 4:2:2 (16 bpp)
		IMPLEMENT_SUBSAMPLING_CLASS(UYVY);			// 4:2:2 (16 bpp)
		IMPLEMENT_SUBSAMPLING_CLASS(YUV422p);		// 4:2:2 (?? bpp)

		// 4:2:0 (YU12/YV12, ? bpp)
		class YUV420p : public ISubsampling {
			UINT m_uIndexU = 24;
			UINT m_uIndexV = 30;
			ULONGLONG m_temp[36]{ 0 };
		public:
			// IMC3 = YU12, иначе IMC1 = YV12
			YUV420p(BOOL bIMC3);
			virtual void Extract(CONVERSION_DATA& c);
		};

		class YUV411 : public ISubsampling {
			ULONGLONG m_temp[4]{ 0 };
		public:
			virtual void Extract(CONVERSION_DATA& c);
		};

		// class C311
		// class C410
		// class C421
	} // END namespace ByteInterpreter::Subsampling
} // END namespace ByteInterpreter