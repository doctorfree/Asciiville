#!/bin/bash

# Copyright 2006-2016 Christian Stigen Larsen
# Copyright 2020 Christoph Raitzig
#
# This is a small script to test if jp2a has been correctly built.

## PATH TO EXECUTABLE jp2a
JP=../src/jp2a
UNAME=`uname -a`
UPDATE=no

if test "`echo ${UNAME} | cut -c1-6`" == "CYGWIN" ; then
	JP=../src/jp2a.exe
fi

if test "${1}" == "update" ; then
	UPDATE=yes
fi

## INITIALIZE VARS
RESULT_OK=0
RESULT_FAILED=0
FAILED_STR=""

function print_intense() {
	echo -e -n "\033[1m${1}\033[0m"
}

function test_ok() {
	print_intense "OK"
	RESULT_OK=$((RESULT_OK + 1))
}

function test_failed() {
	print_intense "FAILED"
	RESULT_FAILED=$((RESULT_FAILED + 1))
	FAILED_STR="${FAILED_STR}\n${2} | diff --strip-trailing-cr --brief - ${1}"
}

function test_jp2a() {
	CMD="${JP} ${2}"
	printf "test (%2s) %-45s " "$((RESULT_OK+RESULT_FAILED+1))" "(${1})"

	if [ ! -e "${3}" ] ; then
		print_intense "(missing ${3}) "
		test_failed ${3} "${CMD}"
	else
		if test "${UPDATE}" == "no" ; then
			eval ${CMD} | diff --strip-trailing-cr --brief - ${3} 1>/dev/null && test_ok || test_failed ${3} "${CMD}"
		else
			eval ${CMD} > ${3}
			test_ok
		fi
	fi

	echo ""
}

function test_results() {
	echo ""
	echo "TEST RESULTS FOR JP2A"
	echo ""
	printf "Tests OK    : %2d of %2d  (%4s)\n" "$((RESULT_OK))" "$((RESULT_OK+RESULT_FAILED))" "$((100*RESULT_OK/(RESULT_OK+RESULT_FAILED)))%"
	printf "Tests FAILED: %2d of %2d  (%4s)\n" "$((RESULT_FAILED))" "$((RESULT_OK+RESULT_FAILED))" "$((100*RESULT_FAILED/(RESULT_OK+RESULT_FAILED)))%"
	echo ""

	if test "x${FAILED_STR}" != "x" ; then
		echo "Summary of failed tests:"
		echo -e "${FAILED_STR}"
	fi
}

echo "-------------------------------------------------------------"
echo " TESTING JP2A BUILD"
echo " "
echo " Note that the output may vary a bit on different platforms,"
echo " so some tests may fail.  This does not mean that jp2a is"
echo " completely broken."
echo "-------------------------------------------------------------"
echo ""

test_jp2a "width" "--width=78 jp2a.jpg" normal.txt
test_jp2a "border, width" "-b --width=78 jp2a.jpg" normal-b.txt
test_jp2a "size" "--size=160x49 jp2a.jpg" 160x49.txt
test_jp2a "height" "--height=10 jp2a.jpg" 10h.txt
test_jp2a "size" "--size=40x40 jp2a.jpg" 40x40.txt
test_jp2a "size" "--size=1x1 --invert jp2a.jpg" 1x1-inv.txt
test_jp2a "invert, border" "-i -b --width=110 --height=30 jp2a.jpg" 110x30-i-b.txt
test_jp2a "width, flip, invert" "--width=78 --flipx --flipy --invert jp2a.jpg" flip-xy-invert.txt
test_jp2a "width, border" "--width=78 -b jp2a.jpg jp2a.jpg" 2xnormal-b.txt
test_jp2a "width, verbose" "--verbose --width=78 jp2a.jpg 2>&1 | tr -d '\r'" normal-verbose.txt

TEMPFILE=`mktemp /tmp/jp2a-test-XXXXXX`
test_jp2a "width, outfile" "--width=78 jp2a.jpg --output=${TEMPFILE} && cat ${TEMPFILE}" normal.txt
rm -f ${TEMPFILE}

