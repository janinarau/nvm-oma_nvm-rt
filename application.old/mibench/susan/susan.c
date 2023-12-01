
typedef unsigned long TOTAL_TYPE; /* this is faster for "int" but should be
                           "float" for large d masks */

/*#define FOPENB*/ /* uncomment if using djgpp gnu C for DOS or certain Win95
                      compilers */
#define SEVEN_SUPP /* size for non-max corner suppression; SEVEN_SUPP or \
                      FIVE_SUPP */
#define MAX_CORNERS 15000 /* max corners per frame */

/* ********** Leave the rest - but you may need to remove one or both of
 * sys/file.h and malloc.h lines */

#include <system/service/logger.h>
#include "meminterface.h"

#define exit_error(IFB, IFC)   \
    {                          \
        log_error(IFB << IFC); \
        while (1)              \
            ;                  \
    }

#define FTOI(a) \
    ((a) < 0 ? ((unsigned long)(a - 0.5)) : ((unsigned long)(a + 0.5)))

#define EOF -1

typedef unsigned char uchar;

typedef struct {
    unsigned long x, y, info, dx, dy, I;
} CORNER_LIST[MAX_CORNERS];

// exponential approximation
float exp(float x) {
    x = 1.0 + x / 1024;
    x *= x;
    x *= x;
    x *= x;
    x *= x;
    x *= x;
    x *= x;
    x *= x;
    x *= x;
    x *= x;
    x *= x;
    return x;
}

// sqrt approximation
float sqrt(unsigned long val) {
    unsigned long a, b;

    if (val < 2) return val; /* avoid div/0 */

    a = 1255; /* starting point is relatively unimportant */

    b = val / a;
    a = (a + b) / 2;
    b = val / a;
    a = (a + b) / 2;
    b = val / a;
    a = (a + b) / 2;
    b = val / a;
    a = (a + b) / 2;

    return a;
}

unsigned long abs(long number) {
    if (number < 0) {
        return number * -1;
    }
    return number;
}

int getint(char *&input_stream) {
    long c, i;
    char dummy[10000];

    c = *(input_stream++);
    while (1) /* find next integer */
    {
        if (c == '#') /* if we're at a comment, read to end of line */
        {
            char *d_ptr = dummy;
            while (c != '\n' && c != EOF) {
                *(d_ptr++) = c;
                c = *(input_stream++);
            }
            if (c == '\n')
                ;
            c = *(input_stream++);
        }
        if (c == EOF) exit_error("Image %s not binary PGM.\n", "is");
        if (c >= '0' && c <= '9') break; /* found what we were looking for */
        c = *(input_stream++);
    }

    /* we're at the start of a number, continue until we hit a non-number */
    i = 0;
    while (1) {
        i = (i * 10) + (c - '0');
        c = *(input_stream++);
        if (c == EOF) return (i);
        if (c < '0' || c > '9') break;
    }

    return (i);
}

void get_image(char *input_stream, unsigned char **in, unsigned long *x_size,
               unsigned long *y_size) {
    char header[100];

    header[0] = *(input_stream++);
    header[1] = *(input_stream++);
    if (!(header[0] == 'P' && header[1] == '5'))
        exit_error("Image %s does not have binary PGM header.\n", "");

    *x_size = getint(input_stream);
    *y_size = getint(input_stream);
    getint(input_stream);

    *in = (unsigned char *)input_stream;
}

void int_to_uchar(unsigned long *r, unsigned char *in, unsigned long size) {
    unsigned long i, max_r = r[0], min_r = r[0];

    for (i = 0; i < size; i++) {
        if (r[i] > max_r) max_r = r[i];
        if (r[i] < min_r) min_r = r[i];
    }

    max_r -= min_r;

    for (i = 0; i < size; i++)
        in[i] = (unsigned char)((int)((int)(r[i] - min_r) * 255) / max_r);
}

void setup_brightness_lut(unsigned char **bp, unsigned long thresh,
                          unsigned long form) {
    unsigned long k;
    float __attribute((aligned(8))) temp;

    *bp = (unsigned char *)malloc(516);
    *bp = *bp + 258;

    for (k = -256; k < 257; k++) {
        temp = ((float)k) / ((float)thresh);
        temp = temp * temp;
        if (form == 6) temp = temp * temp * temp;
        temp = 100.0 * exp(-temp);
        *(*bp + k) = (uchar)temp;
    }
}

void susan_principle(unsigned char *in, unsigned long *r, unsigned char *bp,
                     unsigned long max_no, unsigned long x_size,
                     unsigned long y_size) {
    unsigned long i, j, n;
    unsigned char *p, *cp;

    memset(r, 0, x_size * y_size * sizeof(unsigned long));

    for (i = 3; i < y_size - 3; i++)
        for (j = 3; j < x_size - 3; j++) {
            n = 100;
            p = in + (i - 3) * x_size + j - 1;
            cp = bp + in[i * x_size + j];

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 3;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 5;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 6;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += 2;
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 6;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 5;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 3;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);

            if (n <= max_no) r[i * x_size + j] = max_no - n;
        }
}

