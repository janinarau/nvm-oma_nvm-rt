#include "math.h"
#include <arm_neon.h>

typedef float32x4_t v4sf;  // vector of 4 float
typedef uint32x4_t v4su;   // vector of 4 uint32
typedef int32x4_t v4si;    // vector of 4 uint32

v4sf exp_ps(v4sf x);
v4sf sin_ps(v4sf x);
v4sf cos_ps(v4sf x);

double Math::exp(double num) {
    float number = num;
    float32x4_t inp = vdupq_n_f32(number);
    inp = exp_ps(inp);
    return vgetq_lane_f32(inp, 0);
}

double Math::sqrt(double num) {
    float64x1_t number = vdup_n_f64(num);
    float64x1_t result = vsqrt_f64(number);
    return vget_lane_f64(result, 0);
}

double Math::sin(double num) {
    float number = num;
    float32x4_t inp = vdupq_n_f32(number);
    inp = sin_ps(inp);
    return vgetq_lane_f32(inp, 0);
}
double Math::cos(double num) {
    float number = num;
    float32x4_t inp = vdupq_n_f32(number);
    inp = cos_ps(inp);
    return vgetq_lane_f32(inp, 0);
}
double Math::pi() { return 3.14159265359; }

#define c_exp_hi 88.3762626647949f
#define c_exp_lo -88.3762626647949f

#define c_cephes_LOG2EF 1.44269504088896341
#define c_cephes_exp_C1 0.693359375
#define c_cephes_exp_C2 -2.12194440e-4

#define c_cephes_exp_p0 1.9875691500E-4
#define c_cephes_exp_p1 1.3981999507E-3
#define c_cephes_exp_p2 8.3334519073E-3
#define c_cephes_exp_p3 4.1665795894E-2
#define c_cephes_exp_p4 1.6666665459E-1
#define c_cephes_exp_p5 5.0000001201E-1

/* exp() computed for 4 float at once */
v4sf exp_ps(v4sf x) {
    v4sf tmp, fx;

    v4sf one = vdupq_n_f32(1);
    x = vminq_f32(x, vdupq_n_f32(c_exp_hi));
    x = vmaxq_f32(x, vdupq_n_f32(c_exp_lo));

    /* express exp(x) as exp(g + n*log(2)) */
    fx = vmlaq_f32(vdupq_n_f32(0.5f), x, vdupq_n_f32(c_cephes_LOG2EF));

    /* perform a floorf */
    tmp = vcvtq_f32_s32(vcvtq_s32_f32(fx));

    /* if greater, substract 1 */
    v4su mask = vcgtq_f32(tmp, fx);
    mask = vandq_u32(mask, vreinterpretq_u32_f32(one));

    fx = vsubq_f32(tmp, vreinterpretq_f32_u32(mask));

    tmp = vmulq_f32(fx, vdupq_n_f32(c_cephes_exp_C1));
    v4sf z = vmulq_f32(fx, vdupq_n_f32(c_cephes_exp_C2));
    x = vsubq_f32(x, tmp);
    x = vsubq_f32(x, z);

    static const float cephes_exp_p[6] = {c_cephes_exp_p0, c_cephes_exp_p1,
                                          c_cephes_exp_p2, c_cephes_exp_p3,
                                          c_cephes_exp_p4, c_cephes_exp_p5};
    v4sf y = vld1q_dup_f32(cephes_exp_p + 0);
    v4sf c1 = vld1q_dup_f32(cephes_exp_p + 1);
    v4sf c2 = vld1q_dup_f32(cephes_exp_p + 2);
    v4sf c3 = vld1q_dup_f32(cephes_exp_p + 3);
    v4sf c4 = vld1q_dup_f32(cephes_exp_p + 4);
    v4sf c5 = vld1q_dup_f32(cephes_exp_p + 5);

    y = vmulq_f32(y, x);
    z = vmulq_f32(x, x);
    y = vaddq_f32(y, c1);
    y = vmulq_f32(y, x);
    y = vaddq_f32(y, c2);
    y = vmulq_f32(y, x);
    y = vaddq_f32(y, c3);
    y = vmulq_f32(y, x);
    y = vaddq_f32(y, c4);
    y = vmulq_f32(y, x);
    y = vaddq_f32(y, c5);

    y = vmulq_f32(y, z);
    y = vaddq_f32(y, x);
    y = vaddq_f32(y, one);

    /* build 2^n */
    int32x4_t mm;
    mm = vcvtq_s32_f32(fx);
    mm = vaddq_s32(mm, vdupq_n_s32(0x7f));
    mm = vshlq_n_s32(mm, 23);
    v4sf pow2n = vreinterpretq_f32_s32(mm);

    y = vmulq_f32(y, pow2n);
    return y;
}

#define c_minus_cephes_DP1 -0.78515625
#define c_minus_cephes_DP2 -2.4187564849853515625e-4
#define c_minus_cephes_DP3 -3.77489497744594108e-8
#define c_sincof_p0 -1.9515295891E-4
#define c_sincof_p1 8.3321608736E-3
#define c_sincof_p2 -1.6666654611E-1
#define c_coscof_p0 2.443315711809948E-005
#define c_coscof_p1 -1.388731625493765E-003
#define c_coscof_p2 4.166664568298827E-002
#define c_cephes_FOPI 1.27323954473516  // 4 / M_PI

