#pragma once // ITokenizer ����� ���� ��� ByteInterpreter, ��� ��� ��� ��������������� ������
#include "Interfaces.h"

namespace ByteInterpreter {
    // ========= ========= ========= ========= ========= ========= ========= =========
    // ������ CByteTokenizer � CBitTokenizer ������ ��������� ���������� ����/���
    // ������������ �������� count � ������ Next() ���������� ULONGLONG (8 ����)!

    // ������� Next, Seek ��������� ��������, ����������� � ������
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

    // ������� Next, Seek ��������� ��������, ����������� � �����
    class CBitTokenizer : public CByteTokenizer {
    public:
        virtual void        SetBytes(BYTE* bytes, ULONGLONG len) override;
        virtual ULONGLONG   Next(UINT count) override;
    };
} // END namespace ByteInterpreter