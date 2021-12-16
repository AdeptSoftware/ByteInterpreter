#pragma once

template <typename T1, typename T2>
void mul3x3(const T1 a, const T2 b, double out[3][3]) {
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 3; j++) {
			out[i][j] = 0;
			for (int k = 0; k < 3; k++)
				out[i][j] += (a[i][k] * b[k][j]);
		}
	}
}

template <typename T>
void mul3x3(const T m, const double* vec, double out[3]) {
	out[0] = (m[0][0]*vec[0])+(m[0][1]*vec[1])+(m[0][2]*vec[2]);
	out[1] = (m[1][0]*vec[0])+(m[1][1]*vec[1])+(m[1][2]*vec[2]);
	out[2] = (m[2][0]*vec[0])+(m[2][1]*vec[1])+(m[2][2]*vec[2]);
}

template <typename T>
void inv3x3(const T m, double out[3][3]) {
	double det = 1.0 / (+m[0][0] * (m[1][1] * m[2][2] - m[2][1] * m[1][2])
						-m[1][0] * (m[0][1] * m[2][2] - m[2][1] * m[0][2])
						+m[2][0] * (m[0][1] * m[1][2] - m[1][1] * m[0][2]));

	out[0][0] = +(m[1][1] * m[2][2] - m[2][1] * m[1][2]) * det;
	out[1][0] = -(m[1][0] * m[2][2] - m[2][0] * m[1][2]) * det;
	out[2][0] = +(m[1][0] * m[2][1] - m[2][0] * m[1][1]) * det;
	out[0][1] = -(m[0][1] * m[2][2] - m[2][1] * m[0][2]) * det;
	out[1][1] = +(m[0][0] * m[2][2] - m[2][0] * m[0][2]) * det;
	out[2][1] = -(m[0][0] * m[2][1] - m[2][0] * m[0][1]) * det;
	out[0][2] = +(m[0][1] * m[1][2] - m[1][1] * m[0][2]) * det;
	out[1][2] = -(m[0][0] * m[1][2] - m[1][0] * m[0][2]) * det;
	out[2][2] = +(m[0][0] * m[1][1] - m[1][0] * m[0][1]) * det;
}