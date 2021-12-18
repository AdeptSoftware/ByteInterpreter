#include "pch.h"
#include "ColorConverter.h"		// Использованы данные из: brucelindbloom.com, easyrgb.com, wikipedia.org
#include "m3x3_functions.h"		// Как-нибудь потом реализовать: ICtCp
#include <math.h>

using namespace ByteInterpreter;
using namespace ByteInterpreter::Colors;

const double HLAB_KA = 175.0/198.04;
const double HLAB_KB = 70.0/218.11;
const double PI		 = 3.14159265359;
const size_t MATSIZE = sizeof(double)*9;
const size_t VECSIZE = sizeof(double)*3;

// ========= ========= ========= ========= ========= ========= ========= =========

const double XYZ_ChromaticAdaptationMethod[3][3][3] = {
	{ 1.0000000, 0.0000000,  0.0000000,  0.0000000, 1.0000000, 0.0000000, 0.0000000,  0.0000000, 1.0000000 },
	{ 0.8951000, 0.2664000, -0.1614000, -0.7502000, 1.7135000, 0.0367000, 0.0389000, -0.0685000, 1.0296000 },
	{ 0.4002400, 0.7076000, -0.0808100, -0.2263000, 1.1653200, 0.0457000, 0.0000000,  0.0000000, 0.9182200 }
};

const double XYZ_InvChromaticAdaptationMethod[3][3][3] = { // Обратные матрицы XYZ_ChromaticAdaptationMethod 
	{ 1.0000000,  0.0000000, 0.0000000, 0.0000000, 1.0000000,  0.0000000,  0.0000000, 0.0000000, 1.0000000 },
	{ 0.9869929, -0.1470543, 0.1599627, 0.4323053, 0.5183603,  0.0492912, -0.0085287, 0.0400428, 0.9684867 },
	{ 1.8599364, -1.1293816, 0.2198974, 0.3611914, 0.6388125, -0.0000064,  0.0000000, 0.0000000, 1.0890636 }
};

// E to any Reference White (Illuminant). Адаптационные матрицы.
const double XYZ_ReferenceWhite[2][20][3] = {
    // Standart Observer = 2 deg
    { { 1.09850, 1.0, 0.35585 }, { 0.99093, 1.0, 0.85313 }, { 0.98074, 1.0, 1.18232 }, { 0.96422, 1.0, 0.82521 }, { 0.95682, 1.0, 0.92149 },
      { 0.95047, 1.0, 1.08883 }, { 0.94972, 1.0, 1.22638 }, { 1.00000, 1.0, 1.00000 }, { 0.92834, 1.0, 1.03665 }, { 0.99187, 1.0, 0.67395 },
      { 1.03754, 1.0, 0.49861 }, { 1.09147, 1.0, 0.38813 }, { 0.90872, 1.0, 0.98723 }, { 0.97309, 1.0, 0.60191 }, { 0.95044, 1.0, 1.08755 },
      { 0.96413, 1.0, 0.82333 }, { 1.00365, 1.0, 0.67868 }, { 0.96174, 1.0, 0.81712 }, { 1.00966, 1.0, 0.64370 }, { 1.08046, 1.0, 0.39228 } },
    // Standart Observer = 10 deg
    { { 1.11144, 1.0, 0.35200 }, { 0.99178, 1.0, 0.84349 }, { 0.97285, 1.0, 1.16145 }, { 0.96720, 1.0, 0.81427 }, { 0.95799, 1.0, 0.90926 },
      { 0.94811, 1.0, 1.07304 }, { 0.94416, 1.0, 1.20641 }, { 1.00000, 1.0, 1.00000 }, { 0.94791, 1.0, 1.03191 }, { 1.03280, 1.0, 0.69026 },
      { 1.08968, 1.0, 0.51965 }, { 1.14961, 1.0, 0.40963 }, { 0.93369, 1.0, 0.98636 }, { 1.02148, 1.0, 0.62074 }, { 0.95792, 1.0, 1.07687 },
      { 0.97115, 1.0, 0.81135 }, { 1.02116, 1.0, 0.67826 }, { 0.99001, 1.0, 0.83134 }, { 1.03866, 1.0, 0.65627 }, { 1.11428, 1.0, 0.40353 } }
};

// ========= ========= ========= ========= ========= ========= ========= =========

