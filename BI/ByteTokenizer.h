#pragma once // ITokenizer нужен лишь для ByteInterpreter, так как это самостоятельные классы
#include "Interfaces.h"

namespace ByteInterpreter {
    // ========= ========= ========= ========= ========= ========= ========= =========
    // Классы CByteTokenizer и CBitTokenizer выдают указанное количество байт/бит
    // Максимальное значение count в методе Next() ограничено ULONGLONG (8 байт)!

    // Функции Next, Seek принимают значения, исчисляемые в БАЙТАХ
    class CByteTokenizer : public ITokenizer {
    public:
        virtual ULONGLONG   GetLength();
        virtual ULONGLONG   GetPos();

        virtual void        SetBytes(BYTE* bytes, ULONGLONG len = 1);
        virtual BOOL        IsEmpty();

        virtual ULONGLONG   Next(UINT count);
        virtual void        Seek(LONGLONG offset);
        virtual void        SetPos(ULONGLONG pos);

    protected:
        ULONGLONG   m_pos   = 0ull;
        ULONGLONG   m_len   = 0ull;
        BYTE*       m_bytes = nullptr;
    };

    // ========= ========= ========= ========= ========= ========= ========= =========

    // Функции Next, Seek принимают значения, исчисляемые в БИТАХ
    class CBitTokenizer : public CByteTokenizer {
    public:
        virtual void        SetBytes(BYTE* bytes, ULONGLONG len) override;
        virtual ULONGLONG   Next(UINT count) override;
    };
} // END namespace ByteInterpreter