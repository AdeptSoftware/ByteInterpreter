// dllmain.cpp : Определяет точку входа для приложения DLL.
#include "pch.h"
#include "PluginManager.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) { return TRUE; }


extern "C" { // Без этого функции AkelPad'ом не распознаются...

    void __declspec(dllexport) DllAkelPadID(PLUGINVERSION* pv) {
        pv->dwAkelDllVersion  = AKELDLL;
        pv->dwExeMinVersion3x = MAKE_IDENTIFIER(-1, -1, -1, -1);
        pv->dwExeMinVersion4x = MAKE_IDENTIFIER(4, 9, 7, 0);
        pv->pPluginName       = "ByteInterpreter";
    }

    /*void __declspec(dllexport) Settings(PLUGINDATA* pData) { 
        pData->dwSupport |= PDS_NOAUTOLOAD;
        if (pData->dwSupport & PDS_GETSUPPORT)
            return;
    }*/

    void __declspec(dllexport) Run(PLUGINDATA* pData) {
        // Достигается при установлении галочки автозапуска
        pData->dwSupport |= PDS_SUPPORTALL;    // Автозапуск разрешен
        if (pData->dwSupport & PDS_GETSUPPORT)
            return;
        // Достигается при вызове функции
        g_plugin.OnSwitchCheckBox(pData);
    }

}
