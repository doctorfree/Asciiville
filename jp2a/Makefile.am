VERSION = @VERSION@
SUBDIRS = src
man_MANS = man/jp2a.1
bashcompdir = @bashcompdir@
dist_bashcomp_DATA = completion/bash/jp2a
EXTRA_DIST = tests/??*.txt tests/??*.html tests/??*.jpg tests/??*.png tests/run-tests.sh tests/Makefile include/??*.h man/jp2a.1 Doxyfile.in LICENSES
MOSTLYCLEANFILES = $(DX_CLEANFILES)
CLEANFILES = Doxyfile

test: check

check: all
	$(MAKE) -C tests check

@DX_RULES@

editdx = sed -e 's|@PACKAGE_NAME[@]|$(PACKAGE_NAME)|g' -e 's|@PACKAGE_VERSION[@]|$(PACKAGE_VERSION)|g' -e 's|@top_srcdir[@]|$(top_srcdir)|g'
Doxyfile: Doxyfile.in
	$(editdx) $(srcdir)/$@.in > $@.tmp
	mv $@.tmp $@
