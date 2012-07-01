#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <check.h>
#include <unistd.h>

#include <pulsecore/cpu-x86.h>

START_TEST (svolume_mmx_test)
{
    svolume_mmx_unit_test();
}
END_TEST

START_TEST (svolume_sse_test)
{
    svolume_sse_unit_test();
}
END_TEST

START_TEST (sconv_sse_test)
{
    sconv_sse_unit_test();
}
END_TEST

int main(int argc, char *argv[]) {
    pa_cpu_x86_flag_t flags = 0;
    int failed = 0;

    pa_cpu_get_x86_flags(&flags);

    Suite *s = suite_create("CPU");
    TCase *tc = tcase_create("x86");
    if (flags & PA_CPU_X86_MMX)
        tcase_add_test(tc, svolume_mmx_test);
    if (flags & (PA_CPU_X86_SSE | PA_CPU_X86_SSE2)) {
        tcase_add_test(tc, svolume_sse_test);
        tcase_add_test(tc, sconv_sse_test);
    }
    suite_add_tcase(s, tc);

    SRunner *sr = srunner_create(s);
    srunner_run_all(sr, CK_NORMAL);
    failed = srunner_ntests_failed(sr);
    srunner_free(sr);

    return (failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
