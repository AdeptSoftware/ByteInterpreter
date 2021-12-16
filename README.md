# ByteInterpreter
������������ ����������, ����������� ���������������� �����/���� � ��������� ���� ������ � �����.

������ �������������:
```
#pragma comment(lib, "Debug\\ByteInterpreter.lib")
#include "exports.h"
// ...
CPluginDialogExporter dlg;
HWND hWnd = dlg.Create(hInstDLL, hWndParent, nullptr);
// ...
ShowWindow(hWnd, SW_SHOW);
```

����� ����:
ColorConverter.h ��������� �������������� ��������� �������� ������ � RGB � �������.
����������� �������������� �� RGB � (� ��������):
- RGBA
- CMY
- CMYK
- HSV
- HSL
- HSLA
- HSI
- HWB
- XYZ
- xyY
- L*a*b*
- Hunter L*a*b*
- LCH (ab � uv)
- LMS
- YCoCg
- YUV (������� ������������� � YIQ, YCbCr, YDbDr,...
- GrayScale
