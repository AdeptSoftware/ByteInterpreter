#pragma once
#define _CRTDBG_MAP_ALLOC
#include <windows.h>
#include <stdlib.h>
#include <crtdbg.h>
#include <stdio.h>


class CDebugLogger {
protected:
	static CDebugLogger* s_singleton;
	HANDLE m_hFile = INVALID_HANDLE_VALUE;

	CDebugLogger();
	BOOL Initialize(LPCWSTR lpszFileName);

	template <typename T>
	CDebugLogger& Write(T value, const char* fmt) {
		if (m_hFile != INVALID_HANDLE_VALUE) {
			char buf[32];
			int cnt = sprintf_s(buf, fmt, value);
			WriteFile(m_hFile, buf, cnt, nullptr, nullptr);
		}
		return *this;
	}

public:
	CDebugLogger(CDebugLogger& obj)			= delete;
	void operator=(const CDebugLogger& obj) = delete;

	static CDebugLogger* GetInstance();
	static void Attach(LPCWSTR lpszFileName);
	static void Detach();

	CDebugLogger& operator<< (int    v);
	CDebugLogger& operator<< (float  v);
	CDebugLogger& operator<< (double v);
	CDebugLogger& operator<< (const char* str);
	CDebugLogger& operator<< (const wchar_t* str);
	CDebugLogger& print(const char* format, ...);

	static void msgbox(LPCWSTR lpszText = L"", LPCWSTR lpszCaption = L"Debug Message!", UINT uType = MB_OK);
};


// Для инициализации требуется вызвать создать через CDebugLogger::Attach()
// После использования необходимо вызвать CDebugLogger::Detach();
// Не поддерживает одиночные символы: '0', L"1", ... Используйте: "2" или CDebugLogger::print
#define g_log	(*CDebugLogger::GetInstance())