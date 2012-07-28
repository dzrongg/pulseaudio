/***
  This file is part of PulseAudio.

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published
  by the Free Software Foundation; either version 2.1 of the License,
  or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include <check.h>

#include <pulse/xmalloc.h>
#include <pulsecore/macro.h>

static size_t granularity;
static FILE *i;
static FILE *o;

START_TEST (stripnul_test) {
    pa_bool_t found = FALSE;
    uint8_t *zero;

    /* pre-check */
    fail_unless(granularity >= 1);
    fail_unless(i != NULL);
    fail_unless(o != NULL);

    zero = pa_xmalloc0(granularity);

    for (;;) {
        uint8_t buffer[16*1024], *p;
        size_t k;

        k = fread(buffer, granularity, sizeof(buffer)/granularity, i);

        if (k <= 0)
            break;

        if (found)
            fail_unless(fwrite(buffer, granularity, k, o) == k);
        else {
            for (p = buffer; ((size_t) (p-buffer)/granularity) < k; p += granularity)
                if (memcmp(p, zero, granularity)) {
                    size_t left;
                    found = TRUE;
                    left = (size_t) (k - (size_t) (p-buffer)/granularity);
                    fail_unless(fwrite(p, granularity, left, o) == left);
                    break;
                }
        }
    }

    fflush(o);
}
END_TEST

int main(int argc, char *argv[]) {
    int failed = 0;
    Suite *s;
    TCase *tc;
    SRunner *sr;

    if (argc < 2) {
        fprintf(stderr, "need at least one arg to run this test!\n");
        return EXIT_FAILURE;
    }

    (granularity = (size_t) atoi(argv[1]));
    i = (argc >= 3) ? fopen(argv[2], "r") : stdin;
    o = (argc >= 4) ? fopen(argv[3], "w") : stdout;

    s = suite_create("Stripnul");
    tc = tcase_create("stripnul");
    tcase_add_test(tc, stripnul_test);
    suite_add_tcase(s, tc);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
