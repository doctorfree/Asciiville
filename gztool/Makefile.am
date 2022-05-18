EXTRA_DIST = tests
TESTS = tests/test.sh
TESTS_ENVIRONMENT = \
        gztool_abspath='$(abs_top_builddir)'/gztool; \
	export gztool_abspath;
bin_PROGRAMS = gztool
gztool_SOURCES = gztool.c
gztool_LDADD = -lm -lz
dist_doc_DATA = README.md
dist_man_MANS = gztool.1
dist_noinst_DATA = tests