const double LMS_Matrix[7][3][3] = {	// XYZ -> LMS							   RGB -> LMS:    Lmin      Mmin     Smin     Lmax     Mmax     Smax
	{ 0.38971,  0.68898, -0.07868, -0.22981, 1.18340, 0.04641, 0.0000,  0.0000, 1.00000 },	//  0.000000  0.000000 0.000000 0.973720 1.015510 1.088830 // D65/2
	{ 0.40020,  0.70760, -0.08080, -0.22630, 1.16530, 0.04570, 0.0000,  0.0000, 0.91820 },	//  0.000000  0.000000 0.000000 1.027000 0.984700 0.918200 // E/2
	{ 1.94735, -1.41445,  0.36476,  0.68990, 0.34832, 0.00000, 0.0000,  0.0000, 1.93485 },	// -0.295770  0.000000 0.000000 1.067250 0.988780 2.005760 // F/2					   
	{ 0.85620,  0.33720, -0.19340, -0.83600, 1.83270, 0.00330, 0.3570, -0.0469, 1.01120 },	// -0.004960 -0.015430 0.000000 0.945370 1.057130 1.393440 // CIECAM97s (Linear)
	{ 0.89510,  0.26640, -0.16140, -0.75020, 1.71350, 0.03670, 0.0389, -0.0685, 1.02960 },	//  0.000000  0.000000 0.000000 0.941430 1.040420 1.089530 // CIECAM97s (Bradford)
	{ 0.73280,  0.42960, -0.16240, -0.70360, 1.69750, 0.00610, 0.0030,  0.0136, 0.98340 },	//  0.000000  0.000000 0.000000 0.949280 1.035390 1.087210 // CIECAM02 (Linear?)
	{ 1.00000, -1.00000,  0.00000,  1.00000, 1.00000, 0.00000, 0.0000,  0.0000, 1.00000 }	// -0.357580  0.000000 0.000000 0.308050 1.950470 1.088830 // JPEG XL (LMS2XYB)
};

const double LMS_InvMatrix[7][3][3] = { // LMS -> XYZ
	{ 1.91019683, -1.11212389,  0.20190796,  0.37095009,  0.62905426, -0.00000806, -0.00000000, -0.00000000,  1.00000000 },
	{ 1.86006661, -1.12948008,  0.21989830,  0.36122292,  0.63880431, -0.00000713, -0.00000000, -0.00000000,  1.08908734 },
	{ 0.21057595,  0.85510207, -0.03969800, -0.41707725,  1.17726539,  0.07862785,  0.00000000, -0.00000000,  0.51683593 },
	{ 0.93075221, -0.16668047,  0.17855768,  0.42512585,  0.46946532,  0.07977661, -0.30888067,  0.08061991,  0.92958508 },
	{ 0.98699291, -0.14705426,  0.15996265,  0.43230527,  0.51836027,  0.04929123, -0.00852866,  0.04004282,  0.96848670 },
	{ 1.09612382, -0.27886900,  0.18274518,  0.45436904,  0.47353315,  0.07209780, -0.00962761, -0.00569803,  1.01532564 },
	{ 0.50000000,  0.50000000, -0.00000000, -0.50000000,  0.50000000, -0.00000000,  0.00000000, -0.00000000,  1.00000000 }
};

// ========= ========= ========= ========= ========= ========= ========= =========

const double YUV_ConversionData[4][5] = {
	//  Wr		Wg      Wb	   Umax	  Vmax						RGB -> YUV
	{ 0.2990, 0.5870, 0.1140, 0.436, 0.615 }, // YSD	[0..255], ±111.18, ±156.825		
	{ 0.2990, 0.5870, 0.1140, 0.500, 0.500 }, // YHD	[0..255], ±127.50, ±127.500	
	{ 0.2126, 0.7152, 0.0722, 0.436, 0.615 }, // YSD	[0..255], ±111.18, ±156.825	
	{ 0.2627, 0.6780, 0.0593, 0.500, 0.500 }  // YUHD	[0..255], ±127.50, ±127.500
}; // Wg = 1.0-Wr-Wb, но из таблицы лучше не удалять, так как может привести к путанице

// ========= ========= ========= ========= ========= ========= ========= =========

CColorConverter::CColorConverter() {
	m_xyzChromaticAdaptationMethod	= XYZ_METHOD::BRADFORD;
	YUV_CreateTransformMatrix(YUV_ConversionData[0]);
	m_yuvStandardBT = BT_STANDARD::BT470;
	XYZ_CreateTransformationMatrix();
}

void CColorConverter::RGBA_SetBackgroundColor(const SRGB& clr) {
	m_clrBackground = clr;
}

// ========= ========= ========= ========= ========= ========= ========= =========
// ========= ========= ========= ======= XYZ ======= ========= ========= =========
// ========= ========= ========= ========= ========= ========= ========= =========

void CColorConverter::XYZ_UseAdaptationMethod(XYZ_METHOD method) {
	m_xyzChromaticAdaptationMethod = method;
}

const double* CColorConverter::XYZ_GetReferenceWhite(const XYZ_OBSERVER& stdObserver, const XYZ_ILLUMINANT& illuminant) {
	return XYZ_ReferenceWhite[e_cast(stdObserver)][e_cast(illuminant)];
}

