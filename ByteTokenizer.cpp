#include "pch.h"
#include "ByteTokenizer.h"

using namespace ByteInterpreter;

ULONGLONG CByteTokenizer::GetLength()               { return m_len; }
ULONGLONG CByteTokenizer::GetPos()                  { return m_pos; }
BOOL      CByteTokenizer::IsEmpty()                 { return (m_pos >= m_len); }
void      CByteTokenizer::Stride(ULONGLONG count)   { m_pos += count; }

void CByteTokenizer::SetBytes(BYTE* bytes, ULONGLONG len) {
    m_bytes = bytes;
    m_len   = len;
    m_pos   = 0ull;
}

void CByteTokenizer::Seek(LONGLONG offset, CByteTokenizer::POSITION from) {
    if (offset < 0 && from != POSITION::cur)                    return;
    if (offset < 0 && m_pos <= static_cast<ULONGLONG>(-offset)) m_pos  = 0;
    else if (from == POSITION::begin)                           m_pos  = offset;
    else if (from == POSITION::cur)                             m_pos += offset;
    else                                                        m_pos  = m_len-offset;
}

ULONGLONG CByteTokenizer::Next(UINT count) {
    ULONGLONG value = 0ull;                             // Выходное значение
    ULONGLONG end   = m_pos+count;                      // Последний байт
    for (ULONGLONG i = m_pos; i < end && i < m_len; i++) {
        value = value << SZBYTE;
        value += m_bytes[i];
    }
    m_pos += count;
    return value;
}

// ========= ========= ========= ========= ========= ========= ========= =========

void CBitTokenizer::SetBytes(BYTE* bytes, ULONGLONG len) {
    m_bytes = bytes;
    m_len   = len*SZBYTE;
    m_pos   = 0ull;
}

ULONGLONG CBitTokenizer::Next(UINT count) {
    ULONGLONG i     = m_pos;
    ULONGLONG value = 0ull;                             // Выходное значение
    ULONGLONG end   = m_pos+count;                      // Последний бит
    ULONGLONG index = m_pos/SZBYTE;                     // Текущий байт
    int bit = 7-static_cast<int>(m_pos-(index*SZBYTE)); // Текущий бит
    while (i < end && i < m_len) {
        if (bit == 7 && end-i >= SZBYTE) {              // #1. Текущая позиция бита позволяет присоединять байты целиком
            value = value << SZBYTE;
            value += m_bytes[index];    
            index++;
            i += SZBYTE-1;
        }
        else {                                          // #2. Побитное присоединение
            value = value << 1;
            value += static_cast<bool>(m_bytes[index] & (1 << bit));
            if (bit == 0) {                             // Корректировка, чтобы возможно было воспользоваться пунктом #1
                bit = SZBYTE;
                index++;
            }
            bit--;
        }
        i++;
    }
    m_pos = end;
    return value << (end-i);
}