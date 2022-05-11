#include <check.h>

#include "mocks.h"

void check_parse_codepoint(char* codepoint, int expected_codepoint)
{
	int num = parse_codepoint(codepoint);

	fail_unless((num==expected_codepoint),
		"\nFailed test for parse_codepoint(\"%s\"): \"%d\" expected, \"%d\" received",
		codepoint, expected_codepoint, num);
}

START_TEST (test_parse_codepoint)
{
	check_parse_codepoint("u+0000", 0);
	check_parse_codepoint("u+ffff", 65535);
	check_parse_codepoint("U+FFFF", 65535);
	check_parse_codepoint("0x2648", 9800);
	check_parse_codepoint("9670", 9670);
} END_TEST

void check_parse_object_symbol(char* option_line, int expected_object, int expected_codepoint)
{
	parse_object_symbol(option_line);

	fail_unless(objects[expected_object].unicode_codepoint==expected_codepoint,
		"\nFailed test for parse_object_symbol(\"%s\"): \"%d\" expected, \"%d\" received",
		option_line, expected_codepoint, objects[expected_object].unicode_codepoint);
}

START_TEST (test_parse_object_symbol)
{
	/* items with actual and description name, only description name is allowed */
	check_parse_object_symbol("'key':1234", SKELETON_KEY, 1234);

	/* items with actual and no description name */
	check_parse_object_symbol("'carrot':2345", CARROT, 2345);

	/* items with actual and description name shouldn't work their actual name */
	check_parse_object_symbol("cornuthaum:3456", CORNUTHAUM, 0);
	check_parse_object_symbol("conical hat:3456", CORNUTHAUM, 3456);

	/* items with incomplete descriptive name in objects.c */
	//check_parse_object_symbol("orange potion:1000", CORNUTHAUM, 1000);
	//check_parse_object_symbol("orange potion:1000", ORANGE, 0);
	//check_parse_object_symbol("yellow gem:2000", CITRINE, 2000);
	//check_parse_object_symbol("yellow gem:2000", CHRYSOBERYL, 2000);

	/* wishable subranges */
	//check_parse_object_symbol("shoes:1000", LOW_BOOTS, 1000);
	//check_parse_object_symbol("shield:2000", SHIELD_OF_REFLECTION, 2000);
	//check_parse_object_symbol("wands:2000", SHIELD_OF_REFLECTION, 2000);

} END_TEST

Suite *test_suite(void)
{
	Suite *s = suite_create("UnNetHack Unicode functions");
	TCase *tc_core = tcase_create("functions");

	suite_add_tcase (s, tc_core);

	tcase_add_test(tc_core, test_parse_codepoint);
	tcase_add_test(tc_core, test_parse_object_symbol);

	return s;
}

int main(int argc, char *argv[]) {
	int nf;
	Suite *s = test_suite();
	SRunner *sr = srunner_create(s);
	srunner_set_xml(sr, "test_unicode.xml");
	srunner_run_all(sr, CK_VERBOSE);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
