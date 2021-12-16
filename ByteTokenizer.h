#pragma once
#include "pch.h"

namespace ByteInterpreter {
const ULONGLONG SZBYTE  = 8;                    // Количество бит в байте
const ULONGLONG SZFLOAT = 8*sizeof(float);      // Количество бит в float

// ========= ========= ========= ========= ========= ========= ========= =========
// Классы CByteTokenizer и CBitTokenizer выдают указанное количество байт/бит
// Максимальное значение count в методе Next() ограничено ULONGLONG (8 байт)!

// Функции Next, Seek принимают значения, исчисляемые в БАЙТАХ
class CByteTokenizer {
public:
    enum class POSITION : int { begin = 0, cur, end };
    
    ULONGLONG           GetLength();
    ULONGLONG           GetPos();

    virtual void        SetBytes(BYTE* bytes, ULONGLONG len = 1);
    virtual BOOL        IsEmpty();

    virtual ULONGLONG   Next(UINT count);
    virtual void        Seek(LONGLONG offset, POSITION from = POSITION::cur);
    virtual void        Stride(ULONGLONG count);

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