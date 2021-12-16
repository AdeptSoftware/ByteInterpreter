#include "pch.h"
#include "exports.h"
#include "PluginDialog.h"

CPluginDialog* cast(IPluginDialog* pDlg) {
	return reinterpret_cast<CPluginDialog*>(pDlg);
}

#ifdef _DEBUG

	CPluginDialogExporter::~CPluginDialogExporter() {
		if (m_pDialog) {
			CPluginDialog* pDialog = cast(m_pDialog);
			delete pDialog;
		}
	}

	HWND CPluginDialogExporter::Create(HINSTANCE hInstDLL, HWND hWndParent, HICON hIcon) {
		CPluginDialog* pDlg = new CPluginDialog(hInstDLL);
		if (pDlg) {
			HWND hWnd = pDlg->Create(hWndParent, hIcon); 
			if (hWnd) {
				m_pDialog = reinterpret_cast<IPluginDialog*>(pDlg);
				return hWnd;
			}
		
			delete pDlg;
			pDlg = nullptr;
		}
		return nullptr;
	}


	void CPluginDialogExporter::SetBytes(BYTE* bytes, ULONG length, BOOL bIsWideChar) {
		cast(m_pDialog)->SetBytes(bytes, length, bIsWideChar);
	}

	void CPluginDialogExporter::UpdateOutput(BOOL bCheckKeyState) {
		cast(m_pDialog)->UpdateOutput(bCheckKeyState);
	}

#endif // _DEBUG