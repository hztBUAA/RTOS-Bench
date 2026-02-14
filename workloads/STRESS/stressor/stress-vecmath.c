/* applications/stress-ng/stress-vecmath.c */
#include "stress-ng.h"
#include "stress_osal.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <config.h>

#if defined(__SIZEOF_INT128__)
#define HAVE_INT128_T
#endif

typedef int8_t  stress_vint8_t  __attribute__ ((vector_size (16)));
typedef int16_t stress_vint16_t __attribute__ ((vector_size (16)));
typedef int32_t stress_vint32_t __attribute__ ((vector_size (16)));
typedef int64_t stress_vint64_t __attribute__ ((vector_size (16)));
#if defined(HAVE_INT128_T)
typedef __uint128_t stress_vint128_t __attribute__ ((vector_size (16)));
#endif

#define H8(a0)                      \
    ((int8_t)((uint8_t)a0))
#define H16(a0, a1)                     \
    ((int16_t)(((uint16_t)a0 << 8) |        \
           ((uint16_t)a1 << 0)))
#define H32(a0, a1, a2, a3)             \
    ((int32_t)(((uint32_t)a0 << 24) |       \
           ((uint32_t)a1 << 16) |       \
           ((uint32_t)a2 <<  8) |       \
           ((uint32_t)a3 <<  0)))
#define H64(a0, a1, a2, a3, a4, a5, a6, a7)     \
    ((int64_t)(((uint64_t)a0 << 56) |       \
           ((uint64_t)a1 << 48) |       \
           ((uint64_t)a2 << 40) |       \
           ((uint64_t)a3 << 32) |       \
           ((uint64_t)a4 << 24) |       \
           ((uint64_t)a5 << 16) |       \
           ((uint64_t)a6 <<  8) |       \
           ((uint64_t)a7 <<  0)))

#if defined(HAVE_INT128_T)
#define H128(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af)    \
    ((__int128_t)(((__int128_t)a0 << 120) |     \
             ((__int128_t)a1 << 112) |      \
             ((__int128_t)a2 << 104) |      \
             ((__int128_t)a3 <<  96) |      \
             ((__int128_t)a4 <<  88) |      \
             ((__int128_t)a5 <<  80) |      \
             ((__int128_t)a6 <<  72) |      \
             ((__int128_t)a7 <<  64) |      \
             ((__int128_t)a8 <<  56) |      \
             ((__int128_t)a9 <<  48) |      \
             ((__int128_t)aa <<  40) |      \
             ((__int128_t)ab <<  32) |      \
             ((__int128_t)ac <<  24) |      \
             ((__int128_t)ad <<  16) |      \
             ((__int128_t)ae <<   8) |      \
             ((__int128_t)af <<   0)))      \

#endif

#define A(M)    M(0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,   \
          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00)

#define B(M)    M(0x01, 0x23, 0x45, 0x67, 0x89, 0xab, 0xcd, 0xef,   \
          0x0f, 0x1e, 0x2d, 0x3c, 0x4b, 0x5a, 0x69, 0x78)

#define C(M)    M(0x01, 0x02, 0x03, 0x02, 0x01, 0x02, 0x03, 0x02,   \
          0x03, 0x02, 0x01, 0x02, 0x03, 0x02, 0x01, 0x02)

#define S(M)    M(0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02,   \
          0x01, 0x01, 0x02, 0x02, 0x01, 0x01, 0x02, 0x02)

#define V23(M)  M(0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17,   \
          0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17, 0x17)

#define V3(M)   M(0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,   \
          0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03)

#define INT16x8(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af) \
    H8(a0), H8(a1), H8(a2), H8(a3), H8(a4), H8(a5), H8(a6), H8(a7),     \
    H8(a8), H8(a9), H8(aa), H8(ab), H8(ac), H8(ad), H8(ae), H8(af)

#define INT8x16(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af) \
    H16(a0, a1), H16(a2, a3), H16(a4, a5), H16(a6, a7),                     \
    H16(a8, a9), H16(aa, ab), H16(ac, ad), H16(ae, af)

#define INT4x32(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af) \
    H32(a0, a1, a2, a3), H32(a4, a5, a6, a7),               \
    H32(a8, a9, aa, ab), H32(ac, ad, ae, af)

#define INT2x64(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af) \
    H64(a0, a1, a2, a3, a4, a5, a6, a7),                    \
    H64(a8, a9, aa, ab, ac, ad, ae, af)

#if defined(HAVE_INT128_T)
#define INT1x128(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af)\
    H128(a0, a1, a2, a3, a4, a5, a6, a7, a8, a9, aa, ab, ac, ad, ae, af)
#endif

#define OPS(a, b, c, s, v23, v3) \
do {                \
    a += b;         \
    a |= b;         \
    a -= b;         \
    a &= ~b;        \
    a *= c;         \
    a = ~a;         \
    a *= s;         \
    a ^= c;         \
    a <<= 1;        \
    b >>= 1;        \
    b += c;         \
    a %= v23;       \
    c /= v3;        \
    b = b ^ c;      \
    c = b ^ c;      \
    b = b ^ c;      \
} while (0)

static int32_t s_vecmath_loops = DEFAULT_VECMATH_LOOPS;

