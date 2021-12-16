#pragma once
#include <functional>
#include "Tokenizer.h"


namespace ByteInterpreter {
	const DWORD CD_FLAG_RGB565	= 0x00000001;
	const DWORD CD_FLAG_ARGB	= 0x00000002;
	CONST DWORD CD_FLAG_SHOWASRGB = 0x00000004;

	struct CONVERSION_DATA {
		CAbstractTokenizer*	pTokenizer;				// ��������, ����������� ������� �������� ��������
		UINT				uDataSize;				// ������ ������ � �����
		ULONGLONG			raw[4];					// ����� ������ �� tokenizer
		UINT				uCount;					// ���������� ��-��� � raw ��� ����������

		DWORD				dwFlags;

		std::function<void()>			OnExtract;
		std::function<void(ULONGLONG&)>	InvertBytesOrder = nullptr;

		void CastF(double* out) {
			for (UINT i = 0; i < uCount; i++) {
				raw[i] = _byteswap_ulong(static_cast<unsigned long>(raw[i]));
				out[i] = *reinterpret_cast<float*>(&raw[i]);
			}
		}

		void CastD(double* out) {
			for (UINT i = 0; i < uCount; i++)
				out[i] = static_cast<double>(raw[i])/pTokenizer->GetMaxValue();
		}
	};

} // END namespace ByteInterpreter