void CColorConverter::XYZ_Convert(XYZ& out, const XYZ& in) {
	auto  Ma = XYZ_ChromaticAdaptationMethod[e_cast(m_xyzChromaticAdaptationMethod)];
	auto IMa = XYZ_InvChromaticAdaptationMethod[e_cast(m_xyzChromaticAdaptationMethod)];

	double temp[3][3];
	// Скалирование компонентов вектора
	mul3x3(Ma, XYZ_GetReferenceWhite(out.stdObserver, out.illuminant), temp[0]);
	mul3x3(Ma, XYZ_GetReferenceWhite(in.stdObserver,  in.illuminant),  temp[1]);
	double matrix[3][3] = { temp[0][0]/temp[1][0], 0, 0, 0, temp[0][1]/temp[1][1], 0, 0, 0, temp[0][2]/temp[1][2] };
	// Составим адапционную матрицу
	mul3x3(IMa, matrix, temp);
	mul3x3(temp, Ma, matrix);
	// Получение XYZ c другим Reference White
	mul3x3(matrix, &in.X, &out.X);
}

// Использует на данный момент только sRGB D65
void CColorConverter::XYZ_CreateTransformationMatrix() {
	m_xyzObserer    = XYZ_OBSERVER::CIE1931_2;
	m_xyzIlluminant	= XYZ_ILLUMINANT::D65;
	double temp[3][3];
	// Colorimetric specifications for the working spaces. Primary(xyY):
	// sRGB D50: R = 0.648431, 0.330856, 0.222491 | G = 0.321152, 0.597871, 0.716888 | B = 0.155886, 0.066044, 0.060621
	// sRGB D65: R = 0.640000, 0.330000, 0.212656 | G = 0.300000, 0.600000, 0.715158 | B = 0.150000, 0.060000, 0.072186
	double V[3][3] = { 0.64/0.33, 0.3/0.6, 0.15/0.06, 1.0, 1.0, 1.0, (1.0-0.64-0.33)/0.33, (1.0-0.3-0.6)/0.6, (1.0-0.15-0.06)/0.06 };
	inv3x3(V, temp);							    /* └ всегда 1.0 */
	// Промежуточный вектор S запишем в m_TransformMatrixRGBtoXYZ, в которой д.б { S[0]*V[1][0], S[1]*V[1][1], S[2]*V[1][2] }
	mul3x3(temp, XYZ_GetReferenceWhite(m_xyzObserer, m_xyzIlluminant), m_TransformMatrixRGBtoXYZ[1]);
	// Составим матрицу трансформации RGB в XYZ и получим матрицу XYZ в RGB
	m_TransformMatrixRGBtoXYZ[0][0] = m_TransformMatrixRGBtoXYZ[1][0]*V[0][0];
	m_TransformMatrixRGBtoXYZ[0][1] = m_TransformMatrixRGBtoXYZ[1][1]*V[0][1];
	m_TransformMatrixRGBtoXYZ[0][2] = m_TransformMatrixRGBtoXYZ[1][2]*V[0][2];
	m_TransformMatrixRGBtoXYZ[2][0] = m_TransformMatrixRGBtoXYZ[1][0]*V[2][0];
	m_TransformMatrixRGBtoXYZ[2][1] = m_TransformMatrixRGBtoXYZ[1][1]*V[2][1];
	m_TransformMatrixRGBtoXYZ[2][2] = m_TransformMatrixRGBtoXYZ[1][2]*V[2][2];
	inv3x3(m_TransformMatrixRGBtoXYZ, m_TransformMatrixXYZtoRGB);
}

void CColorConverter::XYZD65toRGB(SRGB& out, const XYZ& in) {
	double* v = &out.R;
	if (in.illuminant != XYZ_ILLUMINANT::D65 || in.stdObserver != XYZ_OBSERVER::CIE1931_2)
		return; 
	/*{ XYZ temp; // На данный момент неизвестно как сделать на прямую через матрицу трансформации
		temp.illuminant  = XYZ_ILLUMINANT::D65;
		temp.stdObserver = XYZ_OBSERVER::CIE1931_2;
		XYZ_Convert(temp, in);
		mul3x3(m_TransformMatrixXYZtoRGB, &temp.X, v);
	} else*/ // Автоконвертация (Работает, но усложняет вычисления)
	mul3x3(m_TransformMatrixXYZtoRGB, &in.X, v);
    for (int i = 0; i < 3; i++) {
        if (v[i] > 0.0031308)	v[i]  = (1.055*pow(v[i], 1.0/2.4))-0.055;
        else					v[i] *= 12.92;
    }
}