void susan_principle_small(unsigned char *in, unsigned long *r,
                           unsigned char *bp, unsigned long max_no,
                           unsigned long x_size, unsigned long y_size) {
    unsigned long i, j, n;
    uchar *p, *cp;

    memset(r, 0, x_size * y_size * sizeof(unsigned long));

    max_no = 730; /* ho hum ;) */

    for (i = 1; i < y_size - 1; i++)
        for (j = 1; j < x_size - 1; j++) {
            n = 100;
            p = in + (i - 1) * x_size + j - 1;
            cp = bp + in[i * x_size + j];

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 2;

            n += *(cp - *p);
            p += 2;
            n += *(cp - *p);
            p += x_size - 2;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);

            if (n <= max_no) r[i * x_size + j] = max_no - n;
        }
}

unsigned char median(unsigned char *in, unsigned long i, unsigned long j,
                     unsigned long x_size) {
    unsigned long p[8], k, l, tmp;

    p[0] = in[(i - 1) * x_size + j - 1];
    p[1] = in[(i - 1) * x_size + j];
    p[2] = in[(i - 1) * x_size + j + 1];
    p[3] = in[(i)*x_size + j - 1];
    p[4] = in[(i)*x_size + j + 1];
    p[5] = in[(i + 1) * x_size + j - 1];
    p[6] = in[(i + 1) * x_size + j];
    p[7] = in[(i + 1) * x_size + j + 1];

    for (k = 0; k < 7; k++)
        for (l = 0; l < (7 - k); l++)
            if (p[l] > p[l + 1]) {
                tmp = p[l];
                p[l] = p[l + 1];
                p[l + 1] = tmp;
            }

    return ((p[3] + p[4]) / 2);
}

void enlarge(unsigned char **in, unsigned char *tmp_image,
             unsigned long *x_size, unsigned long *y_size,
             unsigned long border) {
    unsigned long i, j;

    for (i = 0; i < *y_size; i++) /* copy *in into tmp_image */
        memcpy(tmp_image + (i + border) * (*x_size + 2 * border) + border,
               *in + i * *x_size, *x_size);

    for (i = 0; i < border;
         i++) /* copy top and bottom rows; invert as many as necessary */
    {
        memcpy(tmp_image + (border - 1 - i) * (*x_size + 2 * border) + border,
               *in + i * *x_size, *x_size);
        memcpy(tmp_image + (*y_size + border + i) * (*x_size + 2 * border) +
                   border,
               *in + (*y_size - i - 1) * *x_size, *x_size);
    }

    for (i = 0; i < border; i++) /* copy left and right columns */
        for (j = 0; j < *y_size + 2 * border; j++) {
            tmp_image[j * (*x_size + 2 * border) + border - 1 - i] =
                tmp_image[j * (*x_size + 2 * border) + border + i];
            tmp_image[j * (*x_size + 2 * border) + *x_size + border + i] =
                tmp_image[j * (*x_size + 2 * border) + *x_size + border - 1 -
                          i];
        }

    *x_size += 2 * border; /* alter image size */
    *y_size += 2 * border;
    *in = tmp_image; /* repoint in */
}

void susan_smoothing(unsigned long three_by_three, unsigned char *in, float dt,
                     unsigned long x_size, unsigned long y_size,
                     unsigned char *bp) {
    float __attribute((aligned(8))) temp;
    unsigned long n_max, increment, mask_size, i, j, x, y, area, brightness,
        tmp, centre;
    uchar *ip, *dp, *dpt, *cp, *out = in, *tmp_image;
    TOTAL_TYPE total;

    if (three_by_three == 0)
        mask_size = ((unsigned long)(1.5 * dt)) + 1;
    else
        mask_size = 1;

    total = 0.1; /* test for total's type */
    if ((dt > 15) && (total == 0)) {
        log_error("Distance_thresh (" << dt
                                      << ") too big for integer arithmetic.\n");
        while (1)
            ;
    }

    if ((2 * mask_size + 1 > x_size) || (2 * mask_size + 1 > y_size)) {
        log_error("Mask size (1.5*distance_thresh+1="
                  << mask_size << ") too big for image (" << x_size << "x"
                  << y_size << ").\n");
        while (1)
            ;
    }

    tmp_image = (unsigned char *)malloc((x_size + mask_size * 2) *
                                        (y_size + mask_size * 2));
    enlarge(&in, tmp_image, &x_size, &y_size, mask_size);

    if (three_by_three == 0) {
        n_max = (mask_size * 2) + 1;

        increment = x_size - n_max;

        dp = (unsigned char *)malloc(n_max * n_max);
        dpt = dp;
        temp = -(dt * dt);

        for (i = -mask_size; i <= mask_size; i++)
            for (j = -mask_size; j <= mask_size; j++) {
                x = (unsigned long)(100.0 *
                                    exp(((float)((i * i) + (j * j))) / temp));
                *dpt++ = (unsigned char)x;
            }

        for (i = mask_size; i < y_size - mask_size; i++) {
            for (j = mask_size; j < x_size - mask_size; j++) {
                area = 0;
                total = 0;
                dpt = dp;
                ip = in + ((i - mask_size) * x_size) + j - mask_size;
                centre = in[i * x_size + j];
                cp = bp + centre;
                for (y = -mask_size; y <= mask_size; y++) {
                    for (x = -mask_size; x <= mask_size; x++) {
                        brightness = *ip++;
                        tmp = *dpt++ * *(cp - brightness);
                        area += tmp;
                        total += tmp * brightness;
                    }
                    ip += increment;
                }
                tmp = area - 10000;
                if (tmp == 0)
                    *out++ = median(in, i, j, x_size);
                else
                    *out++ = ((total - (centre * 10000)) / tmp);
            }
        }

    } else {
        for (i = 1; i < y_size - 1; i++) {
            for (j = 1; j < x_size - 1; j++) {
                area = 0;
                total = 0;
                ip = in + ((i - 1) * x_size) + j - 1;
                centre = in[i * x_size + j];
                cp = bp + centre;

                brightness = *ip++;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;
                brightness = *ip++;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;
                brightness = *ip;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;
                ip += x_size - 2;
                brightness = *ip++;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;
                brightness = *ip++;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;
                brightness = *ip;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;
                ip += x_size - 2;
                brightness = *ip++;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;
                brightness = *ip++;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;
                brightness = *ip;
                tmp = *(cp - brightness);
                area += tmp;
                total += tmp * brightness;

                tmp = area - 100;
                if (tmp == 0)
                    *out++ = median(in, i, j, x_size);
                else
                    *out++ = (total - (centre * 100)) / tmp;
            }
        }
    }
}

