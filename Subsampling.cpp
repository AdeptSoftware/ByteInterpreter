// https://docs.microsoft.com/en-us/windows/win32/medfound/recommended-8-bit-yuv-formats-for-video-rendering
#include "pch.h"
#include <utility>
#include "Subsampling.h"
using namespace ByteInterpreter::Subsampling;

const UINT TEMP_COUNT			= 36;
const UINT T_COUNT				= 24;
const UINT TABLEYUV420[T_COUNT] = { 0, 1, 6, 7, 2, 3, 8, 9, 4, 5, 10, 11, 12, 13, 
									18, 19, 14, 15, 20, 21, 16, 17, 22, 23 };

void NONE::Extract(CONVERSION_DATA& c) {
	for (UINT i = 0; i < c.uCount; i++)
		c.raw[i] = c.pTokenizer->Get();
	c.OnExtract();
}

void AYUV::Extract(CONVERSION_DATA& c) {
	c.raw[2] = c.pTokenizer->Get();	// V
	c.raw[1] = c.pTokenizer->Get();	// U
	c.raw[0] = c.pTokenizer->Get();	// Y
	c.raw[3] = c.pTokenizer->Get();	// A
	c.OnExtract();
}

void YUV::Extract(CONVERSION_DATA& c) {
	c.raw[2] = c.pTokenizer->Get();	// V
	c.raw[1] = c.pTokenizer->Get();	// U
	c.raw[0] = c.pTokenizer->Get();	// Y
	c.OnExtract();
}

void YUY2::Extract(CONVERSION_DATA& c) {
	c.raw[0] = c.pTokenizer->Get();	// Y0
	c.raw[1] = c.pTokenizer->Get();	// U0
	c.raw[3] = c.pTokenizer->Get();	// Y1
	c.raw[2] = c.pTokenizer->Get();	// V0
	c.OnExtract();
	c.raw[0] = c.raw[3];
	c.OnExtract();
}

void UYVY::Extract(CONVERSION_DATA& c) {
	c.raw[1] = c.pTokenizer->Get();	// U0
	c.raw[0] = c.pTokenizer->Get();	// Y0
	c.raw[2] = c.pTokenizer->Get();	// V0
	c.OnExtract();
	c.raw[0] = c.pTokenizer->Get();	// Y1
	c.OnExtract();
}

void YUV422p::Extract(CONVERSION_DATA& c) { // Не совсе так
	c.raw[0] = c.pTokenizer->Get();	// Y0
	c.raw[3] = c.pTokenizer->Get();	// Y1
	c.raw[1] = c.pTokenizer->Get();	// U0
	c.raw[2] = c.pTokenizer->Get();	// V0
	c.OnExtract();
	c.raw[0] = c.raw[3];
	c.OnExtract();
}

YUV420p::YUV420p(BOOL bIMC3) {
	if (bIMC3)
		std::swap(m_uIndexU, m_uIndexV);
}

void YUV420p::Extract(CONVERSION_DATA& c) { // Не совсе так
	for (UINT i = 0; i < TEMP_COUNT; i++)
		m_temp[i] = c.pTokenizer->Get();
	// Разбивка по значениям
	for (UINT Y = 0, U = m_uIndexU, V = m_uIndexV; Y < T_COUNT; Y++) {
		c.raw[0] = TABLEYUV420[Y];
		if (Y % 4 == 0) {
			c.raw[1] = m_temp[U++];
			c.raw[2] = m_temp[V++];
		}
		c.OnExtract();
	}
}

void YUV411::Extract(CONVERSION_DATA& c) {
	c.raw[1] = c.pTokenizer->Get();	// U
	c.raw[0] = c.pTokenizer->Get();	// Y1
	c.raw[3] = c.pTokenizer->Get();	// Y2
	c.raw[2] = c.pTokenizer->Get();	// V
	c.OnExtract();
	c.raw[0] = c.raw[3];
	c.OnExtract();
	c.raw[0] = c.pTokenizer->Get();	// Y3
	c.OnExtract();
	c.raw[0] = c.pTokenizer->Get();	// Y4
	c.OnExtract();
}