/***
  This file is part of PulseAudio.

  Copyright 2008 Lennart Poettering

  PulseAudio is free software; you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as
  published by the Free Software Foundation; either version 2.1 of the
  License, or (at your option) any later version.

  PulseAudio is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with PulseAudio; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
  USA.
***/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <inttypes.h>

#include <check.h>

#ifdef HAVE_PTHREAD
#include <pthread.h>
#endif

#include <pulse/util.h>
#include <pulse/timeval.h>
#include <pulse/gccmacro.h>

#include <pulsecore/log.h>
#include <pulsecore/macro.h>
#include <pulsecore/thread.h>
#include <pulsecore/core-util.h>
#include <pulsecore/core-rtclock.h>

static int msec_lower, msec_upper;

static void work(void *p) PA_GCC_NORETURN;

static void work(void *p) {

    pa_log_notice("CPU%i: Created thread.", PA_PTR_TO_UINT(p));

    pa_make_realtime(12);

#ifdef HAVE_PTHREAD_SETAFFINITY_NP
{
    cpu_set_t mask;

    CPU_ZERO(&mask);
    CPU_SET((size_t) PA_PTR_TO_UINT(p), &mask);
    fail_unless(pthread_setaffinity_np(pthread_self(), sizeof(mask), &mask) == 0);
}
#endif

    for (;;) {
        struct timeval now, end;
        uint64_t usec;

        pa_log_notice("CPU%i: Sleeping for 1s", PA_PTR_TO_UINT(p));
        pa_msleep(1000);

        usec =
            (uint64_t) ((((double) rand())*(double)(msec_upper-msec_lower)*PA_USEC_PER_MSEC)/RAND_MAX) +
            (uint64_t) ((uint64_t) msec_lower*PA_USEC_PER_MSEC);

        pa_log_notice("CPU%i: Freezing for %ims", PA_PTR_TO_UINT(p), (int) (usec/PA_USEC_PER_MSEC));

        pa_rtclock_get(&end);
        pa_timeval_add(&end, usec);

        do {
            pa_rtclock_get(&now);
        } while (pa_timeval_cmp(&now, &end) < 0);
    }
}

START_TEST (rtstutter_test) {
    unsigned n;

    pa_log_set_level(PA_LOG_INFO);

    srand((unsigned) time(NULL));

    fail_unless(msec_upper > 0);
    fail_unless(msec_upper >= msec_lower);

    pa_log_notice("Creating random latencies in the range of %ims to %ims.", msec_lower, msec_upper);

    for (n = 1; n < pa_ncpus(); n++) {
        pa_thread *p = pa_thread_new("rtstutter", work, PA_UINT_TO_PTR(n));
        fail_unless(p != NULL);
    }

    work(PA_INT_TO_PTR(0));
}
END_TEST

int main(int argc, char *argv[]) {
    int failed = 0;
    Suite *s;
    TCase *tc;
    SRunner *sr;

    if (argc >= 3) {
        msec_lower = atoi(argv[1]);
        msec_upper = atoi(argv[2]);
    } else if (argc >= 2) {
        msec_lower = 0;
        msec_upper = atoi(argv[1]);
    } else {
        msec_lower = 0;
        msec_upper = 1000;
    }

    s = suite_create("Rtstutter");
    tc = tcase_create("rtstutter");
    tcase_add_test(tc, rtstutter_test);
    suite_add_tcase(s, tc);

    sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
