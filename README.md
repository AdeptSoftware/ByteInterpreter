# ByteInterpreter
Динамическая библиотека, позволяющая интерпретировать байты/биты в различные типы данных и цвета.

Пример использования:
```
#pragma comment(lib, "Debug\\ByteInterpreter.lib")
#include "exports.h"
// ...
CPluginDialogExporter dlg;
HWND hWnd = dlg.Create(hInstDLL, hWndParent, nullptr);
// ...
ShowWindow(hWnd, SW_SHOW);
```

Кроме того:
ColorConverter.h позволяет конвертировать различные цветовые модели в RGB и обратно.
Реализованы преобразования из RGB в (и наоборот):
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
- LCH (ab и uv)
- LMS
- YCoCg
- YUV (возожно преобразовать в YIQ, YCbCr, YDbDr,...
- GrayScale