void CColorConverter::RGBtoXYZD65(XYZ& out, const SRGB& in) {
	double v[3];
	memcpy_s(v, VECSIZE, &in.R, VECSIZE);
	for (int i = 0; i < 3; i++) {
		if (v[i] <= 0.04045)	v[i] /= 12.92;
		else					v[i] = pow((v[i]+0.055)/1.055, 2.4);
    }
	mul3x3(m_TransformMatrixRGBtoXYZ, v, &out.X);
/*	if (out.illuminant != XYZ_ILLUMINANT::D65 || out.stdObserver != XYZ_OBSERVER::CIE1931_2) {
		XYZ temp; // На данный момент неизвестно как сделать на прямую через матрицу трансформации
		temp.illuminant  = out.illuminant;
		temp.stdObserver = out.stdObserver;
		out.illuminant   = XYZ_ILLUMINANT::D65;
		out.stdObserver  = XYZ_OBSERVER::CIE1931_2; 
		XYZ_Convert(temp, out);
		out = temp;
	}*/ // Автоконвертация (Работает, но усложняет вычисления).
	// При раскоментировании кода убрать строки ниже
	out.illuminant   = XYZ_ILLUMINANT::D65;
	out.stdObserver  = XYZ_OBSERVER::CIE1931_2; 
}

// ========= ========= ========= ========= ========= ========= ========= =========
// ========= ========= ========= ==== HSL & HSV ==== ========= ========= =========
// ========= ========= ========= ========= ========= ========= ========= =========

double Hue2ColorComponent(double& v1, double& v2, double H) {
    if (H < 0.0)        H += 6.0;
    else if (H > 6.0)   H -= 6.0;
    if (H < 1.0)        return v1+((v2-v1)*H);
    if (H < 3.0)        return v2;
    if (H < 4.0)        return v1+((v2-v1)*(4.0-H));
    return v1;
}

void CColorConverter::HSL2RGB(SRGB& out, const HSL& in) {
	if (in.S == 0.0)
		GrayScale2RGB(out, in.L);
	else {
		double v2;
		if (in.L >= 0.5)	v2 = (in.L+in.S)-(in.S*in.L);
		else				v2 = in.L*(1+in.S);
		double v1 = (2*in.L)-v2;

		out.B = in.H*6;	// Для минимизации расчетов
		out.R = Hue2ColorComponent(v1, v2, out.B+2.0);
		out.G = Hue2ColorComponent(v1, v2, out.B);
		out.B = Hue2ColorComponent(v1, v2, out.B-2.0);
	}
}
 
#define _VAR1      (in.V*(1.0-in.S))
#define _VAR2      (in.V*(1.0-in.S*(out.B-h)))
#define _VAR3      (in.V*(1.0-in.S*(1.0-(out.B-h))))

void CColorConverter::HSV2RGB(SRGB& out, const HSV& in) {
    if (in.S == 0.0)
		GrayScale2RGB(out, in.V);
	else {
		out.B = fmod(in.H*6.0, 6.0);
		// Соответствует проверке h <= H/60 deg, где H в диапазоне [0.0..360.0) deg
		// Нужно именно отбрасывание целой части (из-за этого результат будет в пределах [0.0 .. 1.0])
		int h = static_cast<int>(out.B);
		if (h == 0)		 { out.R =  in.V; out.G = _VAR3; out.B = _VAR1; }
		else if (h == 1) { out.R = _VAR2; out.G =  in.V; out.B = _VAR1; }
		else if (h == 2) { out.R = _VAR1; out.G =  in.V; out.B = _VAR3; }
		else if (h == 3) { out.R = _VAR1; out.G = _VAR2; out.B =  in.V; }
		else if (h == 4) { out.R = _VAR3; out.G = _VAR1; out.B =  in.V; }
		else			 { out.R =  in.V; out.G = _VAR1; out.B = _VAR2; }
	}
}

enum class _HSX : int { HSL, HSV, HSI };

void _RGB2HSx(double out[3], const SRGB& in, _HSX hsx) {
	double vMin  = min(min(in.R, in.G), in.B);
	double vMax  = max(max(in.R, in.G), in.B);
	double delta = vMax-vMin;
	out[0] = 0.0;

	if (hsx == _HSX::HSV)		out[2] = vMax;					// V
	else if (hsx == _HSX::HSI)	out[2] = (in.R+in.G+in.B)/3.0;	// I
	else						out[2] = (vMax+vMin)/2.0;		// L
	// S
	if (delta == 0) {
		out[1] = 0.0;
		return;
	}
	if (hsx == _HSX::HSV)		out[1] = delta/vMax;
	else if (hsx == _HSX::HSI)	out[1] = 1.0-(vMin/out[2]);		// Если delta == 0 => out[2] = 0
	else {
		if (out[2] < 0.5)		out[1] = delta/(vMax+vMin);
		else					out[1] = delta/(2.0-(vMax+vMin));
	}
	// H
	delta *= 6.0;
	double temp[3];
	memcpy_s(temp, VECSIZE, &in.R, VECSIZE);
	for (int i = 0; i < 3; i++)
		temp[i] = ((vMax-temp[i])/delta)+0.5;

	if (in.R == vMax)			out[0] = temp[2]-temp[1];
	else if (in.G == vMax)		out[0] = temp[0]-temp[2]+(1.0/3.0);
	else if (in.B == vMax)		out[0] = temp[1]-temp[0]+(2.0/3.0);

	if (out[0] < 0.0)			out[0] += 1.0;
	else if (out[0] > 1.0)		out[0] -= 1.0;
}

