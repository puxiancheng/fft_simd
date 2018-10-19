// SIMD.cpp: 定义控制台应用程序的入口点。
// Target: Be familiar with the SIMD command.

#include "stdafx.h"
#include <iostream>
#include <vector>
//#include <intrin.h>
#include <emmintrin.h>	// sse2
#include <math.h>
#include <fstream>
#include <vector>
#include <string>
#include <complex>
#include <time.h>
using namespace std;
#define PI 3.141592653589793238462643383279502884
int standard_dft( vector<double>dft_in, vector<complex<double>> &dft_out );
int fft(vector<double> &fft_in, vector<complex<double>> &fft_out);
int time_consum(vector<double>&data);	// time consum
int main()
{
	/*----------------------------------------------------------------
	 *	SIMD test.
	 * -Task 1. Implement standard DFT algorithm with C++.
	 * -Task 2. Rewrite the DFT with Cooley–Tukey FFT algorithm.
	 * Task 3. Ues SIMD to accelerate the FFT alogrithm.
	 * Task 4. Make a time consuming contrast among these algorithms.
	----------------------------------------------------------------*/
	//const double  PI = acos(-1.0);

	// read the testing data.
	ifstream fi("fft_1024.txt");
	vector<double> data;
	string read_temp;
	while (fi.good())
	{
		getline(fi, read_temp);
		data.push_back( stod(read_temp));
	}

	//	time_consum(data); // 测试时间消耗

	//----------------------Basied SIMD testing---------------------------//
	//double temp[] = { data[0], data[5] };	// 取出4点 double 进行 SIMD 测试
	//const double *p = (double *)&data;
	const double p[10] = {0,1,2,3,4,5,6,7,8,9};

	__m128d temp_sum = _mm_setzero_pd();	// 初始化一个全为0的寄存器 两个 double
	for (size_t n = 0; n != 5;++n) {
		__m128d to_sum = _mm_load_pd(p + n*2);		// 从 p 中加载两个 double,p 需要偏移

		temp_sum = _mm_add_pd(temp_sum, to_sum);
	}
	double * output = new double[2];
	_mm_store_pd(output, temp_sum);
	cout << output[0]+output[1] << endl;
	//----------------------Output the result----------------------------//

	//ofstream fo("fft_out_ct.txt", ios::ate);
	//for (auto per_data : result) {
	//	fo << per_data.real() <<" + "<< per_data.imag()<<"j"<< "\n";
	//}

	fi.close();
	//fo.close();
	

    return 0;
}

int standard_dft(vector<double> dft_in, vector<complex<double>> &dft_out)
{
	// Standard fft algorithm without any optimization.
	// Ref:https://en.wikipedia.org/wiki/DFT

	int N = dft_in.size();
	for (size_t k = 0; k != N; ++k) {
		//complex<double> x_n(0, 0);
		double re = 0;												// the real part
		double im = 0;												// the imag part
		for (size_t n = 0; n != N; ++n) {
			re += dft_in[n] * cos(-double(k * 2 * PI * n / N));		// Euler's formula： exp( i*x ) = cos_x + i*sin_x
			im += dft_in[n] * sin(-double(k * 2 * PI * n / N));
			// x_n += fft_in[n] * exp( (double)k *-1i * 2.0*PI* (double)( n )/double( N) );	
			// Be careful for the integer division  in C++
		}

		dft_out.push_back( complex<double>(re, im) );
	}
	return 0;
}

int fft(vector<double> &fft_in, vector<complex<double>>& fft_out)
{
	// fft_in : 输入的需要进行 FFT 处理的数据
	// fft_out : 经过 FFT 处理的数据
	// 注意： 输入和输出的长度保持一致，not in-place algorithm
	int N = fft_in.size();	 //	N must be a power - of - 2
	if (N >=2) {
		// 分成两部分
		vector<double> odd;		// 奇数部分
		vector<double> even;	// 偶数部分
		
		for (size_t n = 0; n != N ; ++n) {
			if ( (n+1) % 2 == 0) {
				even.push_back(fft_in.at( n ));
			}
			else {
				odd.push_back(fft_in.at( n ));
			}
		}

		// 递归处理
		vector<complex<double>> fft_even_out(fft_out.begin(),fft_out.begin()+N/2);	// low 
		vector<complex<double>> fft_odd_out(fft_out.begin() + N / 2,fft_out.end());	// high

		fft(even, fft_even_out);// 取一半
		fft(odd, fft_odd_out);

		// 合并
		complex<double> odd_out;
		complex<double> even_out;

		for (size_t n = 0; n != N / 2; ++n) {
			//even_out = fft_even_out[n];
			//odd_out = fft_odd_out[n];						// 注意是否越界 
			even_out = (N == 2) ? even[n] + fft_even_out[n ] : fft_even_out[n ];
			odd_out  = (N == 2) ? odd[n] + fft_odd_out[n ]: fft_odd_out[n ];		
			complex<double> w = exp( complex<double>( 0, -2.0 * PI *double(n) / (double)(N) ) );
			fft_out[n] = even_out + w * odd_out;			// even part
			fft_out[n + N / 2] = even_out - w * odd_out;	// odd part

		}
	}
	else
		return 0;

	return 0;
}
int time_consum(vector<double>&data)
{
	vector<complex<double>> result(1024);
	size_t counter = 10;
	clock_t dft_start = clock();
	for (size_t n = 0; n != counter; ++n) {
		standard_dft(data, result); // Task 1.
	}
	clock_t dft_end = clock();
	for (size_t n = 0; n != counter; ++n) {
		fft(data, result);	// Task 2.
	}
	clock_t fft_end = clock();
	cout << "Standard dft cost : " << (dft_end - dft_start) / 10 << " ms.\n";
	cout << "      FFT cost    : " << (fft_end - dft_end) / 10 << " ms.\n";
	return 0;
}
int fft_simd(vector<double> &fft_in, vector<complex<double>>& fft_out)
{
	// fft_in : 输入的需要进行 FFT 处理的数据
	// fft_out : 经过 FFT 处理的数据
	// 注意： 输入和输出的长度保持一致，not in-place algorithm
	int N = fft_in.size();		//	N must be a power - of - 2
	if (N >= 2) {
		// 分成两部分
		vector<double> odd;		// 奇数部分
		vector<double> even;	// 偶数部分

		for (size_t n = 0; n != N; ++n) {
			if ((n + 1) % 2 == 0) {
				even.push_back(fft_in.at(n));
			}
			else {
				odd.push_back(fft_in.at(n));
			}
		}

		// 递归处理
		vector<complex<double>> fft_even_out(fft_out.begin(), fft_out.begin() + N / 2);	// low 
		vector<complex<double>> fft_odd_out(fft_out.begin() + N / 2, fft_out.end());	// high

		fft(even, fft_even_out);// 取一半
		fft(odd, fft_odd_out);

		// 合并
		complex<double> odd_out;
		complex<double> even_out;

		for (size_t n = 0; n != N / 2; ++n) {
			//even_out = fft_even_out[n];
			//odd_out = fft_odd_out[n];						// 注意是否越界 
			even_out = (N == 2) ? even[n] + fft_even_out[n] : fft_even_out[n];
			odd_out = (N == 2) ? odd[n] + fft_odd_out[n] : fft_odd_out[n];
			complex<double> w = exp(complex<double>(0, -2.0 * PI *double(n) / (double)(N)));
			fft_out[n] = even_out + w * odd_out;			// even part
			fft_out[n + N / 2] = even_out - w * odd_out;	// odd part

		}
	}
	else
		return 0;

	return 0;
}