test_jp2a "width, clear" "--width=78 jp2a.jpg --clear" normal-clear.txt
test_jp2a "height, grayscale" "logo-40x25-gray.jpg --height=30" logo-30.txt
test_jp2a "size, invert" "grind.jpg -i --size=80x30" grind.txt
test_jp2a "size, invert, red channel" "grind.jpg -i --size=80x30 --red=1.0 --green=0.0 --blue=0.0" grind-red.txt
test_jp2a "size, invert, blue channel" "grind.jpg -i --size=80x30 --red=0.0 --green=1.0 --blue=0.0" grind-green.txt
test_jp2a "size, invert, green channel" "grind.jpg -i --size=80x30 --red=0.0 --green=0.0 --blue=1.0" grind-blue.txt
test_jp2a "width, grayscale" "--width=78 dalsnuten-640x480-gray-low.jpg" dalsnuten-normal.txt
test_jp2a "invert, width, grayscale" "--invert --width=78 dalsnuten-640x480-gray-low.jpg" dalsnuten-invert.txt
test_jp2a "invert, size, grayscale" "--invert --size=80x49 dalsnuten-640x480-gray-low.jpg" dalsnuten-80x49-inv.txt
test_jp2a "size, invert, border" "dalsnuten-640x480-gray-low.jpg --size=80x25 --invert --border --size=150x45" dalsnuten-640x480-gray-low.txt
test_jp2a "size, html" "--size=80x50 --htmlls --html-fontsize=7 jp2a.jpg" logo.html
test_jp2a "size, xhtml" "--size=80x50 --xhtml --html-fontsize=7 jp2a.jpg" logo-x.html
test_jp2a "size, color, html, flipx" "--size=80x50 --htmlls --color --html-fontsize=8 --flipx grind.jpg" grind-flipx.html
test_jp2a "size, color, html, flipy" "--size=80x50 --htmlls --color --html-fontsize=8 --flipy grind.jpg" grind-flipy.html
test_jp2a "size, color, html, flipxy" "--size=80x50 --htmlls --color --html-fontsize=8 --flipx --flipy grind.jpg" grind-flipxy.html
test_jp2a "size, color, xhtml, flipx" "--size=80x50 --xhtml --color --html-fontsize=8 --flipx grind.jpg" grind-flipx-x.html
test_jp2a "size, color, xhtml, flipy" "--size=80x50 --xhtml --color --html-fontsize=8 --flipy grind.jpg" grind-flipy-x.html
test_jp2a "size, color, xhtml, flipxy" "--size=80x50 --xhtml --color --html-fontsize=8 --flipx --flipy grind.jpg" grind-flipxy-x.html
test_jp2a "width, html, gray, dark" "dalsnuten-640x480-gray-low.jpg --width=128 --htmlls --html-fontsize=8" dalsnuten-256.html
test_jp2a "width, html, gray, light" "dalsnuten-640x480-gray-low.jpg --width=128 --background=light --htmlls --html-fontsize=8" dalsnuten-256-light.html
test_jp2a "color, html, dark" "grind.jpg --color --background=dark --width=60 --htmlls --html-fontsize=8" grind-color-dark.html
test_jp2a "color, html, dark fill" "grind.jpg --color --fill --background=dark --width=60 --htmlls --html-fontsize=8" grind-color-dark-fill.html
test_jp2a "color, html, light" "grind.jpg --color --fill --background=light --width=60 --htmlls --html-fontsize=8" grind-color.html
test_jp2a "color, html, grayscale" "dalsnuten-640x480-gray-low.jpg --color --width=78 --htmlls --background=light --fill --html-fontsize=8" dalsnuten-color.html
test_jp2a "color, html, --grayscale" "grind.jpg --color --width=78 --htmlls --grayscale" grind-2grayscale.html
test_jp2a "color, html, --grayscale, fill" "grind.jpg --color --width=78 --htmlls --grayscale --fill" grind-2grayscale-fill.html
test_jp2a "width, xhtml, gray, dark" "dalsnuten-640x480-gray-low.jpg --width=128 --xhtml --html-fontsize=8" dalsnuten-256-x.html
test_jp2a "width, xhtml, gray, light" "dalsnuten-640x480-gray-low.jpg --width=128 --background=light --xhtml --html-fontsize=8" dalsnuten-256-light-x.html
test_jp2a "color, xhtml, dark" "grind.jpg --color --background=dark --width=60 --xhtml --html-fontsize=8" grind-color-dark-x.html
test_jp2a "color, xhtml, dark fill" "grind.jpg --color --fill --background=dark --width=60 --xhtml --html-fontsize=8" grind-color-dark-fill-x.html
test_jp2a "color, xhtml, light" "grind.jpg --color --fill --background=light --width=60 --xhtml --html-fontsize=8" grind-color-x.html
test_jp2a "color, xhtml, grayscale" "dalsnuten-640x480-gray-low.jpg --color --width=78 --xhtml --background=light --fill --html-fontsize=8" dalsnuten-color-x.html
test_jp2a "color, xhtml, --grayscale" "grind.jpg --color --width=78 --xhtml --grayscale" grind-2grayscale-x.html
test_jp2a "color, xhtml, --grayscale, fill" "grind.jpg --color --width=78 --xhtml --grayscale --fill" grind-2grayscale-fill-x.html
test_jp2a "color, ANSI, fill" "grind.jpg --color-depth=4 --fill --width=78" grind-ANSI-fill.txt
test_jp2a "color, ANSI, fill, --grayscale" "grind.jpg --color-depth=4 --fill --grayscale --width=78" grind-2grayscale-ANSI-fill.txt
test_jp2a "color, 256 color palette, fill" "grind.jpg --color-depth=8 --fill --width=78" grind-256-fill.txt
test_jp2a "color, 256 color palette, fill, --grayscale" "grind.jpg --color-depth=8 --fill --grayscale --width=78" grind-2grayscale-256-fill.txt
test_jp2a "color, truecolor, fill" "grind.jpg --color-depth=24 --fill --width=78" grind-truecolor-fill.txt
test_jp2a "color, truecolor, fill, --grayscale" "grind.jpg --color-depth=24 --fill --grayscale --width=78" grind-2grayscale-truecolor-fill.txt
test_jp2a "color, html, no-bold" "grind.jpg --colors --htmlls --html-no-bold --width=78" grind-nobold.html
test_jp2a "color, xhtml, no-bold" "grind.jpg --colors --xhtml --html-no-bold --width=78" grind-nobold-x.html
test_jp2a "html-title, xhtml" "--width=10 --xhtml --html-title='just testing' jp2a.jpg" html-title.txt
test_jp2a "html-title, html" "--size=80x50 --htmlls --html-title=\"C&amp;amp;\\\"&'<script>\" jp2a.jpg" html-title-entities.html
test_jp2a "color, html-raw" "--width=10 --color --html-raw jp2a.jpg" html-raw.txt
test_jp2a "color, ANSI" "grind.jpg --color-depth=4 --width=60" grind-color-ANSI.txt
test_jp2a "color, ANSI, grayscale" "dalsnuten-640x480-gray-low.jpg --color-depth=4 --width=78" dalsnuten-color-ANSI.txt
test_jp2a "color, 256 color palette" "grind.jpg --color-depth=8 --width=60" grind-color-256.txt
test_jp2a "color, 256 color palette, grayscale" "dalsnuten-640x480-gray-low.jpg --color-depth=8 --width=78" dalsnuten-color-256.txt
test_jp2a "color, truecolor" "grind.jpg --color-depth=24 --width=60" grind-color-truecolor.txt
test_jp2a "color, truecolor, grayscale" "dalsnuten-640x480-gray-low.jpg --color-depth=24 --width=78" dalsnuten-color-truecolor.txt
test_jp2a "standard input, width" " 2>/dev/null ; cat jp2a.jpg | ${JP} --width=78 -" normal.txt
test_jp2a "standard input, width, height" " 2>/dev/null ; cat jp2a.jpg | ${JP} - --width=40 --height=40" 40x40.txt
test_jp2a "big size" "--size=2000x2000 dalsnuten-640x480-gray-low.jpg jp2a.jpg | tr -d '\r' | wc -c | tr -d ' '" dalsnuten-jp2a-2000x2000-md5.txt
test_jp2a "PNG invert" "--invert --color --size=144x36 jp2a-colorful.png" jp2a-png-invert.txt
test_jp2a "PNG colormap standard input" "2>/dev/null ; cat jp2a-colormap.png | ${JP} - --size=144x36" jp2a-png-colormap.txt
test_jp2a "PNG interlaced 16-bit" "--size=300x150 --color jp2a-interlaced-16-bit.png" jp2a-png-interlaced-16-bit.txt
test_jp2a "size, curl download" "--size=454x207 http://jp2a.sourceforge.net/jp2a.jpg" normal-curl.txt
test_jp2a "size, curl download" "--size=454x207 http://jp2a.sf.net/jp2a.jpg" normal-curl.txt
test_jp2a "very long jpg" "long.jpg" no-output.txt
test_jp2a "very long png" "long.png" no-output.txt

test_results
