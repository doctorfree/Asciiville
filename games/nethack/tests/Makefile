all: options/test_base32 \
     hacklib/test_hacklib \
     options/test_options \
     unicode/test_unicode \
     wishing/test_wishing

clean:
	rm -f `find */test_* -executable`

TEST_CFLAGS=-Wall -Wno-unused-variable -Wno-unused-but-set-variable
%: %.c
	echo `pkg-config --libs check`
	$(CC) $(TEST_CFLAGS) -I../include -I../src -g $(CFLAGS) $@.c `pkg-config --libs check` -o $@ && ./$@
