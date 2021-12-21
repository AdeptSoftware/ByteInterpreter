#pragma once
#include "ConversionData.h"
#include "ColorConverter.h"


namespace ByteInterpreter {
	CAbstractHandler* CreateAdapter(const OPTIONS* pBI);

	namespace Adapter {
		class DEFAULT : public CAbstractHandler {
			UINT m_uMaxValue;
		public:
			virtual void Call();
			virtual void SetConversionData(CONVERSION_DATA* pData) override;
		};

		IMPLEMENT_HANDLER(RGB565);
		IMPLEMENT_HANDLER(FLOAT);
	} // END namespace ByteInterpreter::Adapter
} // END namespace ByteInterpreter