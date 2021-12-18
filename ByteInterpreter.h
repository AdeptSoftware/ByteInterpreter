#pragma once
#include "pch.h"
#include "Subsampling.h"
#include "ConversionMethods.h"

namespace ByteInterpreter {
	// https://russianblogs.com/article/4816448935/
	// https://docs.microsoft.com/en-us/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering
	// https://en.wikipedia.org/wiki/YUV

	enum class DataType  : int { INT64 = 0, INT32, INT16, INT8, HEX, OCT, BIN, FLOAT, DOUBLE, COLOR, UNICODEBYTES };
	enum class ColorList : int { RGB, RGBA, GrayScale, CMYK, CMY, HSV, HSL, HSLA, HSI, HWB, XYZ, LAB, xyY, LUV, LCH_AB, LCH_UV, LMS, HLAB, YUV, YCbCr, YIQ, YDbDr, YCoCg };
	enum class BPPFormat : int { DEFAULT = 0, FLOATS, RGB888, RGB565, RGB555, RGB444 };

	struct OPTIONS {
		struct COLOR {
			ColorList		type			= ColorList::RGB;
			XYZ_OBSERVER	XYZ_observer	= XYZ_OBSERVER::CIE1931_2;
			XYZ_ILLUMINANT	XYZ_illuminant	= XYZ_ILLUMINANT::D65;

			ColorModeView	view_mode		= ColorModeView::DEFAULT;

			BOOL			bARGB			= FALSE;
			BOOL			bShowAsRGB		= FALSE;
			BOOL			bFullRangeYCbCr = FALSE;

			BPPFormat		bpp				= BPPFormat::DEFAULT;

			SubsamplingMode	subsampling		= SubsamplingMode::NONE;	// В основном для YUV
		} clr;

		struct IMAGE {
			UINT			uWidth			= 0;
			UINT			uHeight			= 0;
			BOOL			bAutoSize		= TRUE;
		} img;

		struct FORMAT {
			LPCWSTR			lpszPrefix		= L"";
			LPCWSTR			lpszSuffix		= L"";
			LPCWSTR			lpszSeparator	= L" ";						// После каждой итерации добавляет разделитель
		} fmt;

		DataType			type			= DataType::COLOR;
		BOOL				bExtended		= FALSE;					// Расширение типа (checkbox на окне)
		BOOL				bInvBytesOrder	= TRUE;

		UINT				uStride			= 0;						// После каждой итерации пропускать N байт
		UINT				uOffset			= 0;						// Начальное смещение от начала bytes
		UINT				uSkip			= 0;						// После каждого uWidth байт (строки цвета) делать пропуск

		ULONGLONG			uLength			= 0;						// Длина массива байт
		BYTE*				bytes			= nullptr;
	};

	// ========= ========= ========= ========= ========= ========= ========= =========

	class CAbstractByteInterpreter {
	protected:
		CONVERSION_DATA		m_data;
		const OPTIONS&		m_bi;

		SubsamplingMode		m_curSubsampling;
		ISubsampling*		m_pSubsampling		= nullptr;
		IConversionMethod*	m_pMethod			= nullptr;

		void ReleaseMemory();
	public:
		~CAbstractByteInterpreter();
		CAbstractByteInterpreter(const OPTIONS& bi);

		void UseSubsampling(SubsamplingMode mode);
		void Extract();

	protected:
		void Prepare();

		virtual void OnInitialize() = 0;
		virtual void OnIteration() = 0;
	};

} // END ByteInterpreter

#define BI ByteInterpreter