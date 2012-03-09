#ifndef STRUTILS_H
#define STRUTILS_H
/*
 * Copyright (c) 2005 Uli Martens <uli@youam.net>
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions: 
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include "common.h"


struct strmod_s {
	int humanize;
};


typedef int strfnc( FILE *f, void *p1, struct strmod_s *m );

void strmod_clear( struct strmod_s *m );


struct strpart_s;

strfnc str_char;
strfnc str_str;
strfnc str_time;
strfnc str_off_t;
strfnc str_percent;
strfnc str_int;

struct strpart_s *straddpart( struct strpart_s *p, strfnc *fnc, void *p1, struct strmod_s *m );

int pprintf( FILE *f, struct strpart_s *p );

#endif	/* STRUTILS_H */