void CColorConverter::RGB2HSL(HSL& out, const SRGB& in) {
	_RGB2HSx(&out.H, in, _HSX::HSL);
}

void CColorConverter::RGB2HSV(HSV& out, const SRGB& in) {
	_RGB2HSx(&out.H, in, _HSX::HSV);
}

void CColorConverter::RGB2HSI(HSI& out, const SRGB& in) {
	_RGB2HSx(&out.H, in, _HSX::HSI);
	// i.stack.imgur.com/ukpFr.jpg
	// Нашел формулу. Выдает близкие результаты к истинной H, что даёт погрешность...
	/*out.I = in.R-in.B;	// Промежуточные результаты
	out.S = in.R-in.G;		// Промежуточные результаты

	// Проблема где-то здесь
	out.H = acos((0.5*(out.S+out.I))/sqrt((out.S*out.S)+(out.I*(in.G-in.B))));

	if (in.G < in.B)
		out.H = (2*PI)-out.H;
	out.H *= 1.0/(PI*2);

	out.I = (in.R+in.G+in.B)/3.0;
	out.S = 1.0-(min(in.R, min(in.G, in.B))/out.I);*/
}

void CColorConverter::HSI2RGB(SRGB& out, const HSI& in) {
	out.B = in.H*6.0;						// H'
	out.G = 1.0-fabs(fmod(out.B, 2.0)-1.0); // Z
	out.R = (3.0*in.I*in.S)/(1+out.G);		// Chroma
	out.G *= out.R;							// X
	double m = in.I*(1.0-in.S);
	if (out.B <= 1.0)		out = { m+out.R, m+out.G, m		  };
	else if (out.B <= 2.0)	out = { m+out.G, m+out.R, m		  };
	else if (out.B <= 3.0)	out = { m,		 m+out.R, m+out.G };
	else if (out.B <= 4.0)	out = { m,		 m+out.G, m+out.R };
	else if (out.B <= 5.0)	out = { m+out.G, m,		  m+out.R };
	else					out = { m+out.R, m,		  m+out.G };
}

void CColorConverter::HSV2HWB(HWB& out, const HSV& in) {
	out.H = in.H;
	out.W = (1.0-in.S)*in.V;
	out.B = 1.0-in.V;
}

void CColorConverter::HWB2HSV(HSV& out, const HWB& in) {
	out.H = in.H;
	out.S = 1.0-(in.W/(1.0-in.B));
	out.V = 1.0-in.B;
}

// ========= ========= ========= ========= ========= ========= ========= =========
// ========= ========= ========= ==== CMY / CMYK === ========= ========= =========
// ========= ========= ========= ========= ========= ========= ========= =========

void CColorConverter::RGB2CMY(CMY& out, const SRGB& in) {
	out.C = 1.0-in.R;	out.M = 1.0-in.G;	out.Y = 1.0-in.B;
}

void CColorConverter::CMY2RGB(SRGB& out, const CMY& in) {
	out.R = 1.0-in.C;	out.G = 1.0-in.M;	out.B = 1.0-in.Y;
}

void CColorConverter::CMYK2CMY(CMY& out, const CMYK& in) {
	out.Y = 1.0-in.K;				// Промежуточный результат
	out.C = (in.C*out.Y)+in.K;
	out.M = (in.M*out.Y)+in.K;
	out.Y = (in.Y*out.Y)+in.K;
}

void CColorConverter::CMY2CMYK(CMYK& out, const CMY& in) {
	out.K = min(min(min(1.0, in.C), in.M), in.Y);
	if (out.K == 1.0)
		out.C = out.M = out.Y = 0.0;
	else {
		out.C = (in.C-out.K)/(1.0-out.K);
		out.M = (in.M-out.K)/(1.0-out.K);
		out.Y = (in.Y-out.K)/(1.0-out.K);
	}
}

void CColorConverter::CMYK2RGB(SRGB& out, const CMYK& in) {
	out.B = 1.0-in.K;			// Промежуточный результат
	out.R = (1.0-in.C)*out.B;
	out.G = (1.0-in.M)*out.B;
	out.B = (1.0-in.Y)*out.B;
}

