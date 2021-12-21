#pragma once
#include "Interfaces.h"

// https://russianblogs.com/article/4816448935/
// https://docs.microsoft.com/en-us/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering
// https://en.wikipedia.org/wiki/YUV

namespace ByteInterpreter {
#ifdef BYTEINTERPRETER_BYTE2STRING
	const UINT FORMAT_SIZE	= 32;					// Размер буфера для строк-формата
#endif // END BYTEINTERPRETER_BYTE2STRING
	const UINT BUFFER_SIZE	= 512;					// Размер буфера для строк
	const UINT SZBYTE		= 8;                    // Количество бит в байте
	const UINT SZFLOAT		= 8*sizeof(float);      // Количество бит в float
	const UINT BYTEMAX		= 255;


	enum class DataType			: int { INT64 = 0, INT32, INT16, INT8, HEX, OCT, BIN, FLOAT, DOUBLE, COLOR, UNICODEBYTES };
	enum class ColorList		: int { RGB, RGBA, GrayScale, CMYK, CMY, HSV, HSL, HSLA, HSI, HWB, XYZ, LAB, xyY, LUV, LCH_AB, LCH_UV, LMS, HLAB, YUV, YCbCr, YIQ, YDbDr, YCoCg };
	enum class BPPFormat		: int { DEFAULT = 0, FLOATS, RGB888, RGB565, RGB555, RGB444 };
	enum class SubsamplingMode	: int { NONE = -1, AYUV = 0, YUV, YUY2, UYVY, YUV422p, YU12, YV12, YUV411 };

#ifdef BYTEINTERPRETER_BYTE2STRING
	enum class ColorViewMode	: int { DEFAULT = 0, HEX, INTEGER, REAL };
#endif // END BYTEINTERPRETER_BYTE2STRING


	struct OPTIONS {
		struct FLAGS {
			BOOL			bARGB			= FALSE;	// Использовать ARGB вместо RGBA
			BOOL			bExtended		= FALSE;	// Расширение типа (checkbox на окне)
			BOOL			bFullRangeYCbCr = TRUE;		// if full: [0..255] else: [16..240]
			BOOL			bInvert			= TRUE;		// Инвертировать порядок байт
			BOOL			bShowAsRGB		= FALSE;	// Отображать цвета как RGB
			BOOL			bYCoCgR			= FALSE;	// Использовать YCoCg-R вместо YCoCg
		} flags;

		DataType			type			= DataType::COLOR;
		
		struct COLOR {
			SubsamplingMode	subsampling		= SubsamplingMode::NONE;
			ColorList		type			= ColorList::RGB;
			BPPFormat		bpp				= BPPFormat::DEFAULT;

#ifdef BYTEINTERPRETER_BYTE2STRING
			ColorViewMode	viewmode		= ColorViewMode::DEFAULT;
#endif // END BYTEINTERPRETER_BYTE2STRING
		} clr;

#ifdef BYTEINTERPRETER_BYTE2STRING
		struct FORMAT {
			wchar_t			szPrefix[FORMAT_SIZE]{ 0 };
			wchar_t			szSuffix[FORMAT_SIZE]{ 0 };
			wchar_t			szSeparator[FORMAT_SIZE] = L" ";
		} fmt;
#endif // END BYTEINTERPRETER_BYTE2STRING

		UINT				uStride			= 0;		// После каждой итерации пропускать N байт
		UINT				uOffset			= 0;		// Начальное смещение от начала bytes
		
		// Tip1: Может удалить это от сюда...
		UINT				uSkip			= 0;		// После каждого uWidth байт (строки цвета) делать пропуск
		struct IMAGE {
			UINT			uWidth			= 0;
			UINT			uHeight			= 0;
			BOOL			bAutoSize		= TRUE;
		} img;
		// END Tip1

		ULONGLONG			uLength			= 0;		// Длина массива байт
		BYTE*				bytes			= nullptr;
	};


	struct CONVERSION_DATA {
		union VALUE {
			ULONGLONG		u;
			double			d;
			float			f;
		} out[4];

		UINT				uValueSize		= 0;
		UINT				uCount			= 0;
		const OPTIONS*		pBI				= nullptr;

		ITokenizer*			pTokenizer		= nullptr;
		IManager*			pManager		= nullptr;

#ifdef BYTEINTERPRETER_BYTE2STRING
		int					nViewModeIndex	= 0;
		wchar_t				szBuffer[BUFFER_SIZE]{ 0 };
		wchar_t				szFormat[BUFFER_SIZE]{ 0 };
#endif // END BYTEINTERPRETER_BYTE2STRING
	};


	// Базовый класс для всех обработчиков байтов
	class CAbstractHandler {
	protected:
		CONVERSION_DATA* m_pData;
	public:
		virtual void Call() = 0;
		virtual void SetConversionData(CONVERSION_DATA* pData) {
			m_pData = pData;
		}
	};

#define IMPLEMENT_HANDLER(N) class N : public CAbstractHandler { public: virtual void Call(); }
} // END namespace ByteInterpreter