#pragma once

// Note: proudly borrowing from : https://forums.developer.nvidia.com/t/an-accurate-single-precision-implementation-of-the-lambert-w-function/221292

namespace cuda_hidden
{
/*
  Copyright (c) 2022-2023, Norbert Juffa
  All rights reserved.

  Redistribution and use in source and binary forms, with or without 
  modification, are permitted provided that the following conditions
  are met:

  1. Redistributions of source code must retain the above copyright 
     notice, this list of conditions and the following disclaimer.

  2. Redistributions in binary form must reproduce the above copyright
     notice, this list of conditions and the following disclaimer in the
     documentation and/or other materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

__forceinline__ __device__ float raw_rcp (float a)
{
    float r;
    asm ("rcp.approx.ftz.f32 %0,%1;" : "=f"(r) : "f"(a));
    return r;
}

__forceinline__ __device__ float raw_sqrt (float a)
{
    float r;
    asm ("sqrt.approx.ftz.f32 %0,%1;" : "=f"(r) : "f"(a));
    return r;
}

__forceinline__ __device__ float raw_ex2 (float a)
{
    float r;
    asm ("ex2.approx.ftz.f32 %0,%1;" : "=f"(r) : "f"(a));
    return r;
}

__forceinline__ __device__ float raw_lg2 (float a)
{
    float r;
    asm ("lg2.approx.ftz.f32 %0,%1;" : "=f"(r) : "f"(a));
    return r;
}

/* Compute exp(a) * 2**scale. Max ulp err = 0.86565 */
__forceinline__ __device__ float expf_scale (float a, int scale)
{
    const float flt_int_cvt = 12582912.0f; // 0x1.8p23
    float f, r, j, t;
    int i;

    // exp(a) = 2**i * exp(f); i = rintf (a / log(2))
    j = fmaf (1.442695f, a, flt_int_cvt); // // 0x1.715476p0 // log2(e)
    t = j - flt_int_cvt; 
    f = fmaf (t, -6.93145752e-1f, a); // -0x1.62e400p-1  // log_2_hi 
    f = fmaf (t, -1.42860677e-6f, f); // -0x1.7f7d1cp-20 // log_2_lo 
    i = __float_as_int (j);
    // approximate r = exp(f) on interval [-log(2)/2, +log(2)/2]
    r =             1.37805939e-3f;  // 0x1.694000p-10
    r = fmaf (r, f, 8.37312452e-3f); // 0x1.125edcp-7
    r = fmaf (r, f, 4.16695364e-2f); // 0x1.555b5ap-5
    r = fmaf (r, f, 1.66664720e-1f); // 0x1.555450p-3
    r = fmaf (r, f, 4.99999851e-1f); // 0x1.fffff6p-2
    r = fmaf (r, f, 1.00000000e+0f); // 0x1.000000p+0
    r = fmaf (r, f, 1.00000000e+0f); // 0x1.000000p+0
    // exp(a) = 2**(i+scale) * r;
    r = __int_as_float (((i + scale) << 23) + __float_as_int (r));
    return r;
}

/*
  Compute the principal branch of the Lambert W function, W_0. The maximum
  error in the positive half-plane is 1.49923 ulps and the maximum error in
  the negative half-plane is 2.56303 ulps; a total of 76403840 results are 
  not correctly rounded.
*/
__device__ float lambert_w0f (float z) 
{
    const float MY_INF = __int_as_float (0x7f800000);
    const float em1_fact_0 = 0.625529587f; // exp(-1)_factor_0
    const float em1_fact_1 = 0.588108778f; // exp(-1)_factor_1
    const float qe1 = 2.71828183f / 4.0f; // exp(1)/4
    float e, w, num, den, rden, redz, y, r;

    if (isnan (z) || (z == MY_INF) || (z == 0.0f)) return z + z;
    if (fabsf (z) < 1.220703125e-4f) return fmaf (-z, z, z); // 0x1.0p-13
    redz = fmaf (em1_fact_0, em1_fact_1, z); // z + exp(-1)
    if (redz < 0.0625f) { // expansion at -(exp(-1))
        r = raw_sqrt (redz);
        w =             -1.23046875f;  // -0x1.3b0000p+0
        w = fmaf (w, r,  2.17185670f); //  0x1.15ff66p+1
        w = fmaf (w, r, -2.19554094f); // -0x1.19077cp+1 
        w = fmaf (w, r,  1.92107077f); //  0x1.ebcb4cp+0
        w = fmaf (w, r, -1.81141856f); // -0x1.cfb920p+0
        w = fmaf (w, r,  2.33162979f); //  0x1.2a72d8p+1
        w = fmaf (w, r, -1.00000000f); // -0x1.000000p+0
    } else {
        /* Compute initial approximation. Based on: Roberto Iacono and John 
           Philip Boyd, "New approximations to the principal real-valued branch
           of the Lambert W function", Advances in Computational Mathematics, 
           Vol. 43, No. 6, December 2017, pp. 1403-1436
        */
        y = fmaf (2.0f, raw_sqrt (fmaf (qe1, z, 0.25f)), 1.0f);
        y = raw_lg2 (raw_rcp (fmaf (0.31819974f, raw_lg2 (y), 1.0f)) * 
                     fmaf (1.15262585f, y, -0.15262585f));
        w = fmaf (1.41337042f, y, -1.0f);

        /* perform Newton iterations to refine approximation to full accuracy */
        const float l2e = 1.44269504f; // log2(exp(1))

        e = raw_ex2 (fmaf (l2e, w, -3.0f));
        num = fmaf (w, e, -0.125f * z);
        den = fmaf (w, e, e);
        rden = raw_rcp (den);
        w = fmaf (-num, rden, w);

        e = raw_ex2 (fmaf (l2e, w, -3.0f));
        num = fmaf (w, e, -0.125f * z);
        den = fmaf (w, e, e);
        rden = raw_rcp (den);
        w = fmaf (-num, rden, w);

        e = expf_scale (w, -3);
        num = fmaf (w, e, -0.125f * z);
        den = fmaf (w, e, e);
        rden = raw_rcp (den);
        w = fmaf (-num, rden, w);
    }
    return w;
}
} // namespace cuda_hidden
