#include <iostream>		// Еще то извращение конечно....
#include <windows.h>
#include <limits>
using namespace std;

#undef min
#undef max

// Вне зависимости от конфигурации Debug/Release линкуем Debug-версию
// Так как только в ней есть доступ к классу-экспортеру CPluginDialog...
#pragma comment(lib, "Debug\\ByteInterpreter.lib")
#include "exports.h"

const UINT BUFFER_SIZE	= 4096;
const UINT ID_EDIT		= 125;

// Можно было конечно создать нормальное окно и не колхозить
// Однако в консоль дублируется с CDebug::print(), что может быть крайне полезно...
/*LPCWSTR CLASS_NAME = L"MainWnd";

LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_CLOSE)
		PostQuitMessage(0);
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

HWND CreateSimpleWindow(HINSTANCE hInst) {
	WNDCLASS wc;
	wc.style		 = 0;
	wc.cbClsExtra	 = 0;
	wc.hIcon		 = nullptr;
	wc.lpszMenuName  = nullptr; 
	wc.lpfnWndProc	 = WndProc;
	wc.cbWndExtra	 = 0;
	wc.hInstance	 = hInst;
	wc.hCursor		 = LoadCursor(nullptr, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(GetStockObject(WHITE_BRUSH));
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);

	return ::CreateWindow(CLASS_NAME, L"", WS_VISIBLE | WS_SYSMENU | WS_BORDER, 0, 0, 100, 100, GetDesktopWindow(), nullptr, hInst, nullptr);
}*/

HWND GetConsoleHandle(LPCWSTR lpszTitle = L"BYTEINTERPRETER CONSOLE") {
	SetConsoleTitleW(lpszTitle);
	Sleep(40);
	return FindWindow(nullptr, lpszTitle);
}

// ========= ========= ========= ========= ========= ========= ========= =========
CPluginDialogExporter dlg;

// Данное приложение для откладки DLL. Поэтому и так сойдет...
class CDataManager {
public:
	BYTE*					m_bytes		= nullptr;
	HWND					m_hDialog	= nullptr;
	HWND					m_hWnd		= nullptr;
	HWND					m_hEdit		= nullptr;
	HFONT					m_hFont		= nullptr;
	HMODULE					m_hInstDLL	= nullptr;
	
	CDataManager() {
	//	hWnd	 = CreateSimpleWindow(hInst);
		m_hWnd	 = GetConsoleHandle();
		m_hInstDLL = LoadLibraryW(L"ByteInterpreter.dll");
	}

	~CDataManager() {
		//	UnregisterClassW(CLASS_NAME, hInst);
		if (m_hFont)		DeleteObject(m_hFont);
		if (m_bytes)		delete[] m_bytes;
		if (m_hInstDLL)	FreeLibrary(m_hInstDLL);
	}
} g;

template<typename T>
void PrintBytes(T z) {
	BYTE* bytes = reinterpret_cast<BYTE*>(&z);
	for (int i = 0; i < sizeof(T); i++)
		printf("%02X ", int(bytes[i]));
	cout << endl;
}

void PrintColor(COLORREF clr) {
	printf("%03i %03i %03i\n", (int)GetRValue(clr), (int)GetGValue(clr), (int)GetBValue(clr));
}

void CreateEdit(HINSTANCE hInst, HWND hWndParent, UINT uID) {
	RECT rc;
	GetClientRect(hWndParent, &rc);
	HMENU hMenu = reinterpret_cast<HMENU>(uID);
	g.m_hEdit  = CreateWindow(L"EDIT", L"", WS_BORDER | WS_CHILD | WS_VISIBLE | ES_MULTILINE | WS_VSCROLL,
					      rc.right-425, 0, 425, rc.bottom, hWndParent, hMenu, hInst, nullptr);
	if (g.m_hEdit) { // Важно: шрифт должен быть постоянной ширины!
		g.m_hFont = CreateFont(-12, 0, 0, 0, 400, 0, 0, 0, 1, 0, 0, 2, FF_DONTCARE, L"Consolas");
		SendMessageW(g.m_hEdit, WM_SETFONT, reinterpret_cast<WPARAM>(g.m_hFont), TRUE);
	}
}

void Other();
void LoadFile(LPCWSTR lpszFileName);
void CreateValuesFile(LPCWSTR lpszFileName);