static int stress_vecmath_opt_loops(const char *opt_name, const char *opt_arg)
{
    int val = atoi(opt_arg);
    if (val < 1) val = 1;

    s_vecmath_loops = val;
    stress_osal_print("rtos_stress: debug: vecmath-loops set to %d\n", s_vecmath_loops);
    return 0;
}

const stress_opt_t stress_vecmath_opts[] = {
    { "vecmath-loops", stress_vecmath_opt_loops },
    { NULL, NULL }
};

void stress_vecmath(stress_args_t *args)
{
    int rc = EXIT_SUCCESS;

    const stress_vint8_t v23_8 = { V23(INT16x8) };
    const stress_vint8_t v3_8 = { V3(INT16x8) };
    const stress_vint16_t v23_16 = { V23(INT8x16) };
    const stress_vint16_t v3_16 = { V3(INT8x16) };
    const stress_vint32_t v23_32 = { V23(INT4x32) };
    const stress_vint32_t v3_32 = { V3(INT4x32) };
    const stress_vint64_t v23_64 = { V23(INT2x64) };
    const stress_vint64_t v3_64 = { V3(INT2x64) };
#if defined(HAVE_INT128_T)
    const stress_vint128_t v23_128 = { V23(INT1x128) };
    const stress_vint128_t v3_128 = { V3(INT1x128) };
#endif

    if (!g_stress_silent_mode) {
        stress_osal_print("rtos_stress: info: [vecmath-%d] starting vector math stressor (loops=%d, checksum=disabled)\n",
                          args->instance, s_vecmath_loops);
    }

    do {
        int i;

        stress_vint8_t a8 = { A(INT16x8) };
        stress_vint8_t b8 = { B(INT16x8) };
        stress_vint8_t c8 = { C(INT16x8) };
        stress_vint8_t s8 = { S(INT16x8) };

        stress_vint16_t a16 = { A(INT8x16) };
        stress_vint16_t b16 = { B(INT8x16) };
        stress_vint16_t c16 = { C(INT8x16) };
        stress_vint16_t s16 = { S(INT8x16) };

        stress_vint32_t a32 = { A(INT4x32) };
        stress_vint32_t b32 = { B(INT4x32) };
        stress_vint32_t c32 = { C(INT4x32) };
        stress_vint32_t s32 = { S(INT4x32) };

        stress_vint64_t a64 = { A(INT2x64) };
        stress_vint64_t b64 = { B(INT2x64) };
        stress_vint64_t c64 = { C(INT2x64) };
        stress_vint64_t s64 = { S(INT2x64) };

#if defined(HAVE_INT128_T)
        stress_vint128_t a128 = { A(INT1x128) };
        stress_vint128_t b128 = { B(INT1x128) };
        stress_vint128_t c128 = { C(INT1x128) };
        stress_vint128_t s128 = { S(INT1x128) };
#endif
        for (i = s_vecmath_loops; i; i--) {
            OPS(a8, b8, c8, s8, v23_8, v3_8);
            OPS(a16, b16, c16, s16, v23_16, v3_16);
            OPS(a32, b32, c32, s32, v23_32, v3_32);
            OPS(a64, b64, c64, s64, v23_64, v3_64);
#if defined(HAVE_INT128_T)
            OPS(a128, b128, c128, s128, v23_128, v3_128);
#endif
            OPS(a32, b32, c32, s32, v23_32, v3_32);
            OPS(a16, b16, c16, s16, v23_16, v3_16);
#if defined(HAVE_INT128_T)
            OPS(a128, b128, c128, s128, v23_128, v3_128);
#endif
            OPS(a8, b8, c8, s8, v23_8, v3_8);
            OPS(a64, b64, c64, s64, v23_64, v3_64);

            OPS(a8, b8, c8, s8, v23_8, v3_8);
            OPS(a8, b8, c8, s8, v23_8, v3_8);
            OPS(a8, b8, c8, s8, v23_8, v3_8);
            OPS(a8, b8, c8, s8, v23_8, v3_8);

            OPS(a16, b16, c16, s16, v23_16, v3_16);
            OPS(a16, b16, c16, s16, v23_16, v3_16);
            OPS(a16, b16, c16, s16, v23_16, v3_16);
            OPS(a16, b16, c16, s16, v23_16, v3_16);

            OPS(a32, b32, c32, s32, v23_32, v3_32);
            OPS(a32, b32, c32, s32, v23_32, v3_32);
            OPS(a32, b32, c32, s32, v23_32, v3_32);
            OPS(a32, b32, c32, s32, v23_32, v3_32);

            OPS(a64, b64, c64, s64, v23_64, v3_64);
            OPS(a64, b64, c64, s64, v23_64, v3_64);
            OPS(a64, b64, c64, s64, v23_64, v3_64);
            OPS(a64, b64, c64, s64, v23_64, v3_64);
#if defined(HAVE_INT128_T)
            OPS(a128, b128, c128, s128, v23_128, v3_128);
            OPS(a128, b128, c128, s128, v23_128, v3_128);
            OPS(a128, b128, c128, s128, v23_128, v3_128);
            OPS(a128, b128, c128, s128, v23_128, v3_128);
#endif
        }

        args->bogo.current_ops++;

        if (rc != EXIT_SUCCESS) break;
        stress_osal_sleep_ms(1);

    } while (stress_continue(args));
}
