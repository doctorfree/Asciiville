#include <check.h>

#include "mocks.h"

static void check_parse_color_definition(
        char* definition,
        int expected_color,
        uint64_t expected_rgb)
{
    parse_color_definition(definition);

    fail_unless((iflags.color_definitions[expected_color] == expected_rgb),
            "\nFailed test for parse_color_definition(\"%s\"): \"%06x\" expected, \"%06x\" received",
            definition,
            (int)expected_rgb,
            (int)iflags.color_definitions[expected_color]);
}

START_TEST (test_parse_color_definition)
{
    check_parse_color_definition("blue:0000ff",  CLR_BLUE,  0x0000ff);
    check_parse_color_definition("red:FF0000",  CLR_RED,   0xff0000);
    check_parse_color_definition("white:11aabb", CLR_WHITE, 0x11aabb);
} END_TEST

Suite *test_suite(void)
{
    Suite *s = suite_create("NetHack options");
    TCase *tc_core = tcase_create("functions");

    suite_add_tcase (s, tc_core);

    tcase_add_test(tc_core, test_parse_color_definition);

    return s;
}

int main(int argc, char *argv[]) {
    int nf;
    Suite *s = test_suite();
    SRunner *sr = srunner_create(s);
    srunner_set_xml(sr, "test_options.xml");
    srunner_run_all(sr, CK_VERBOSE);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