// ========= ========= ========= ========= ========= ========= ========= =========
// ========= ========= ========= ======= YUV ======= ========= ========= =========
// ========= ========= ========= ========= ========= ========= ========= ========= [!]
// https://en.wikipedia.org/wiki/YUV + ей подобные и стандарты BT (Rec.)					    

void CColorConverter::YUV_GetTransformMatrix(double* m3x3, BOOL b2RGB) {
	memcpy_s(m3x3, MATSIZE, b2RGB ? m_TransformMatrixYUVtoRGB : m_TransformMatrixRGBtoYUV, MATSIZE);
}

void CColorConverter::YUV_GetConversionData(double* W5, BT_STANDARD standard) {
	size_t sz = sizeof(double)*5;
	if (standard != BT_STANDARD::CUSTOM)
		memcpy_s(W5, sz, YUV_ConversionData[e_cast(standard)], sz);
	else
		ZeroMemory(W5, sz);
}

void CColorConverter::YUV_CreateTransformMatrix(const double W[5]) {
	double M[3][3] = {
		{ 1.0, 0.0,							  (1.0-W[0])/W[4]				},
		{ 1.0, (W[2]*(W[2]-1.0))/(W[3]*W[1]), (W[0]*(W[0]-1.0))/(W[4]*W[1]) },
		{ 1.0, (1.0-W[2])/W[3],				  0.0							}
	};
	memcpy_s(m_TransformMatrixYUVtoRGB, MATSIZE, M, MATSIZE);
	inv3x3(m_TransformMatrixYUVtoRGB, m_TransformMatrixRGBtoYUV);
	m_yuvStandardBT = BT_STANDARD::CUSTOM;
}

void CColorConverter::YUV_RotateTransformMatrix(const double deg) {
	if (deg == 0.0)
		return;

	double radians = deg*((2*PI)/360.0);
	double ksin	   = sin(radians);
	double kcos    = cos(radians);
	double M[3][3] = { 1.0, 0.0, 0.0, 0.0, -ksin, kcos, 0.0, kcos, ksin };

	double temp[3][3];
	mul3x3(M, m_TransformMatrixRGBtoYUV, temp);
	inv3x3(temp, m_TransformMatrixYUVtoRGB);
	memcpy_s(m_TransformMatrixRGBtoYUV, MATSIZE, temp, MATSIZE);
	m_yuvStandardBT = BT_STANDARD::CUSTOM;
}

void CColorConverter::YUV_UseBTStandard(BT_STANDARD standard) {
	if (standard == BT_STANDARD::CUSTOM || m_yuvStandardBT == standard)
		return;
	YUV_CreateTransformMatrix(YUV_ConversionData[e_cast(standard)]);
	m_yuvStandardBT = standard;
}

void CColorConverter::YUV2RGB(SRGB& out, const YUV& in) {
	mul3x3(m_TransformMatrixYUVtoRGB, &in.Y, &out.R);
}

void CColorConverter::RGB2YUV(YUV& out, const SRGB& in) {
	mul3x3(m_TransformMatrixRGBtoYUV, &in.R, &out.Y);
}

// ========= ========= ========= ========= ========= ========= ========= =========

void CColorConverter::xyY2XYZ(XYZ& out, const XYY& in) {
	if (in.y == 0.0)
		out.X = out.Y = out.Z = 0.0;
	else {
		out.X = (in.x*in.Y)/in.y;
		out.Y = in.Y;
		out.Z = ((1.0-in.x-in.y)*in.Y)/in.y;
	}
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
}

void CColorConverter::XYZ2xyY(XYY& out, const XYZ& in) {
	out.Y = in.X+in.Y+in.Z;		// Промежуточный результат
	if (out.Y == 0.0) {
		const double* RW = XYZ_GetReferenceWhite(in.stdObserver, in.illuminant);
		out.x = RW[0];
		out.y = RW[1];
	}
	else {
		out.x = in.X/out.Y;
		out.y = in.Y/out.Y;
	}
	out.Y			= in.Y;
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
}

void CColorConverter::XYZ2HunterLab(HLAB& out, const XYZ& in) {
	const double* RW = XYZ_GetReferenceWhite(in.stdObserver, in.illuminant);

	out.L = sqrt(in.Y/RW[1]);
	out.a = HLAB_KA*(RW[0]+RW[1])*(((in.X/RW[0])-(in.Y/RW[1]))/out.L);
	out.b = HLAB_KB*(RW[1]+RW[2])*(((in.Y/RW[1])-(in.Z/RW[2]))/out.L);
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
}

