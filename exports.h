#pragma once
#include <Windows.h>

extern "C" {

	struct IPluginDialog {}; 

// В релизной версии нам не нужен этот класс! Иначе в плагинах AkelPad'а будет мусор
#ifdef _DEBUG 

	class __declspec(dllexport) CPluginDialogExporter {
		IPluginDialog* m_pDialog;
	public:
		~CPluginDialogExporter();

		// bytes - должны быть динамически выделены!
		void SetBytes(BYTE* bytes, ULONG length, BOOL bIsWideChar);
		void UpdateOutput(BOOL bCheckKeyState = TRUE);

		HWND Create(HINSTANCE hInstDLL, HWND hWndParent, HICON hIcon);
	};

#endif // _DEBUG
}