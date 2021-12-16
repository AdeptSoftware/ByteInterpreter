#include "pch.h"
#include "Debug.h"

CDebugLogger* CDebugLogger::s_singleton = nullptr;
CDebugLogger::CDebugLogger() {}

BOOL CDebugLogger::Initialize(LPCWSTR lpszFileName) {
	m_hFile = CreateFileW(lpszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (m_hFile == INVALID_HANDLE_VALUE) {
		MessageBoxW(nullptr, L"Output file wasn't created!", L"Warning!", MB_OK); 
		return FALSE;
	}
	else {
		BYTE BOM[3] = { 0xEF, 0xBB, 0xBF };	// UTF-8
		WriteFile(m_hFile, BOM, 3, nullptr, nullptr);
		// Настройка вывода для утечек памяти
		_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
		_CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
		_CrtSetReportFile(_CRT_WARN, m_hFile);
		return TRUE;
	}
}

void CDebugLogger::Attach(LPCWSTR lpszFileName) {
	if (!s_singleton) {
		s_singleton = new CDebugLogger;
		if (!s_singleton->Initialize(lpszFileName))
			Detach();
	}
}

void CDebugLogger::Detach() {
	if (s_singleton) {
		HANDLE hFile = s_singleton->m_hFile;
		// Если не очистить перед вызовом CDML, то в выводе будет показывать утечку памяти...
		delete s_singleton;
		if (hFile != INVALID_HANDLE_VALUE) {
			_CrtDumpMemoryLeaks();	// CDML
			CloseHandle(hFile);
		}
		s_singleton = nullptr;
	}
}

CDebugLogger* CDebugLogger::GetInstance() {
	return s_singleton;
}

CDebugLogger& CDebugLogger::operator<< (int    v) { return Write(v, "%i"); }
CDebugLogger& CDebugLogger::operator<< (float  v) { return Write(v, "%f"); }
CDebugLogger& CDebugLogger::operator<< (double v) { return Write(v, "%lf"); }

CDebugLogger& CDebugLogger::operator<< (const char* str) {
	if (m_hFile != INVALID_HANDLE_VALUE)
		WriteFile(m_hFile, str, strlen(str), nullptr, nullptr);
	return *this;
}

CDebugLogger& CDebugLogger::operator<< (const wchar_t* str) {
	if (m_hFile != INVALID_HANDLE_VALUE) {
		size_t len = wcslen(str) + 1;
		char* buf = new char[len];
		sprintf_s(buf, len, "%ws", str);
		WriteFile(m_hFile, buf, len-1, nullptr, nullptr); 
		delete[] buf;
	}
	return *this;
}

CDebugLogger& CDebugLogger::print(const char* format, ...) {
	if (m_hFile != INVALID_HANDLE_VALUE) {
		va_list args;
		va_start(args, format);
		int len = vprintf_s(format, args)+1;
		char* buf = new char[len] { 0 };
		vsnprintf_s(buf, len, _TRUNCATE, format, args);
		va_end(args);
		WriteFile(m_hFile, buf, len-1, nullptr, nullptr);
		delete[] buf;
	}
	return *this;
}

void CDebugLogger::msgbox(LPCWSTR lpszText, LPCWSTR lpszCaption, UINT uType) {
	MessageBoxW(nullptr, lpszText, lpszCaption, uType);
}