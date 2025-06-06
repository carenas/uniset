#include "set.h"

void
set_print(FILE *f, struct set const *x)
{
    struct range const *xp = x->r, *xe = x->r + x->length;
    for (; xp != xe; ++xp) {
        if (xp->first == xp->last)
            fprintf(f, "%x\n", xp->first);
        else
            fprintf(f, "%x..%x\n", xp->first, xp->last);
    }
}

static void
set_print_n(FILE *f, struct set const *x, int planes, unsigned int mask,
            char const *fmt)
{
    unsigned int plane = 0, fr, la, pf, pl;
    struct range const *xs = x->r, *xe = x->r + x->length, *xp;
    if (planes) {
        unsigned int ranges[17][2];
        unsigned int pos = 0;
        xp = xs;
        for (plane = 0; plane < 17; ++plane) {
            if (xp == xe || (xp->first >> 16) > plane) {
                ranges[plane][0] = 0;
                ranges[plane][1] = 0;
            } else {
                ranges[plane][0] = pos;
                while (xp != xe && (xp->last >> 16) == plane) {
                    pos++;
                    xp++;
                }
                if (xp != xe && (xp->first >> 16) <= plane)
                    pos++;
                ranges[plane][1] = pos;
            }
            if (plane) fputs(",\n", f);
            fprintf(f, "{ /* plane %u */ %u, %u }", plane,
                    ranges[plane][0], ranges[plane][1]);
        }
    }
    for (xp = xs; xp != xe; ++xp) {
        char line_fmt[32];
        sprintf(line_fmt, "{ %s, %s }", fmt, fmt);
        if (!planes || ((xp->first ^ xp->last) >> 16) == 0) {
            if (plane) fputs(",\n", f);
            plane = 1;
            fprintf(f, line_fmt, xp->first & mask, xp->last & mask);
        } else {
            pf = xp->first >> 16;
            pl = xp->last >> 16;
            for (unsigned int p = pf; p <= pl; ++p) {
                fr = p == pf ? xp->first : p << 16;
                la = p == pl ? xp->last : p << 16 | 0xffff;
                if (plane) fputs(",\n", f);
                plane = 1;
                fprintf(f, line_fmt, fr & mask, la & mask);
            }
        }
    }
    putc('\n', f);
}

void
set_print16(FILE *f, struct set const *x, char const *fmt)
{
    set_print_n(f, x, 1, 0xffff, fmt);
}

void
set_print32(FILE *f, struct set const *x, char const *fmt)
{
    set_print_n(f, x, 0, 0xffffffff, fmt);
}