void CColorConverter::HunterLab2XYZ(XYZ& out, const HLAB& in) {
	const double* RW = XYZ_GetReferenceWhite(in.stdObserver, in.illuminant);

	out.X = in.L/RW[1];			// Промежуточный результат
	out.Y = out.X*out.X;
	out.Z = out.Y/RW[1];		// Промежуточный результат
	out.X = (((in.a/(HLAB_KA*(RW[0]+RW[1])))*sqrt(out.Z))+out.Z)*RW[0];
	out.Z = -(((in.b/(HLAB_KB*(RW[1]+RW[2])))*sqrt(out.Z))-out.Z)*RW[2];
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
}

void CColorConverter::XYZ2CIELAB(LAB& out, const XYZ& in) {
	const double* RW = XYZ_GetReferenceWhite(in.stdObserver, in.illuminant);
	const double* xyz = &in.X;

	double v[3];
	for (int i = 0; i < 3; i++) {
		v[i] = xyz[i]/RW[i];
		if (v[i] > 0.008856)	v[i] = pow(v[i], 1.0/3.0);
		else					v[i] = (7.787*v[i])+(16.0/116.0);
	}
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
	out.L = (116.0*v[1])-16.0;
	out.a = 500.0*(v[0]-v[1]);
	out.b = 200.0*(v[1]-v[2]);
}

void CColorConverter::CIELAB2XYZ(XYZ& out, const LAB& in) {
	const double* RW = XYZ_GetReferenceWhite(in.stdObserver, in.illuminant);
	double* xyz = &out.X;

	double v[3];
	v[1] = (in.L+16.0)/116.0;
	v[0] = (in.a/500.0)+v[1];
	v[2] = v[1]-(in.b/200.0);
	double sqv = 0.0;
	for (int i = 0; i < 3; i++) {
		sqv = pow(v[i], 3.0);
		if (sqv > 0.008856)		v[i] = sqv;
		else					v[i] = (v[i]-(16.0/116.0))/7.787;
		xyz[i] = v[i]*RW[i];
	}
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
}

void _CIELxx2CIELCHxx(double* out, const double* in) {
	out[2] = atan2(in[2], in[1]); // Убережет так же от a == 0
	if (out[2] > 0.0)	out[2] = (out[2]/PI)*180.0;
	else				out[2] = 360.0+((out[2]/PI)*180.0);
	out[1] = sqrt((in[1]*in[1])+(in[2]*in[2]));
	out[0] = in[0];
}

void _CIELCHxx2CIELxx(double* out, const double* in) {
	out[1] = cos((PI/180.0)*in[2])*in[1];
	out[2] = sin((PI/180.0)*in[2])*in[1];
	out[0] = in[0];
}

void CColorConverter::CIELAB2CIELCHab(LCH& out, const LAB& in) {
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
	_CIELxx2CIELCHxx(&out.L, &in.L);
}

void CColorConverter::CIELCHab2CIELAB(LAB& out, const LCH& in) {
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
	_CIELCHxx2CIELxx(&out.L, &in.L);
}

void CColorConverter::CIELUV2CIELCHuv(LCH& out, const LUV& in) {
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
	_CIELxx2CIELCHxx(&out.L, &in.L);
}

void CColorConverter::CIELCHuv2CIELUV(LUV& out, const LCH& in) {
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
	_CIELCHxx2CIELxx(&out.L, &in.L);
}

void CColorConverter::XYZ2CIELUV(LUV& out, const XYZ& in) {
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
	out.L = in.X+(15.0*in.Y)+(3.0*in.Z);			// Промежуточный результат
	if (out.L == 0.0) {
		out.u = out.v = 0.0;
		return;
	}
	const double* RW = XYZ_GetReferenceWhite(in.stdObserver, in.illuminant);
	out.v = RW[0]+(15.0*RW[1])+(3.0*RW[2]);			// Промежуточный результат
	out.u = ((4.0*in.X)/out.L)-((4.0*RW[0])/out.v);
	out.v = ((9.0*in.Y)/out.L)-((9.0*RW[1])/out.v);
	out.L = in.Y;									// Промежуточный результат
	if (out.L > 0.008856)	out.L = pow(out.L, 1.0/3.0);
	else					out.L = (7.787*out.L)+(16.0/116.0);
	out.L = (116.0*out.L)-16.0;
	out.u = 13.0*out.L*out.u;
	out.v = 13.0*out.L*out.v;
}

