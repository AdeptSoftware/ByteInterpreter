#pragma once						// PLUGINDATA::hInstanceEXE <=> ::GetModuleHandle(nullptr)
#define WIN32_LEAN_AND_MEAN			// PLUGINDATA::hInstanceDLL <=> ::GetModuleHandle(L"ByteInterpreter.dll")
#include "AkelEdit.h"
#include "AkelDLL.h"
#include "PluginDialog.h"

#define SEND(h, msg, wp, lp)	SendMessage(h, msg, reinterpret_cast<WPARAM>(wp), reinterpret_cast<LPARAM>(lp))

static UINT			  g_AKD				 = 0;
static WNDPROCDATA*	  g_DocumentProcData = nullptr;

LRESULT CALLBACK DocumentProc(HWND, UINT, WPARAM, LPARAM);


class CAkelPadPlugin {
public:
	CAkelPadPlugin()  { CDebugLogger::Attach(L"C:\\Users\\Adept\\Desktop\\log.txt"); }
	~CAkelPadPlugin() { CDebugLogger::Detach(); }

	// Включение/отключение плагина
	void OnSwitchCheckBox(PLUGINDATA* pData) {
		m_nState ? Disconnect(pData) : Connect(pData);
	}

	void Connect(PLUGINDATA* pData) {
		if (!m_nState) {
			m_dlg = new CPluginDialog(pData->hInstanceDLL);
			if (m_dlg->Create(pData->hMainWnd, pData->hMainIcon)) {
				m_hMainWnd = pData->hMainWnd;
				g_AKD = (pData->nMDI == WMD_MDI ? AKD_SETFRAMEPROC : AKD_SETMAINPROC);
				SEND(pData->hMainWnd, g_AKD, DocumentProc, &g_DocumentProcData);
				pData->nUnload = UD_NONUNLOAD_ACTIVE;  // Отображение статуса (Работает)
				m_nState = PluginState::HIDE;
			}
			else {
				delete m_dlg;
				MessageBoxW(nullptr, L"Failed to initialize plugin!", L"Plugin", MB_OK);
			}
		}
	}

	void Disconnect(PLUGINDATA* pData) {
		if (m_nState) {
			if (g_DocumentProcData) {
				SEND(pData->hMainWnd, g_AKD, nullptr, &g_DocumentProcData);
				g_DocumentProcData = nullptr;
			}
			OnDestroyDialog();
			pData->nUnload = UD_NONUNLOAD_NONACTIVE;   // Отображение статуса (Не работает)
		}
	}

	void OnDestroyDialog() {
		m_nState = PluginState::UNINIT;
		if (m_dlg) {
			m_dlg->DestroyWindow();
			delete m_dlg;
			m_dlg = nullptr;
		}
	}

	void OnSysCommand(WPARAM wParam) {
		if (m_nState) {
			if (wParam == SC_MINIMIZE) {
				m_dlg->ShowWindow(SW_HIDE);
				m_nState = PluginState::HIDE;
			}
			else if ((wParam == SC_MAXIMIZE || wParam == SC_RESTORE) && m_nState == PluginState::SHOW) { 
				m_dlg->ShowWindow(SW_SHOW);
				m_nState = PluginState::SHOW;
			}
		}
	}

	void OnSelChange(HWND hEdit) {
		EXGETTEXTRANGE tr;
		// Проверка есть ли выделение текста
		if (SEND(hEdit, AEM_GETSEL, nullptr, &tr.cr)) {
			tr.pText    = nullptr;
			tr.nNewLine = AELB_ASIS;
			// Получим выделенный текст
			ULONG uLen = SEND(m_hMainWnd, AKD_EXGETTEXTRANGE, hEdit, &tr);
			if (m_nState == PluginState::HIDE) { 
				m_dlg->ShowWindow(SW_SHOW);
				::SetFocus(hEdit);
				m_nState = PluginState::SHOW;
			}
			m_dlg->SetBytes(tr.pText, uLen);
			SEND(m_hMainWnd, AKD_FREETEXT, nullptr, tr.pText);
		}
		else {
			m_dlg->ShowWindow(SW_HIDE);
			m_nState = PluginState::HIDE;
		}
	}

private:
	enum PluginState { UNINIT, HIDE, SHOW, DISABLED };

	HWND		   m_hMainWnd	= nullptr;
	int			   m_nState		= PluginState::UNINIT;
	CPluginDialog* m_dlg		= nullptr;

	/*
	UINT GetLength(const AECHARRANGE& cr) {
		if (cr.ciMin.lpLine == cr.ciMax.lpLine)
			return cr.ciMax.nCharInLine-cr.ciMin.nCharInLine;
		UINT uLen = cr.ciMax.nCharInLine-cr.ciMin.nCharInLine;
		for (AELINEDATA* line = cr.ciMin.lpLine; line != cr.ciMax.lpLine; line = line->next) {
			uLen += line->nLineLen;
			if (line->nLineBreak == AELB_EOF)		break;
			if (line->nLineBreak == AELB_WRAP)		continue;		// Разорвано
			if (line->nLineBreak == AELB_RN)		uLen += 2;		// \r\n
			else if (line->nLineBreak == AELB_RRN)	uLen += 3;		// \r\r\n
			else									uLen++;			// \n
		}
		return uLen;
	}
	*/
};


static CAkelPadPlugin g_plugin;


LRESULT CALLBACK DocumentProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_SYSCOMMAND)
		g_plugin.OnSysCommand(wParam);
	else if (uMsg == WM_DESTROY)
		g_plugin.OnDestroyDialog();
	else if (uMsg == WM_NOTIFY && wParam == ID_EDIT) {
		NMHDR* pNMHDR = reinterpret_cast<NMHDR*>(lParam);
		if (pNMHDR->code == EN_SELCHANGE)
			g_plugin.OnSelChange(pNMHDR->hwndFrom);
	}
	return g_DocumentProcData->NextProc(hWnd, uMsg, wParam, lParam);
}