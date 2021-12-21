#pragma once

namespace ByteInterpreter {
	class ITokenizer {
	public:
		virtual ULONGLONG   GetLength() = 0;
		virtual ULONGLONG   GetPos() = 0;

		virtual void        SetBytes(BYTE* bytes, ULONGLONG len) = 0;
		virtual BOOL        IsEmpty() = 0;

		virtual ULONGLONG	Get() = 0;
		virtual ULONGLONG   Next(UINT count) = 0;
		virtual void        Seek(LONGLONG offset) = 0;
		virtual void        SetPos(ULONGLONG pos) = 0;
	};

	// »нтерфейс класса, управл€ющего процессом преобразовани€ данных
	class IManager {
	public:
		virtual void OnExtract() = 0;
	};
} // END namespace ByteInterpreter