void Experiments() {
	CreateValuesFile(L"C:\\Users\\Adept\\Desktop\\values.txt");
	LoadFile(L"C:\\Users\\Adept\\Desktop\\values.txt");
	Other();
}

// ========= ========= ========= ========= ========= ========= ========= =========
// ========= ========= ========= ========= ========= ========= ========= =========

void OnSelChange() {
	DWORD dwStart = 0;
	DWORD dwEnd = 0;
	SendMessage(g.m_hEdit, EM_GETSEL, reinterpret_cast<WPARAM>(&dwStart), reinterpret_cast<WPARAM>(&dwEnd));
	// Посчитаем количество переносов (ранее мы добавляли символ '\r')
	DWORD i = 0;
	while (i < dwEnd) {
		if (g.m_bytes[i] == '\n') {
			if (i <= dwStart)
				dwStart--;
			dwEnd--;
		}
		i++;
	}
	if (dwStart >= dwEnd)
		return;
	// CPluginDialog требует, чтобы в функцию SetBytes поступал динамически выделенный массив байт!
	// Кроме того, нам нужен лишь небольшой участок...
	DWORD dwSize = dwEnd-dwStart;
	BYTE* bytes = new BYTE[dwSize];
	memcpy_s(bytes, dwSize, &g.m_bytes[dwStart], dwSize);
	dlg.SetBytes(bytes, dwSize, FALSE);
	if (!GetAsyncKeyState(VK_SHIFT))
		SetActiveWindow(g.m_hDialog);
}

int main(_In_ HINSTANCE hInst, _In_opt_ HINSTANCE hPrevInst, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow) {
	CreateEdit(hInst, g.m_hWnd, ID_EDIT);
	if (!g.m_hInstDLL || !g.m_hWnd || !g.m_hEdit) {
		MessageBox(nullptr, L"FAILED!", L"ERROR!", MB_OK);
		return 0;
	}
	SetWindowPos(g.m_hWnd, nullptr, 20, 140, 0, 0, SWP_NOZORDER | SWP_NOSIZE);

	g.m_hDialog = dlg.Create(g.m_hInstDLL, g.m_hWnd, nullptr);
	if (!g.m_hDialog) {
		MessageBox(nullptr, L"DON'T CREATE PLUGIN DIALOG!", L"ERROR!", MB_OK); 
		return 0;
	}
	ShowWindow(g.m_hDialog, SW_SHOW);

	Experiments();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0)) {
		// Отлавливаем сообщение закрытия диалога
		if (msg.message == WM_TIMER && !IsWindowVisible(g.m_hDialog))
			break; 
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		// Произошло выделение текста или просто клик...
		if (msg.hwnd == g.m_hEdit) {
			if (msg.message == WM_LBUTTONUP ||
				msg.message == WM_KEYDOWN && (msg.wParam >= VK_END && msg.wParam <= VK_DOWN) && GetAsyncKeyState(VK_SHIFT))
				OnSelChange();
			if (msg.message == WM_KEYUP && msg.wParam == VK_SHIFT)
				SetActiveWindow(g.m_hDialog);
		}
	}
	return (int)msg.wParam;
}

// ========= ========= ========= ========= ========= ========= ========= =========
// ========= ========= ========= ========= ========= ========= ========= =========

template <typename T> void WriteBytes(HANDLE hFile, UINT cnt, LPCSTR buf, ...) {
	WriteFile(hFile, buf, strlen(buf), nullptr, nullptr);
	
	va_list args;
	va_start(args, cnt);
	args += 4;	// По какой-то причине в начале записано 4 байт не пойми чего
	for (UINT i = 0; i < cnt; i++) {
		T v = va_arg(args, T);
		WriteFile(hFile, reinterpret_cast<LPCVOID>(&v), sizeof(T), nullptr, nullptr);
	}
	va_end(args);
}

void WriteBytesFloat(HANDLE hFile, LPCSTR buf, float v1, float v2 = 0.0f, float v3 = 0.0f, float v4 = 0.0f) {
	WriteFile(hFile, buf, strlen(buf), nullptr, nullptr);
	WriteFile(hFile, reinterpret_cast<LPCVOID>(&v1), sizeof(float), nullptr, nullptr);
	if (v2 != 0.0f)	WriteFile(hFile, reinterpret_cast<LPCVOID>(&v2), sizeof(float), nullptr, nullptr);
	if (v3 != 0.0f)	WriteFile(hFile, reinterpret_cast<LPCVOID>(&v3), sizeof(float), nullptr, nullptr);
	if (v4 != 0.0f)	WriteFile(hFile, reinterpret_cast<LPCVOID>(&v4), sizeof(float), nullptr, nullptr);
}

