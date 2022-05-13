#include <check.h>

#include "mocks.h"

static void
check_decode_base32(
        const char* input,
        unsigned long expected_number)
{
    long converted_number = decode_base32(input);
    fail_unless((converted_number == expected_number),
            "\n\nFailed expected \"%s\" to convert to %lu but got %lu.\n",
            input, expected_number, converted_number);
}

START_TEST (test_decode_base32)
{
    check_decode_base32("0",  0);
    check_decode_base32("00", 0);
    check_decode_base32("01", 1);

    // ignore unmapped characters
    check_decode_base32(" 1", 1);
    check_decode_base32("_2_", 2);
    check_decode_base32("11",   33);
    check_decode_base32("1u1o", 33);

    check_decode_base32("a",  10);
    check_decode_base32("A",  10);
    check_decode_base32("z",  31);
    check_decode_base32("Z",  31);

    // characters with same value
    check_decode_base32("0", 0);
    check_decode_base32("o", 0);
    check_decode_base32("O", 0);
    check_decode_base32("1", 1);
    check_decode_base32("i", 1);
    check_decode_base32("I", 1);
    check_decode_base32("l", 1);
    check_decode_base32("L", 1);

    check_decode_base32("0aa", 330);
    check_decode_base32("0AA", 330);
    check_decode_base32(" az", 351);
    check_decode_base32("0AAA", 10570);

    check_decode_base32("62", 194);
    check_decode_base32("FVCK", 519571);
    check_decode_base32("c515", 398373);
    check_decode_base32("1CKE", 45678);
    check_decode_base32("ABCDEF", 347485647);
    check_decode_base32("FZZZZZZZZZZZZ", 18446744073709551615U);
} END_TEST

static void
check_encode_base32(
        unsigned long input,
        char* expected_output)
{
    const char *converted = encode_base32(input);
    fail_unless((!strcmp(converted, expected_output)),
            "\n\nFailed expected %lu to convert to \"%s\" but got \"%s\".\n",
            input, expected_output, converted);
}

START_TEST (test_encode_base32)
{
    check_encode_base32(0, "0");
    check_encode_base32(1, "i");

    check_encode_base32(10, "a");
    check_encode_base32(31, "z");

    check_encode_base32(398373, "c5i5");
    check_encode_base32(45678,  "icke");

    check_encode_base32(330,    "aa");
    check_encode_base32(351,    "az");
    check_encode_base32(10570, "aaa");
    check_encode_base32(347485647, "abcdef");
} END_TEST

Suite *test_suite(void)
{
    Suite *s = suite_create("NetHack options");
    TCase *tc_core = tcase_create("functions");

    suite_add_tcase (s, tc_core);

    tcase_add_test(tc_core, test_decode_base32);
    tcase_add_test(tc_core, test_encode_base32);

    return s;
}

int main(int argc, char *argv[]) {
    int nf;
    Suite *s = test_suite();
    SRunner *sr = srunner_create(s);
    srunner_set_xml(sr, "test_base32.xml");
    srunner_run_all(sr, CK_VERBOSE);
    nf = srunner_ntests_failed(sr);
    srunner_free(sr);
    return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
