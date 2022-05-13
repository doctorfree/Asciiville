#include <check.h>

#include "mocks.h"

static void check_duration(
        char* (*function)(long),
        const char* function_name,
        long seconds,
        const char* expected_string)
{
    char *string = function(seconds);

    fail_unless(!strcmp(string, expected_string),
            "\nFailed test for %s(%ld): \"%s\" expected, \"%s\" received",
            function_name,
            seconds,
            expected_string,
            string);
}

START_TEST (test_formatted_duration)
{
    check_duration(&format_duration, "format_duration",      0,         "0m:00s");
    check_duration(&format_duration, "format_duration",      1,         "0m:01s");
    check_duration(&format_duration, "format_duration",     59,         "0m:59s");
    check_duration(&format_duration, "format_duration",     60,         "1m:00s");
    check_duration(&format_duration, "format_duration",   3599,        "59m:59s");
    check_duration(&format_duration, "format_duration",   3600,     "1h:00m:00s");
    check_duration(&format_duration, "format_duration",  86399,    "23h:59m:59s");
    check_duration(&format_duration, "format_duration",  86400, "1d:00h:00m:00s");
    check_duration(&format_duration, "format_duration", 172799, "1d:23h:59m:59s");
} END_TEST

START_TEST (test_iso8601_duration)
{
    check_duration(&iso8601_duration, "iso8601_duration",      0,    "PT00:00:00");
    check_duration(&iso8601_duration, "iso8601_duration",      1,    "PT00:00:01");
    check_duration(&iso8601_duration, "iso8601_duration",     59,    "PT00:00:59");
    check_duration(&iso8601_duration, "iso8601_duration",     60,    "PT00:01:00");
    check_duration(&iso8601_duration, "iso8601_duration",   3599,    "PT00:59:59");
    check_duration(&iso8601_duration, "iso8601_duration",   3600,    "PT01:00:00");
    check_duration(&iso8601_duration, "iso8601_duration",  86399,    "PT23:59:59");
    check_duration(&iso8601_duration, "iso8601_duration",  86400, "P01DT00:00:00");
    check_duration(&iso8601_duration, "iso8601_duration", 172799, "P01DT23:59:59");
} END_TEST

Suite *test_suite(void)
{
    Suite *s = suite_create("NetHack hacklib");
    TCase *tc_core = tcase_create("functions");

    suite_add_tcase (s, tc_core);

    tcase_add_test(tc_core, test_formatted_duration);
    tcase_add_test(tc_core, test_iso8601_duration);

    return s;
}

int main(int argc, char *argv[]) {
    int nf;
    Suite *s = test_suite();
    SRunner *sr = srunner_create(s);
    srunner_set_xml(sr, "test_hacklib.xml");
    srunner_run_all(sr, CK_VERBOSE);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
