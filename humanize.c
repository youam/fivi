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
#include "humanize.h"

#include <math.h>

char *humanize(off_t n)
{
	char *r;
	int i;

	int maxdigits=4;
	int base = 10;
	off_t outputint = n;

	off_t step = 1024;
	char mult[] = { ' ', 'K', 'M', 'G', '\0' };

	int range = base;
	for ( i = 1 ; i < maxdigits; i++ ) {
		range *= base;
	}

	i = 0;
	while ( mult[i] && outputint > range ) {
		i++;
		outputint /= step;
	}

	assert( mult[i] != '\0' );

	i=asprintf(&r,"%*lli%c", maxdigits, outputint, mult[i] );
	if ( i == -1 ) {
		r = NULL;
	}
	return r;
}

/*
 * human readable duration
 *    0.0s
 *   10.2s
 *   9m59s
 *   12,0m
 *   23,0h
 *   9d23h
 *   10,1d
 *   9999d */
char *human_time(double t)
{
	char *s = "undef";
	int r = 0;
	
	int x = fmod( t * 10, 10 );
	int S = fmod( t, 60 );
	int M = fmod( t / 60, 60 );
	int H = fmod( t / 60 / 60, 24 );
	int D =       t / 60 / 60 / 24;

	if ( D > 9999 ) {
		s = "INVAL";
	} else if ( D >= 100 ) {
		r = asprintf( &s, "%4id", D );
	}

	if ( t < 0 ) {
		fprintf( stderr, "oops. duration can't be negative!\n" );
		s = "     ";
	} else if ( t < 60 ) {
		r = asprintf( &s, "%4.1fs", t );
	} else if ( t < 60 * 10 ) {
		r = asprintf( &s, "%im%02is", (int)(t / 60), (int)fmod( t, 60 ) );
	} else if ( t < 60 * 60 ) {
		r = asprintf( &s, "%4.1fm", t / 60 );
	}

	if ( r == -1 ) {
		s = NULL;
	}
	return s;
}