/* evaluation of 4 sines & cosines at once.

   The code is the exact rewriting of the cephes sinf function.
   Precision is excellent as long as x < 8192 (I did not bother to
   take into account the special handling they have for greater values
   -- it does not return garbage for arguments over 8192, though, but
   the extra precision is missing).

   Note that it is such that sinf((float)M_PI) = 8.74e-8, which is the
   surprising but correct result.

   Note also that when you compute sin(x), cos(x) is available at
   almost no extra price so both sin_ps and cos_ps make use of
   sincos_ps..
  */
void sincos_ps(v4sf x, v4sf *ysin, v4sf *ycos) {  // any x
    v4sf xmm1, xmm2, xmm3, y;

    v4su emm2;

    v4su sign_mask_sin, sign_mask_cos;
    sign_mask_sin = vcltq_f32(x, vdupq_n_f32(0));
    x = vabsq_f32(x);

    /* scale by 4/Pi */
    y = vmulq_f32(x, vdupq_n_f32(c_cephes_FOPI));

    /* store the integer part of y in mm0 */
    emm2 = vcvtq_u32_f32(y);
    /* j=(j+1) & (~1) (see the cephes sources) */
    emm2 = vaddq_u32(emm2, vdupq_n_u32(1));
    emm2 = vandq_u32(emm2, vdupq_n_u32(~1));
    y = vcvtq_f32_u32(emm2);

    /* get the polynom selection mask
       there is one polynom for 0 <= x <= Pi/4
       and another one for Pi/4<x<=Pi/2

       Both branches will be computed.
    */
    v4su poly_mask = vtstq_u32(emm2, vdupq_n_u32(2));

    /* The magic pass: "Extended precision modular arithmetic"
       x = ((x - y * DP1) - y * DP2) - y * DP3; */
    xmm1 = vmulq_n_f32(y, c_minus_cephes_DP1);
    xmm2 = vmulq_n_f32(y, c_minus_cephes_DP2);
    xmm3 = vmulq_n_f32(y, c_minus_cephes_DP3);
    x = vaddq_f32(x, xmm1);
    x = vaddq_f32(x, xmm2);
    x = vaddq_f32(x, xmm3);

    sign_mask_sin = veorq_u32(sign_mask_sin, vtstq_u32(emm2, vdupq_n_u32(4)));
    sign_mask_cos = vtstq_u32(vsubq_u32(emm2, vdupq_n_u32(2)), vdupq_n_u32(4));

    /* Evaluate the first polynom  (0 <= x <= Pi/4) in y1,
       and the second polynom      (Pi/4 <= x <= 0) in y2 */
    v4sf z = vmulq_f32(x, x);
    v4sf y1, y2;

    y1 = vmulq_n_f32(z, c_coscof_p0);
    y2 = vmulq_n_f32(z, c_sincof_p0);
    y1 = vaddq_f32(y1, vdupq_n_f32(c_coscof_p1));
    y2 = vaddq_f32(y2, vdupq_n_f32(c_sincof_p1));
    y1 = vmulq_f32(y1, z);
    y2 = vmulq_f32(y2, z);
    y1 = vaddq_f32(y1, vdupq_n_f32(c_coscof_p2));
    y2 = vaddq_f32(y2, vdupq_n_f32(c_sincof_p2));
    y1 = vmulq_f32(y1, z);
    y2 = vmulq_f32(y2, z);
    y1 = vmulq_f32(y1, z);
    y2 = vmulq_f32(y2, x);
    y1 = vsubq_f32(y1, vmulq_f32(z, vdupq_n_f32(0.5f)));
    y2 = vaddq_f32(y2, x);
    y1 = vaddq_f32(y1, vdupq_n_f32(1));

    /* select the correct result from the two polynoms */
    v4sf ys = vbslq_f32(poly_mask, y1, y2);
    v4sf yc = vbslq_f32(poly_mask, y2, y1);
    *ysin = vbslq_f32(sign_mask_sin, vnegq_f32(ys), ys);
    *ycos = vbslq_f32(sign_mask_cos, yc, vnegq_f32(yc));
}

v4sf sin_ps(v4sf x) {
    v4sf ysin, ycos;
    sincos_ps(x, &ysin, &ycos);
    return ysin;
}

v4sf cos_ps(v4sf x) {
    v4sf ysin, ycos;
    sincos_ps(x, &ysin, &ycos);
    return ycos;
}

// Float versions
float Math::exp_s(float num) {
    float number = num;
    float32x4_t inp = vdupq_n_f32(number);
    inp = exp_ps(inp);
    return vgetq_lane_f32(inp, 0);
}

float Math::sqrt_s(float num) {
    float32x2_t number = vdup_n_f32(num);
    float32x2_t result = vsqrt_f32(number);
    return vget_lane_f32(result, 0);
}

float Math::sin_s(float num) {
    float number = num;
    float32x4_t inp = vdupq_n_f32(number);
    inp = sin_ps(inp);
    return vgetq_lane_f32(inp, 0);
}
float Math::cos_s(float num) {
    float number = num;
    float32x4_t inp = vdupq_n_f32(number);
    inp = cos_ps(inp);
    return vgetq_lane_f32(inp, 0);
}
float Math::pi_s() { return 3.14159265359; }