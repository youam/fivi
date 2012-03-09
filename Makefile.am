bin_PROGRAMS = fivi

fivi_SOURCES = main.c fileutils.c fileutils.h humanize.c humanize.h buffer.c buffer.h error.c error.h strutils.c strutils.h common.h
fivi_CFLAGS  = -Wall @GLIB_CFLAGS@
fivi_LDFLAGS = @GLIB_LIBS@

TESTS = test/*
EXTRA_DIST = test/*

testdata.in:
	tar cf testdata.in .

testdata.in.bz2: testdata.in
	bzip2 -9kf testdata.in

test2:	testdata.in testdata.in.bz2
