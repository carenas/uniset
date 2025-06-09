#include "node.h"
#include "token.h"
#include "set.h"
#include <string.h>
#include <inttypes.h>
#include <stdlib.h>

static char const HELP[] =
"uniset: generate sets of Unicode characters.\n"
"Usage: uniset [OPTS..] [--] [SET]\n"
"\n"
"Options:\n"
"  --help         : show this help screen\n"
"  --16           : use planar output format\n"
"  --32           : use planar output format in hex without plane offsets\n"
"  --fmt=<string> : use the format string specified for C ranges (%u)\n"
"\n"
"Output format:\n"
"  Sorted list of non-overlapping ranges of characters in the set.\n"
"  Each range is of the format 'CHAR' or 'CHAR..CHAR'.  Characters\n"
"  are given in hexadecimal.  Each range appears on a separate line.\n"
"\n"
"Set operations:\n"
"  [SET] + [SET]  : union\n"
"  [SET] - [SET]  : difference\n"
"  [SET] * [SET]  : intersection\n"
"  ! [SET]        : complement\n"
"  ( [SET] )      : parentheses\n"
"\n"
"Basic sets:\n"
"  U+XXXX             : Individual character\n"
"  U+XXXX..U+XXXX     : Character range\n"
"  all                : All characters, U+0000..U+10FFFF\n"
"  ascii              : 7-bit characters, U+0000..U+007F\n"
"  cat:CAT1,CAT2,...  : General category (Lu,Ll,...)\n"
"  eaw:W1,W2,...      : East Asian width (F,H,W,Na,A,N)\n"
;

static int
valid_suffix(char const *restrict suffix)
{
    int r = 1, ll = 0, lu = 0, ul = 0, uu = 0;
    for (; *suffix; ++suffix) {
        switch (*suffix) {
        case 'L':
            if (ul == 2 || ll) {
                r = 0;
                goto out;
            }
            ul++;
            break;
        case 'U':
            if (uu) {
                r = 0;
                goto out;
            }
            uu++;
            break;
        case 'l':
            if (ll == 2 || ul) {
                r = 0;
                goto out;
            }
            ll++;
            break;
        case 'u':
            if (lu) {
                r = 0;
                goto out;
            }
            lu++;
            break;
        default:
            r = 0;
            goto out;
        }
    }
out:
    return r;
}

static int
valid_fmt(char const *restrict fmt)
{
    char buf[15];
    char const *percent;
#ifdef _LP64
    long l = 0x1ffffffff;
#define STRTOL strtol
#else
    long long l = 0x1ffffffff;
#define STRTOL strtoll
#endif
    int lower = 0;
    char *endptr = 0;
    size_t r, sz = strlen(fmt);
    if (sz > 9 || !sz)
        return 0;
    if (strcspn(fmt, " $'*+,.ABCEFGISZabcefghjmnpqstz{}") != sz)
        return 0;

    percent = strchr(fmt, '%');
    if (!percent || percent != strrchr(fmt, '%'))
        return 0;

    if (strchr(fmt, 'x') || strchr(fmt, 'X')) {
        r = (size_t)snprintf(buf, sizeof(buf), fmt, l);
        if (!r || r >= sizeof(buf) || STRTOL(buf, 0, 16) == l)
            return 0;
        r = (size_t)sprintf(buf, fmt, 17);
        l = strtol(buf, 0, 0);
        if (l != 17 || strspn(buf, " 01LUXlux") != r)
            return 0;
        lower = 2;
    } else if (strchr(fmt, 'o')) {
        r = (size_t)snprintf(buf, sizeof(buf), fmt, l);
        if (!r || r >= sizeof(buf) || STRTOL(buf, 0, 8) == l)
            return 0;
        r = (size_t)sprintf(buf, fmt, 9);
        l = strtol(buf, 0, 0);
        if (l != 9 || strspn(buf, " 01LUlu") != r)
            return 0;
        lower = 1;
    } else {
        r = (size_t)snprintf(buf, sizeof(buf), fmt, l);
        if (!r || r >= sizeof(buf) || STRTOL(buf, 0, 10) == l)
            return 0;
        r = (size_t)snprintf(buf, sizeof(buf), fmt, 1114111);
        if (r >= sizeof(buf) || strspn(buf, " 14LUlu") != r)
            return 0;
    }

    if (lower)
        sprintf(buf, fmt, 1);
    l = strtol(buf, &endptr, 0);

    if (l != (lower ? 1 : 1114111))
        return 0;

    return valid_suffix(endptr);
}

int main(int argc, char *argv[])
{
    struct tokenizer t;
    struct node *n;
    struct set *s;
    int i;
    char *opt;
    char const *fmt = "%u";
    int format = 0, verbose = 0;

    for (i = 1; i < argc; ++i) {
        if (memcmp(argv[i], "--", 2))
            break;
        opt = argv[i] + 2;
        if (!strcmp(opt, "16")) {
            format = 1;
        } else if (!strcmp(opt, "32")) {
            format = 2;
        } else if (!strcmp(opt, "help")) {
            fputs(HELP, stderr);
            return 1;
        } else if (!strcmp(opt, "verbose")) {
            verbose = 1;
        } else if (!memcmp(opt, "fmt=", 4)) {
            fmt = opt + 4;
            if (!valid_fmt(fmt)) {
                fprintf(stderr, "Invalid format string: '%s'\n", fmt);
                return 1;
            }
        } else {
            fprintf(stderr, "Invalid option: '%s'\n", argv[i]);
            return 1;
        }
    }
    if (i == argc) {
        fputs(HELP, stderr);
        return 1;
    }

    tokenizer_init(&t);
    tokenizer_addtext(&t, argv + i);
    n = node_read(&t);
    tokenizer_destroy(&t);

    if (verbose) {
        fputs("Expression:", stderr);
        node_print(stderr, n);
        fputs("\n\n", stderr);
    }

    s = node_eval(n);
    if (!s->length)
        fputs("Warning: Set is empty\n", stderr);
    switch (format) {
    case 0:
        set_print(stdout, s);
        break;
    case 1:
        set_print16(stdout, s, fmt);
        break;
    case 2:
        set_print32(stdout, s, fmt);
        break;
    }

    return 0;
}
