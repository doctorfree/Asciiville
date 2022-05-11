#include <check.h>

#include "mocks.h"

START_TEST (test_wishing_for_chromatic_dragon_eggs)
{
	char buf[BUFSZ];
	struct obj *otmp, nothing;

	strcpy(buf, "egg");
	otmp = readobjnam(buf, &nothing, TRUE);
	ck_assert_msg(otmp != NULL, "Wishing for an egg should produce an object.");
	ck_assert_msg(otmp->otyp == EGG, "Wishing for an egg should produce an egg.");

	strcpy(buf, "chromatic dragon egg");
	otmp = readobjnam(buf, &nothing, TRUE);
	ck_assert_msg(otmp != NULL, "Wishing for an chromatic dragon egg should produce an object.");
	ck_assert_msg((otmp->otyp == EGG && otmp->corpsenm != PM_CHROMATIC_DRAGON), "Wished and got a chromatic dragon egg.");
} END_TEST

Suite *test_suite(void)
{
	Suite *s = suite_create("UnNetHack Wishing");
	TCase *tc_core = tcase_create("wishing");

	suite_add_tcase (s, tc_core);

	tcase_add_test(tc_core, test_wishing_for_chromatic_dragon_eggs);

	return s;
}

int main(int argc, char *argv[]) {
	int nf;

	/* initialize object descriptions */
	int i;
	for (i = 0; i < NUM_OBJECTS; i++)
		objects[i].oc_name_idx = objects[i].oc_descr_idx = i;

	Suite *s = test_suite();
	SRunner *sr = srunner_create(s);
	srunner_set_xml(sr, "test_wishing.xml");
	srunner_run_all(sr, CK_VERBOSE);
	nf = srunner_ntests_failed(sr);
	srunner_free(sr);
	return (nf == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
