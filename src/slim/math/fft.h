#pragma once

#include "./comp.h"

INLINE_XPU void initFFTws(comp *w, u8 power_of_2) {
//    generateOrbit(1 << power_of_2, w, false, true);
    u32 N = 1 << power_of_2;
    u32 N_over_2 = N / 2;
    f64 one_over_N = 1.0 / (f64)N;
    f64 angle_step = TAU * one_over_N;
    f64 angle = 0;
    for (u32 i = 0; i < N_over_2; i++, angle += angle_step) {
        w[i] = {(f32)(cos(angle)), -(f32)(sin(angle))};
        w[i + N_over_2] = w[i].conjugate();
    }
}

INLINE_XPU u32 bitReversed(u32 number, u8 bit_count) {
    u32 reverse_number = 0;
    for (u32 i = 0, bit = 1; i < bit_count; i++, bit <<= 1)
        if ((number & bit))
            reverse_number |= 1 << ((bit_count - 1) - i);

    return reverse_number;
}

INLINE_XPU void FFT(comp *signal, comp *w, u8 power_of_2, comp *waves) {
    u32 N = 1 << power_of_2;
    u32 N_over_2 = N >> 1;
    for (u32 i = 0; i < N; i++) waves[i] = signal[bitReversed(i, power_of_2)];
    for (u32 step = 1, jump = 2, w_step = N_over_2; step < N; step <<= 1, jump <<= 1, w_step >>= 1) {
        for (u32 butterfly = 0, w_index = 0; butterfly < step; butterfly++, w_index += w_step) {
            for (u32 top_index = butterfly; top_index < N; top_index += jump) {
                comp &top{waves[top_index]};
                comp &bottom{waves[top_index + step]};
                comp bottom_rotated{bottom * w[w_index]};
                bottom = top - bottom_rotated;
                top += bottom_rotated;
            }
        }
    }
}

INLINE_XPU void IFFT(comp *signal, comp *w, u8 power_of_2, comp *waves) {
    FFT(waves, w + (1 << (power_of_2 - 1)), power_of_2, signal);
}

bool compareFFT(comp *output, comp *expected) {
    for (u8 i = 0; i < 8; i++)
        if (abs(output[i].real - expected[i].real) > 0.01f ||
            abs(output[i].imag - expected[i].imag) > 0.01f)
            return false;

    return true;
}
/*
bool testFFT() {
    comp ws[4], output[8], data[8], expected[8];
    initFFTws(ws, 3);



    float data1_re[8] = {0.0, 1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0};
    float data1_im[8] = {7.0, 6.0, 5.0, 4.0, 3.0, 2.0, 1.0, 0.0};
    float expected1_re[8] = {28.0, 5.656, 0.0, -2.343, -4.0, -5.656, -8.0, -13.656};
    float expected1_im[8] = {28.0, 13.656, 8.0, 5.656, 4.0, 2.343, 0.0, -5.656};
    for (u8 i = 0; i < 8; i++) {
        data[i] = {data1_re[i], data1_im[i]};
        expected[i] = {expected1_re[i], expected1_im[i]};
    }
    FFT(data, ws, 3, output);
    if (!compareFFT(output, expected))
        return false;


    float data2_re[8] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    float data2_im[8] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
    float expected2_re[8] = {8.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    float expected2_im[8] = {8.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    for (u8 i = 0; i < 8; i++) {
        data[i] = {data2_re[i], data2_im[i]};
        expected[i] = {expected2_re[i], expected2_im[i]};
    }
    FFT(data, ws, 3, output);
    if (!compareFFT(output, expected))
        return false;

    float data3_re[8] = { 1.0, -1.0,  1.0, -1.0,  1.0, -1.0,  1.0, -1.0};
    float data3_im[8] = {-1.0,  1.0, -1.0,  1.0, -1.0,  1.0, -1.0,  1.0};
    float expected3_re[8] = {0.0, 0.0, 0.0, 0.0,  8.0, 0.0, 0.0, 0.0};
    float expected3_im[8] = {0.0, 0.0, 0.0, 0.0, -8.0, 0.0, 0.0, 0.0};
    for (u8 i = 0; i < 8; i++) {
        data[i] = {data3_re[i], data3_im[i]};
        expected[i] = {expected3_re[i], expected3_im[i]};
    }
    FFT(data, ws, 3, output);
    if (!compareFFT(output, expected))
        return false;


    float data4_re[4] = {1.0, 2.0, 3.0, 4.0};
    float data4_im[4] = {0.0, 0.0, 0.0, 0.0};
    float expected4_re[4] = {10.0, -2.0, -2.0, -2.0};
    float expected4_im[4] = {0.0, 2.0, 0.0, -2.0};
    for (u8 i = 0; i < 4; i++) {
        data[i] = {data4_re[i], data4_im[i]};
        expected[i] = {expected4_re[i], expected4_im[i]};
    }
    initFFTws(ws, 2);
    FFT(data, ws, 2, output);
    if (!compareFFT(output, expected))
        return false;

    return true;
}*/