using ldouble = long double;
// Файл с разными переменными для тестирования ByteInterpreter
void CreateValuesFile(LPCWSTR lpszFileName) {
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_WRITE, FILE_SHARE_WRITE, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	
	wchar_t buf[8] = L"AZ-АЯ";
	WriteBytes<int64_t>(hFile, 1, "int64\t\t17\t\t\t=",						17i64);
	WriteBytes<int64_t>(hFile, 1, "\nint64\t\t-10\t\t\t=",				   -10i64);
	WriteBytes<int32_t>(hFile, 1, "\nint32\t\t15\t\t\t=",					15i32);
	WriteBytes<int32_t>(hFile, 1, "\nint32\t\t-9\t\t\t=",				   -9i32);
	WriteBytes<int16_t>(hFile, 1, "\nint16\t\t240\t\t\t=",					240i16);
	WriteBytes<int16_t>(hFile, 1, "\nint16\t\t-240\t\t\t=",				   -240i16);
	WriteBytes<int8_t>( hFile, 1, "\nint8\t\t45\t\t\t=",					45i8);
	WriteBytes<int8_t>( hFile, 1, "\nint8\t\t-12\t\t\t=",				   -12i8);
	WriteBytesFloat(	hFile,	  "\nfloat\t\t11.907\t\t\t=",				11.907f);
	WriteBytesFloat(	hFile,	  "\nfloat\t\t-190.5\t\t\t=",			   -190.5f);
	WriteBytes<double>( hFile, 1, "\ndouble\t\t1.05507\t\t\t=",				1.05507);
	WriteBytes<double>( hFile, 1, "\ndouble\t\t-0.89902\t\t=",			   -0.89902);
	WriteFile( hFile, "\nwchar\t\tAZ-АЯ\t\t\t=", 17, nullptr, nullptr);
	WriteFile( hFile, buf, wcslen(buf)*sizeof(wchar_t), nullptr, nullptr);
	
	WriteBytes<int64_t>(hFile, 3, "\n\nint64:\t\t35, -80, 502\t\t=",		35i64, -80i64, 502i64);
	WriteBytes<int32_t>(hFile, 3, "\nint32:\t\t1005, -803, 17028\t=",		1005i32, -803i32, 17028i32);
	WriteBytes<int16_t>(hFile, 4, "\nint16:\t\t-18, 65530, 225, 48\t=",	   -18i16, 65530i16, 225i16, 48i16);
	WriteBytes<int8_t>( hFile, 4, "\nint8:\t\t25, -13, 84, 241\t=",			25i8, -13i8, 84i8, 241i8);
	WriteBytesFloat(    hFile,	  "\nfloat:\t\t24.5, -11.2, 8.78, -0.5\t=", 24.5f, -11.2f, 8.78f, -0.5f);
	WriteBytesFloat(    hFile,    "\nfloat:\t\t0.875, 0.614, 0.73, 0.8\t=",	0.875f, 0.614f, 0.73f, 0.8f);
	WriteBytes<ldouble>(hFile, 4, "\nlong double:\tmin, max\t\t=", numeric_limits<ldouble>::min(), numeric_limits<ldouble>::max());
	WriteBytes<int8_t>( hFile, 4, "\nint8:\t\t15, 190, 240, 200\t=",		15i8, 190i8, 240i8, 200i8);

	WriteFile(hFile, "\n\nRGB\\A [81 115 148 75] \\ [125 150 175]\tViewMode: Floats", 57, nullptr, nullptr);
	WriteBytesFloat(hFile, "\nRGB\t0.49017 0.58824 0.68627\t\t=",			0.49017f, 0.58824f, 0.68627f);
	WriteBytesFloat(hFile, "\nRGBA\t0.31765 0.45098 0.58039 0.75\t=",		0.31765f, 0.45098f, 0.58039f, 0.75f);
	WriteBytesFloat(hFile, "\nHSLA\t0.58300 0.29000 0.45000 0.75\t=",		0.58333f, 0.29000f, 0.45000f, 0.75f);
	WriteBytesFloat(hFile, "\nHSL\t0.58333 0.23810 0.58824\t\t=",			0.58333f, 0.23810f, 0.58824f);
	WriteBytesFloat(hFile, "\nHSV\t0.58333 0.28572 0.68627\t\t=",			0.58333f, 0.28572f, 0.68627f);
	WriteBytesFloat(hFile, "\nHSI\t0.58333 0.16667 0.58824\t\t=",			0.58333f, 0.16667f, 0.58824f);
	WriteBytesFloat(hFile, "\nHWB\t0.58333 0.49019 0.31373\t\t=",			0.58333f, 0.49019f, 0.31373f);
	WriteBytesFloat(hFile, "\nCMY\t0.50980 0.41176 0.31373\t\t=",			0.50980f, 0.41176f, 0.31373f);
	WriteBytesFloat(hFile, "\nCMYK\t0.28571 0.14286 0 0.31373\t=",			0.28571f, 0.14286f, 0.00001f, 0.31373f);
	WriteBytesFloat(hFile, "\nXYZ\t0.27099 0.29267 0.44770\t\t=",			0.27099f, 0.29267f, 0.44770f);
	WriteBytesFloat(hFile, "\nxyY\t0.26795 0.28938 0.29267\t\t=",			0.26795f, 0.28938f, 0.29267f);
	WriteBytesFloat(hFile, "\nLab\t61.0161 -2.8802 -15.9346\t=",			61.0161f, -2.8802f, -15.9346f);
	WriteBytesFloat(hFile, "\nLCHab\t61.0161 16.1928 259.7542\t=",			61.0161f, 16.1928f, 259.7542f);
	WriteBytesFloat(hFile, "\nLuv\t61.0161 -13.724 -23.5076\t=",			61.0161f, -13.7235f, -23.5076f);
	WriteBytesFloat(hFile, "\nLCHuv\t61.0161 27.2202 239.7241\t=",			61.0161f, 27.2202f, 239.7241f);
	WriteBytesFloat(hFile, "\nHLab\t54.0990 -2.4060 -14.6860\t=",			54.0990f, -2.40600f, -14.6860f);
	WriteBytesFloat(hFile, "\nYUV\t0.57010 0.05717 -0.07010\t=",			0.57010f, 0.05717f, -0.07010f);
	WriteBytesFloat(hFile, "\nYCC\t0.63285 0.55717 0.42990\t\t=",			0.63285f, 0.55717f, 0.42990f);
	WriteBytesFloat(hFile, "\nYIQ\t0.57010 -0.0899 0.00977\t\t=",			0.57010f, -0.08993f, 0.00977f);
	WriteBytesFloat(hFile, "\nYDD\t0.57010 0.17479 0.15194\t\t=",			0.57010f, 0.17479f, 0.15194f);
	WriteFile(hFile, "\n\n", 2, nullptr, nullptr);
	CloseHandle(hFile);
}

void LoadFile(LPCWSTR lpszFileName) {
	HANDLE hFile = CreateFile(lpszFileName, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return;
	
	DWORD len = 0;
	g.m_bytes = new BYTE[BUFFER_SIZE]{ 0 };
	if (ReadFile(hFile, g.m_bytes, BUFFER_SIZE, &len, nullptr)) {
		// Чтобы отобразить в Edit необходимо добавить '\r' к '\n' и избавиться от '\0'...
		int n = 0;	// offset
		char buffer[BUFFER_SIZE*2];
		for (int i = 0; i < BUFFER_SIZE; i++) {
			if (g.m_bytes[i] == '\0')
				buffer[i+n] = ' ';
			else if (g.m_bytes[i] == '\n') {
				buffer[i+n] = '\r';
				buffer[i+n+1] = '\n';
				n++;
			}
			else
				buffer[i+n] = g.m_bytes[i];
		}
		SetWindowTextA(g.m_hEdit, buffer);
	//	Вывод напрямую из файла:
	//	dlg.SetBytes(buf, len, FALSE);
	}
	CloseHandle(hFile);
}

// ========= ========= ========= ========= ========= ========= ========= =========

// Для всего чего душе угодно...
void Other() {

}