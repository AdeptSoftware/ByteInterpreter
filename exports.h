#pragma once
#include <Windows.h>

extern "C" {

	struct IPluginDialog {}; 

// � �������� ������ ��� �� ����� ���� �����! ����� � �������� AkelPad'� ����� �����
#ifdef _DEBUG 

	class __declspec(dllexport) CPluginDialogExporter {
		IPluginDialog* m_pDialog;
	public:
		~CPluginDialogExporter();

		// bytes - ������ ���� ����������� ��������!
		void SetBytes(BYTE* bytes, ULONG length, BOOL bIsWideChar);
		void UpdateOutput(BOOL bCheckKeyState = TRUE);

		HWND Create(HINSTANCE hInstDLL, HWND hWndParent, HICON hIcon);
	};

#endif // _DEBUG
}