void CColorConverter::CIELUV2XYZ(XYZ& out, const LUV& in) {
	out.illuminant  = in.illuminant;
	out.stdObserver = in.stdObserver;
	if (in.L == 0) {
		out.X = out.Y = out.Z = 0.0;
		return;
	}
	const double* RW = XYZ_GetReferenceWhite(in.stdObserver, in.illuminant);
	out.Y = (in.L+16.0)/116.0;
	out.Z = pow(out.Y, 3.0);						// Промежуточный результат
	if (out.Z > 0.008856)	out.Y = out.Z;
	else					out.Y = (out.Y-(16.0/116.0))/7.787;
	out.Z = RW[0]+(15.0*RW[1])+(3.0*RW[2]);			// Промежуточный результат
	out.X = (in.u/(13.0*in.L))+((4.0*RW[0])/out.Z); // Промежуточный результат
	out.Z = (in.v/(13.0*in.L))+((9.0*RW[1])/out.Z); // Промежуточный результат
	out.X = (-9.0*out.Y*out.X)/(((out.X-4.0)*out.Z)-(out.X*out.Z));
	out.Z = ((9.0*out.Y)-(15.0*out.Z*out.Y)-(out.Z*out.X))/(3.0*out.Z);
}

// ========= ========= ========= ========= ========= ========= ========= =========


XYZ_ILLUMINANT _LMS_GetIlluminant(const LMS_CONVERSION& c) {
	switch (c) {
	case LMS_CONVERSION::LMS2XYZ_E_2: return XYZ_ILLUMINANT::E;
	case LMS_CONVERSION::LMS2XYZ_F_2: return XYZ_ILLUMINANT::F1;	// Не уверен
	default:
		return XYZ_ILLUMINANT::D65;		// Неизвестно, но предположительно этот источник света 
	}
}

void CColorConverter::XYZD65toLMS(LMS& out, const XYZ& in, const LMS_CONVERSION c) {
	out.illuminant  = _LMS_GetIlluminant(c);
	if (in.illuminant != out.illuminant || in.stdObserver != XYZ_OBSERVER::CIE1931_2)
		return;
	out.stdObserver = XYZ_OBSERVER::CIE1931_2;
	mul3x3(LMS_Matrix[e_cast(c)], &in.X, &out.L);
}

void CColorConverter::LMStoXYZ(XYZ& out, const LMS& in, const LMS_CONVERSION c) {
	out.illuminant = _LMS_GetIlluminant(c);
	if (in.illuminant != out.illuminant)
		return;
	out.stdObserver = XYZ_OBSERVER::CIE1931_2;
	mul3x3(LMS_InvMatrix[e_cast(c)], &in.L, &out.X);
}

// ========= ========= ========= ========= ========= ========= ========= =========

void CColorConverter::YCoCg2RGB(SRGB& out, const YCOCG& in, BOOL bYCoCgR) {
	if (bYCoCgR) { // YCoCg-R вариант
		out.B  = in.Y-(in.Cg/2.0);
		out.G  = in.Cg+out.B;
		out.B -= in.Co/2.0;
		out.R  = out.B+in.Co;
	}
	else {
		out.B  = in.Y-in.Cg;
		out.R  = out.B+in.Co;
		out.G  = in.Y+in.Cg;
		out.B -= in.Co;
	}
}

void CColorConverter::RGB2YCoCg(YCOCG& out, const SRGB& in, BOOL bYCoCgR) {
	if (bYCoCgR) {
		out.Co = in.R-in.B; 
		out.Y  = in.B+(out.Co/2.0);
		out.Cg = in.G-out.Y;
		out.Y += out.Cg/2.0; 
	}
	else {
		out.Y  = (in.R+in.B)/4.0;
		out.Co = (in.G/2.0);
		out.Cg = out.Co-out.Y;
		out.Y += out.Co;
		out.Co = (in.R-in.B)/2.0;
	}
}

// ========= ========= ========= ========= ========= ========= ========= =========

void CColorConverter::GrayScale2RGB(SRGB& out, const double lightness) {
	out.R = out.G = out.B = lightness;
}

void CColorConverter::RGBA2RGB(SRGB& out, const RGBA& in) {
	out.R = ((1.0-in.A)*m_clrBackground.R)+(in.A*in.R);
	out.G = ((1.0-in.A)*m_clrBackground.G)+(in.A*in.G);
	out.B = ((1.0-in.A)*m_clrBackground.B)+(in.A*in.B);
}

COLORREF CColorConverter::RGBA2RGB(const RGBA& in) {
	return  (static_cast<BYTE>(round((((1.0-in.A)*m_clrBackground.R)+(in.A*in.R))*255))		 |
		    (static_cast<WORD>(round((((1.0-in.A)*m_clrBackground.G)+(in.A*in.G))*255))  << 8) | 
		    (static_cast<DWORD>(round((((1.0-in.A)*m_clrBackground.B)+(in.A*in.B))*255)) << 16));
}

void CColorConverter::HSLA2HSL(HSL& out, const HSLA& in) {
	SRGB rgb;
	RGBA rgba;
	HSL2RGB(rgba, in);
	rgba.A = in.A;
	RGBA2RGB(rgb, rgba);
	RGB2HSL(out, rgb);
}