void edge_draw(unsigned char *in, unsigned char *mid, unsigned long x_size,
               unsigned long y_size, unsigned long drawing_mode) {
    unsigned long i;
    unsigned char *inp, *midp;

    if (drawing_mode == 0) {
        /* mark 3x3 white block around each edge point */
        midp = mid;
        for (i = 0; i < x_size * y_size; i++) {
            if (*midp < 8) {
                inp = in + (midp - mid) - x_size - 1;
                *inp++ = 255;
                *inp++ = 255;
                *inp = 255;
                inp += x_size - 2;
                *inp++ = 255;
                inp++;
                *inp = 255;
                inp += x_size - 2;
                *inp++ = 255;
                *inp++ = 255;
                *inp = 255;
            }
            midp++;
        }
    }

    /* now mark 1 black pixel at each edge point */
    midp = mid;
    for (i = 0; i < x_size * y_size; i++) {
        if (*midp < 8) *(in + (midp - mid)) = 0;
        midp++;
    }
}

/* only one pass is needed as i,j are decremented if necessary to go
   back and do bits again */

void susan_thin(unsigned long *r, unsigned char *mid, unsigned long x_size,
                unsigned long y_size) {
    unsigned long l[9], centre, b01, b12, b21, b10, p1, p2, p3, p4, b00, b02,
        b20, b22, m, n, a, b, x, y, i, j;
    unsigned char *mp;

    for (i = 4; i < y_size - 4; i++)
        for (j = 4; j < x_size - 4; j++)
            if (mid[i * x_size + j] < 8) {
                centre = r[i * x_size + j];
                /* {{{ count number of neighbours */

                mp = mid + (i - 1) * x_size + j - 1;

                n = (*mp < 8) + (*(mp + 1) < 8) + (*(mp + 2) < 8) +
                    (*(mp + x_size) < 8) + (*(mp + x_size + 2) < 8) +
                    (*(mp + x_size + x_size) < 8) +
                    (*(mp + x_size + x_size + 1) < 8) +
                    (*(mp + x_size + x_size + 2) < 8);

                /* }}} */
                /* {{{ n==0 no neighbours - remove point */

                if (n == 0) mid[i * x_size + j] = 100;

                /* }}} */
                /* {{{ n==1 - extend line if I can */

                /* extension is only allowed a few times - the value of mid is
                 * used to control this */

                if ((n == 1) && (mid[i * x_size + j] < 6)) {
                    /* find maximum neighbour weighted in direction opposite the
                       neighbour already present. e.g.
                       have: O O O  weight r by 0 2 3
                             X X O              0 0 4
                             O O O              0 2 3     */

                    l[0] = r[(i - 1) * x_size + j - 1];
                    l[1] = r[(i - 1) * x_size + j];
                    l[2] = r[(i - 1) * x_size + j + 1];
                    l[3] = r[(i)*x_size + j - 1];
                    l[4] = 0;
                    l[5] = r[(i)*x_size + j + 1];
                    l[6] = r[(i + 1) * x_size + j - 1];
                    l[7] = r[(i + 1) * x_size + j];
                    l[8] = r[(i + 1) * x_size + j + 1];

                    if (mid[(i - 1) * x_size + j - 1] < 8) {
                        l[0] = 0;
                        l[1] = 0;
                        l[3] = 0;
                        l[2] *= 2;
                        l[6] *= 2;
                        l[5] *= 3;
                        l[7] *= 3;
                        l[8] *= 4;
                    } else {
                        if (mid[(i - 1) * x_size + j] < 8) {
                            l[1] = 0;
                            l[0] = 0;
                            l[2] = 0;
                            l[3] *= 2;
                            l[5] *= 2;
                            l[6] *= 3;
                            l[8] *= 3;
                            l[7] *= 4;
                        } else {
                            if (mid[(i - 1) * x_size + j + 1] < 8) {
                                l[2] = 0;
                                l[1] = 0;
                                l[5] = 0;
                                l[0] *= 2;
                                l[8] *= 2;
                                l[3] *= 3;
                                l[7] *= 3;
                                l[6] *= 4;
                            } else {
                                if (mid[(i)*x_size + j - 1] < 8) {
                                    l[3] = 0;
                                    l[0] = 0;
                                    l[6] = 0;
                                    l[1] *= 2;
                                    l[7] *= 2;
                                    l[2] *= 3;
                                    l[8] *= 3;
                                    l[5] *= 4;
                                } else {
                                    if (mid[(i)*x_size + j + 1] < 8) {
                                        l[5] = 0;
                                        l[2] = 0;
                                        l[8] = 0;
                                        l[1] *= 2;
                                        l[7] *= 2;
                                        l[0] *= 3;
                                        l[6] *= 3;
                                        l[3] *= 4;
                                    } else {
                                        if (mid[(i + 1) * x_size + j - 1] < 8) {
                                            l[6] = 0;
                                            l[3] = 0;
                                            l[7] = 0;
                                            l[0] *= 2;
                                            l[8] *= 2;
                                            l[1] *= 3;
                                            l[5] *= 3;
                                            l[2] *= 4;
                                        } else {
                                            if (mid[(i + 1) * x_size + j] < 8) {
                                                l[7] = 0;
                                                l[6] = 0;
                                                l[8] = 0;
                                                l[3] *= 2;
                                                l[5] *= 2;
                                                l[0] *= 3;
                                                l[2] *= 3;
                                                l[1] *= 4;
                                            } else {
                                                if (mid[(i + 1) * x_size + j +
                                                        1] < 8) {
                                                    l[8] = 0;
                                                    l[5] = 0;
                                                    l[7] = 0;
                                                    l[6] *= 2;
                                                    l[2] *= 2;
                                                    l[1] *= 3;
                                                    l[3] *= 3;
                                                    l[0] *= 4;
                                                }
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }

                    m = 0; /* find the highest point */
                    for (y = 0; y < 3; y++)
                        for (x = 0; x < 3; x++)
                            if (l[y + y + y + x] > m) {
                                m = l[y + y + y + x];
                                a = y;
                                b = x;
                            }

                    if (m > 0) {
                        if (mid[i * x_size + j] < 4)
                            mid[(i + a - 1) * x_size + j + b - 1] = 4;
                        else
                            mid[(i + a - 1) * x_size + j + b - 1] =
                                mid[i * x_size + j] + 1;
                        if ((a + a + b) < 3) /* need to jump back in image */
                        {
                            i += a - 1;
                            j += b - 2;
                            if (i < 4) i = 4;
                            if (j < 4) j = 4;
                        }
                    }
                }

                /* }}} */
                /* {{{ n==2 */

                if (n == 2) {
                    /* put in a bit here to straighten edges */
                    b00 =
                        mid[(i - 1) * x_size + j - 1] < 8; /* corners of 3x3 */
                    b02 = mid[(i - 1) * x_size + j + 1] < 8;
                    b20 = mid[(i + 1) * x_size + j - 1] < 8;
                    b22 = mid[(i + 1) * x_size + j + 1] < 8;
                    if (((b00 + b02 + b20 + b22) == 2) &&
                        ((b00 | b22) &
                         (b02 | b20))) { /* case: move a point back into line.
                                            e.g. X O X  CAN  become X X X
                                                 O X O              O O O
                                                 O O O              O O O    */
                        if (b00) {
                            if (b02) {
                                x = 0;
                                y = -1;
                            } else {
                                x = -1;
                                y = 0;
                            }
                        } else {
                            if (b02) {
                                x = 1;
                                y = 0;
                            } else {
                                x = 0;
                                y = 1;
                            }
                        }
                        if (((float)r[(i + y) * x_size + j + x] /
                             (float)centre) > 0.7) {
                            if (((x == 0) &&
                                 (mid[(i + (2 * y)) * x_size + j] > 7) &&
                                 (mid[(i + (2 * y)) * x_size + j - 1] > 7) &&
                                 (mid[(i + (2 * y)) * x_size + j + 1] > 7)) ||
                                ((y == 0) &&
                                 (mid[(i)*x_size + j + (2 * x)] > 7) &&
                                 (mid[(i + 1) * x_size + j + (2 * x)] > 7) &&
                                 (mid[(i - 1) * x_size + j + (2 * x)] > 7))) {
                                mid[(i)*x_size + j] = 100;
                                mid[(i + y) * x_size + j + x] =
                                    3; /* no jumping needed */
                            }
                        }
                    } else {
                        b01 = mid[(i - 1) * x_size + j] < 8;
                        b12 = mid[(i)*x_size + j + 1] < 8;
                        b21 = mid[(i + 1) * x_size + j] < 8;
                        b10 = mid[(i)*x_size + j - 1] < 8;
                        /* {{{ right angle ends - not currently used */

#ifdef IGNORETHIS
                        if ((b00 & b01) | (b00 & b10) | (b02 & b01) |
                            (b02 & b12) | (b20 & b10) | (b20 & b21) |
                            (b22 & b21) |
                            (b22 &
                             b12)) { /* case; right angle ends. clean up.
                                        e.g.; X X O  CAN  become X X O
                                              O X O              O O O
                                              O O O              O O O        */
                            if (((b01) & (mid[(i - 2) * x_size + j - 1] > 7) &
                                 (mid[(i - 2) * x_size + j] > 7) &
                                 (mid[(i - 2) * x_size + j + 1] > 7) &
                                 ((b00 & ((2 * r[(i - 1) * x_size + j + 1]) >
                                          centre)) |
                                  (b02 & ((2 * r[(i - 1) * x_size + j - 1]) >
                                          centre)))) |
                                ((b10) & (mid[(i - 1) * x_size + j - 2] > 7) &
                                 (mid[(i)*x_size + j - 2] > 7) &
                                 (mid[(i + 1) * x_size + j - 2] > 7) &
                                 ((b00 & ((2 * r[(i + 1) * x_size + j - 1]) >
                                          centre)) |
                                  (b20 & ((2 * r[(i - 1) * x_size + j - 1]) >
                                          centre)))) |
                                ((b12) & (mid[(i - 1) * x_size + j + 2] > 7) &
                                 (mid[(i)*x_size + j + 2] > 7) &
                                 (mid[(i + 1) * x_size + j + 2] > 7) &
                                 ((b02 & ((2 * r[(i + 1) * x_size + j + 1]) >
                                          centre)) |
                                  (b22 & ((2 * r[(i - 1) * x_size + j + 1]) >
                                          centre)))) |
                                ((b21) & (mid[(i + 2) * x_size + j - 1] > 7) &
                                 (mid[(i + 2) * x_size + j] > 7) &
                                 (mid[(i + 2) * x_size + j + 1] > 7) &
                                 ((b20 & ((2 * r[(i + 1) * x_size + j + 1]) >
                                          centre)) |
                                  (b22 & ((2 * r[(i + 1) * x_size + j - 1]) >
                                          centre))))) {
                                mid[(i)*x_size + j] = 100;
                                if (b10 & b20) j -= 2;
                                if (b00 | b01 | b02) {
                                    i--;
                                    j -= 2;
                                }
                            }
                        }
#endif

                        /* }}} */
                        if (((b01 + b12 + b21 + b10) == 2) &&
                            ((b10 | b12) & (b01 | b21)) &&
                            ((b01 & ((mid[(i - 2) * x_size + j - 1] < 8) |
                                     (mid[(i - 2) * x_size + j + 1] < 8))) |
                             (b10 & ((mid[(i - 1) * x_size + j - 2] < 8) |
                                     (mid[(i + 1) * x_size + j - 2] < 8))) |
                             (b12 & ((mid[(i - 1) * x_size + j + 2] < 8) |
                                     (mid[(i + 1) * x_size + j + 2] < 8))) |
                             (b21 &
                              ((mid[(i + 2) * x_size + j - 1] < 8) |
                               (mid[(i + 2) * x_size + j + 1] <
                                8))))) { /* case; clears odd right angles.
                                            e.g.; O O O  becomes O O O
                                                  X X O          X O O
                                                  O X O          O X O     */
                            mid[(i)*x_size + j] = 100;
                            i--; /* jump back */
                            j -= 2;
                            if (i < 4) i = 4;
                            if (j < 4) j = 4;
                        }
                    }
                }

                /* }}} */
                /* {{{ n>2 the thinning is done here without breaking
                 * connectivity */

                if (n > 2) {
                    b01 = mid[(i - 1) * x_size + j] < 8;
                    b12 = mid[(i)*x_size + j + 1] < 8;
                    b21 = mid[(i + 1) * x_size + j] < 8;
                    b10 = mid[(i)*x_size + j - 1] < 8;
                    if ((b01 + b12 + b21 + b10) > 1) {
                        b00 = mid[(i - 1) * x_size + j - 1] < 8;
                        b02 = mid[(i - 1) * x_size + j + 1] < 8;
                        b20 = mid[(i + 1) * x_size + j - 1] < 8;
                        b22 = mid[(i + 1) * x_size + j + 1] < 8;
                        p1 = b00 | b01;
                        p2 = b02 | b12;
                        p3 = b22 | b21;
                        p4 = b20 | b10;

                        if (((p1 + p2 + p3 + p4) - ((b01 & p2) + (b12 & p3) +
                                                    (b21 & p4) + (b10 & p1))) <
                            2) {
                            mid[(i)*x_size + j] = 100;
                            i--;
                            j -= 2;
                            if (i < 4) i = 4;
                            if (j < 4) j = 4;
                        }
                    }
                }

                /* }}} */
            }
}

void susan_edges(unsigned char *in, unsigned long *r, unsigned char *mid,
                 unsigned char *bp, unsigned long max_no, unsigned long x_size,
                 unsigned long y_size) {
    float __attribute((aligned(8))) z;
    unsigned long do_symmetry, i, j, m, n, a, b, x, y, w;
    unsigned char c, *p, *cp;

    memset(r, 0, x_size * y_size * sizeof(unsigned long));

    for (i = 3; i < y_size - 3; i++)
        for (j = 3; j < x_size - 3; j++) {
            n = 100;
            p = in + (i - 3) * x_size + j - 1;
            cp = bp + in[i * x_size + j];

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 3;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 5;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 6;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += 2;
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 6;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 5;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 3;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);

            if (n <= max_no) r[i * x_size + j] = max_no - n;
        }

    for (i = 4; i < y_size - 4; i++)
        for (j = 4; j < x_size - 4; j++) {
            if (r[i * x_size + j] > 0) {
                m = r[i * x_size + j];
                n = max_no - m;
                cp = bp + in[i * x_size + j];

                if (n > 600) {
                    p = in + (i - 3) * x_size + j - 1;
                    x = 0;
                    y = 0;

                    c = *(cp - *p++);
                    x -= c;
                    y -= 3 * c;
                    c = *(cp - *p++);
                    y -= 3 * c;
                    c = *(cp - *p);
                    x += c;
                    y -= 3 * c;
                    p += x_size - 3;

                    c = *(cp - *p++);
                    x -= 2 * c;
                    y -= 2 * c;
                    c = *(cp - *p++);
                    x -= c;
                    y -= 2 * c;
                    c = *(cp - *p++);
                    y -= 2 * c;
                    c = *(cp - *p++);
                    x += c;
                    y -= 2 * c;
                    c = *(cp - *p);
                    x += 2 * c;
                    y -= 2 * c;
                    p += x_size - 5;

                    c = *(cp - *p++);
                    x -= 3 * c;
                    y -= c;
                    c = *(cp - *p++);
                    x -= 2 * c;
                    y -= c;
                    c = *(cp - *p++);
                    x -= c;
                    y -= c;
                    c = *(cp - *p++);
                    y -= c;
                    c = *(cp - *p++);
                    x += c;
                    y -= c;
                    c = *(cp - *p++);
                    x += 2 * c;
                    y -= c;
                    c = *(cp - *p);
                    x += 3 * c;
                    y -= c;
                    p += x_size - 6;

                    c = *(cp - *p++);
                    x -= 3 * c;
                    c = *(cp - *p++);
                    x -= 2 * c;
                    c = *(cp - *p);
                    x -= c;
                    p += 2;
                    c = *(cp - *p++);
                    x += c;
                    c = *(cp - *p++);
                    x += 2 * c;
                    c = *(cp - *p);
                    x += 3 * c;
                    p += x_size - 6;

                    c = *(cp - *p++);
                    x -= 3 * c;
                    y += c;
                    c = *(cp - *p++);
                    x -= 2 * c;
                    y += c;
                    c = *(cp - *p++);
                    x -= c;
                    y += c;
                    c = *(cp - *p++);
                    y += c;
                    c = *(cp - *p++);
                    x += c;
                    y += c;
                    c = *(cp - *p++);
                    x += 2 * c;
                    y += c;
                    c = *(cp - *p);
                    x += 3 * c;
                    y += c;
                    p += x_size - 5;

                    c = *(cp - *p++);
                    x -= 2 * c;
                    y += 2 * c;
                    c = *(cp - *p++);
                    x -= c;
                    y += 2 * c;
                    c = *(cp - *p++);
                    y += 2 * c;
                    c = *(cp - *p++);
                    x += c;
                    y += 2 * c;
                    c = *(cp - *p);
                    x += 2 * c;
                    y += 2 * c;
                    p += x_size - 3;

                    c = *(cp - *p++);
                    x -= c;
                    y += 3 * c;
                    c = *(cp - *p++);
                    y += 3 * c;
                    c = *(cp - *p);
                    x += c;
                    y += 3 * c;

                    z = sqrt((float)((x * x) + (y * y)));
                    if (z > (0.9 * (float)n)) /* 0.5 */
                    {
                        do_symmetry = 0;
                        if (x == 0)
                            z = 1000000.0;
                        else
                            z = ((float)y) / ((float)x);
                        if (z < 0) {
                            z = -z;
                            w = -1;
                        } else
                            w = 1;
                        if (z < 0.5) { /* vert_edge */
                            a = 0;
                            b = 1;
                        } else {
                            if (z > 2.0) { /* hor_edge */
                                a = 1;
                                b = 0;
                            } else { /* diag_edge */
                                if (w > 0) {
                                    a = 1;
                                    b = 1;
                                } else {
                                    a = -1;
                                    b = 1;
                                }
                            }
                        }
                        if ((m > r[(i + a) * x_size + j + b]) &&
                            (m >= r[(i - a) * x_size + j - b]) &&
                            (m > r[(i + (2 * a)) * x_size + j + (2 * b)]) &&
                            (m >= r[(i - (2 * a)) * x_size + j - (2 * b)]))
                            mid[i * x_size + j] = 1;
                    } else
                        do_symmetry = 1;
                } else
                    do_symmetry = 1;

                if (do_symmetry == 1) {
                    p = in + (i - 3) * x_size + j - 1;
                    x = 0;
                    y = 0;
                    w = 0;

                    /*   |      \
                         y  -x-  w
                         |        \   */

                    c = *(cp - *p++);
                    x += c;
                    y += 9 * c;
                    w += 3 * c;
                    c = *(cp - *p++);
                    y += 9 * c;
                    c = *(cp - *p);
                    x += c;
                    y += 9 * c;
                    w -= 3 * c;
                    p += x_size - 3;

                    c = *(cp - *p++);
                    x += 4 * c;
                    y += 4 * c;
                    w += 4 * c;
                    c = *(cp - *p++);
                    x += c;
                    y += 4 * c;
                    w += 2 * c;
                    c = *(cp - *p++);
                    y += 4 * c;
                    c = *(cp - *p++);
                    x += c;
                    y += 4 * c;
                    w -= 2 * c;
                    c = *(cp - *p);
                    x += 4 * c;
                    y += 4 * c;
                    w -= 4 * c;
                    p += x_size - 5;

                    c = *(cp - *p++);
                    x += 9 * c;
                    y += c;
                    w += 3 * c;
                    c = *(cp - *p++);
                    x += 4 * c;
                    y += c;
                    w += 2 * c;
                    c = *(cp - *p++);
                    x += c;
                    y += c;
                    w += c;
                    c = *(cp - *p++);
                    y += c;
                    c = *(cp - *p++);
                    x += c;
                    y += c;
                    w -= c;
                    c = *(cp - *p++);
                    x += 4 * c;
                    y += c;
                    w -= 2 * c;
                    c = *(cp - *p);
                    x += 9 * c;
                    y += c;
                    w -= 3 * c;
                    p += x_size - 6;

                    c = *(cp - *p++);
                    x += 9 * c;
                    c = *(cp - *p++);
                    x += 4 * c;
                    c = *(cp - *p);
                    x += c;
                    p += 2;
                    c = *(cp - *p++);
                    x += c;
                    c = *(cp - *p++);
                    x += 4 * c;
                    c = *(cp - *p);
                    x += 9 * c;
                    p += x_size - 6;

                    c = *(cp - *p++);
                    x += 9 * c;
                    y += c;
                    w -= 3 * c;
                    c = *(cp - *p++);
                    x += 4 * c;
                    y += c;
                    w -= 2 * c;
                    c = *(cp - *p++);
                    x += c;
                    y += c;
                    w -= c;
                    c = *(cp - *p++);
                    y += c;
                    c = *(cp - *p++);
                    x += c;
                    y += c;
                    w += c;
                    c = *(cp - *p++);
                    x += 4 * c;
                    y += c;
                    w += 2 * c;
                    c = *(cp - *p);
                    x += 9 * c;
                    y += c;
                    w += 3 * c;
                    p += x_size - 5;

                    c = *(cp - *p++);
                    x += 4 * c;
                    y += 4 * c;
                    w -= 4 * c;
                    c = *(cp - *p++);
                    x += c;
                    y += 4 * c;
                    w -= 2 * c;
                    c = *(cp - *p++);
                    y += 4 * c;
                    c = *(cp - *p++);
                    x += c;
                    y += 4 * c;
                    w += 2 * c;
                    c = *(cp - *p);
                    x += 4 * c;
                    y += 4 * c;
                    w += 4 * c;
                    p += x_size - 3;

                    c = *(cp - *p++);
                    x += c;
                    y += 9 * c;
                    w -= 3 * c;
                    c = *(cp - *p++);
                    y += 9 * c;
                    c = *(cp - *p);
                    x += c;
                    y += 9 * c;
                    w += 3 * c;

                    if (y == 0)
                        z = 1000000.0;
                    else
                        z = ((float)x) / ((float)y);
                    if (z < 0.5) { /* vertical */
                        a = 0;
                        b = 1;
                    } else {
                        if (z > 2.0) { /* horizontal */
                            a = 1;
                            b = 0;
                        } else { /* diagonal */
                            if (w > 0) {
                                a = -1;
                                b = 1;
                            } else {
                                a = 1;
                                b = 1;
                            }
                        }
                    }
                    if ((m > r[(i + a) * x_size + j + b]) &&
                        (m >= r[(i - a) * x_size + j - b]) &&
                        (m > r[(i + (2 * a)) * x_size + j + (2 * b)]) &&
                        (m >= r[(i - (2 * a)) * x_size + j - (2 * b)]))
                        mid[i * x_size + j] = 2;
                }
            }
        }
}

/* }}} */
/* {{{ susan_edges_small(in,r,sf,max_no,out) */

void susan_edges_small(unsigned char *in, unsigned long *r, unsigned char *mid,
                       unsigned char *bp, unsigned long max_no,
                       unsigned long x_size, unsigned long y_size) {
    float __attribute((aligned(8))) z;
    unsigned long do_symmetry, i, j, m, n, a, b, x, y, w;
    unsigned char c, *p, *cp;

    memset(r, 0, x_size * y_size * sizeof(unsigned long));

    max_no = 730; /* ho hum ;) */

    for (i = 1; i < y_size - 1; i++)
        for (j = 1; j < x_size - 1; j++) {
            n = 100;
            p = in + (i - 1) * x_size + j - 1;
            cp = bp + in[i * x_size + j];

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);
            p += x_size - 2;

            n += *(cp - *p);
            p += 2;
            n += *(cp - *p);
            p += x_size - 2;

            n += *(cp - *p++);
            n += *(cp - *p++);
            n += *(cp - *p);

            if (n <= max_no) r[i * x_size + j] = max_no - n;
        }

    for (i = 2; i < y_size - 2; i++)
        for (j = 2; j < x_size - 2; j++) {
            if (r[i * x_size + j] > 0) {
                m = r[i * x_size + j];
                n = max_no - m;
                cp = bp + in[i * x_size + j];

                if (n > 250) {
                    p = in + (i - 1) * x_size + j - 1;
                    x = 0;
                    y = 0;

                    c = *(cp - *p++);
                    x -= c;
                    y -= c;
                    c = *(cp - *p++);
                    y -= c;
                    c = *(cp - *p);
                    x += c;
                    y -= c;
                    p += x_size - 2;

                    c = *(cp - *p);
                    x -= c;
                    p += 2;
                    c = *(cp - *p);
                    x += c;
                    p += x_size - 2;

                    c = *(cp - *p++);
                    x -= c;
                    y += c;
                    c = *(cp - *p++);
                    y += c;
                    c = *(cp - *p);
                    x += c;
                    y += c;

                    z = sqrt((float)((x * x) + (y * y)));
                    if (z > (0.4 * (float)n)) /* 0.6 */
                    {
                        do_symmetry = 0;
                        if (x == 0)
                            z = 1000000.0;
                        else
                            z = ((float)y) / ((float)x);
                        if (z < 0) {
                            z = -z;
                            w = -1;
                        } else
                            w = 1;
                        if (z < 0.5) { /* vert_edge */
                            a = 0;
                            b = 1;
                        } else {
                            if (z > 2.0) { /* hor_edge */
                                a = 1;
                                b = 0;
                            } else { /* diag_edge */
                                if (w > 0) {
                                    a = 1;
                                    b = 1;
                                } else {
                                    a = -1;
                                    b = 1;
                                }
                            }
                        }
                        if ((m > r[(i + a) * x_size + j + b]) &&
                            (m >= r[(i - a) * x_size + j - b]))
                            mid[i * x_size + j] = 1;
                    } else
                        do_symmetry = 1;
                } else
                    do_symmetry = 1;

                if (do_symmetry == 1) {
                    p = in + (i - 1) * x_size + j - 1;
                    x = 0;
                    y = 0;
                    w = 0;

                    /*   |      \
                         y  -x-  w
                         |        \   */

                    c = *(cp - *p++);
                    x += c;
                    y += c;
                    w += c;
                    c = *(cp - *p++);
                    y += c;
                    c = *(cp - *p);
                    x += c;
                    y += c;
                    w -= c;
                    p += x_size - 2;

                    c = *(cp - *p);
                    x += c;
                    p += 2;
                    c = *(cp - *p);
                    x += c;
                    p += x_size - 2;

                    c = *(cp - *p++);
                    x += c;
                    y += c;
                    w -= c;
                    c = *(cp - *p++);
                    y += c;
                    c = *(cp - *p);
                    x += c;
                    y += c;
                    w += c;

                    if (y == 0)
                        z = 1000000.0;
                    else
                        z = ((float)x) / ((float)y);
                    if (z < 0.5) { /* vertical */
                        a = 0;
                        b = 1;
                    } else {
                        if (z > 2.0) { /* horizontal */
                            a = 1;
                            b = 0;
                        } else { /* diagonal */
                            if (w > 0) {
                                a = -1;
                                b = 1;
                            } else {
                                a = 1;
                                b = 1;
                            }
                        }
                    }
                    if ((m > r[(i + a) * x_size + j + b]) &&
                        (m >= r[(i - a) * x_size + j - b]))
                        mid[i * x_size + j] = 2;
                }
            }
        }
}

#include "input.h"

void app_init() {
    unsigned char *in, *bp, *mid;
    float __attribute((aligned(8))) dt = 4.0;
    unsigned long *r, bt = 20, three_by_three = 0, max_no_edges = 2650, x_size,
                      y_size;
    input_large[sizeof(input_large) - 1] = EOF;
    log_info("Loading image");
    get_image(input_large, &in, &x_size, &y_size);

    log_info("Setting up brightness");
    setup_brightness_lut(&bp, bt, 2);
    log_info("Smoothing");
    susan_smoothing(three_by_three, in, dt, x_size, y_size, bp);

    r = (unsigned long *)malloc(x_size * y_size * sizeof(int));
    log_info("Setting up brightness");
    setup_brightness_lut(&bp, bt, 6);

    log_info("Principle");
    susan_principle(in, r, bp, max_no_edges, x_size, y_size);
    log_info("Into to Char");
    int_to_uchar(r, in, x_size * y_size);

    mid = (uchar *)malloc(x_size * y_size);
    memset(mid, 100, x_size * y_size); /* note not set to zero */

    log_info("Edges");
    susan_edges(in, r, mid, bp, max_no_edges, x_size, y_size);

    asm volatile("svc #0");
}

/* }}} */
