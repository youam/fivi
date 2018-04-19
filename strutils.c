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
#include "strutils.h"
#include "humanize.h"

#include <assert.h>
void strmod_clear( struct strmod_s *m )
{
	m->humanize = 0;
}

struct strpart_s {
	strfnc *fnc;
	void *p1;
	struct strmod_s *m;

	struct strpart_s *next;
};


int str_char( FILE *f, void *p1, struct strmod_s *m )
{
	char *c = p1;
	return fprintf( f, "%c", *c );
}

int str_int( FILE *f, void *p1, struct strmod_s *m )
{
	int *c = p1;
	return fprintf( f, "%i", *c );
}

int str_str( FILE *f, void *p1, struct strmod_s *m )
{
	return fprintf( f, "%s", (char *)p1 );
}

int str_time( FILE *f, void *p1, struct strmod_s *m )
{
	double *v = p1;
	if ( m->humanize ) {
		return fprintf( f, "%s", human_time( *v ) );
	} else {
		return fprintf( f, "%5.1fs", *v );
	}
}

int str_off_t( FILE *f, void *p1, struct strmod_s *m )
{
	off_t *v = p1;
	if ( m->humanize )
		//FIXME humanize() leaks
		return fprintf( f, "%s", humanize( *v ) );
	else
		return fprintf( f, "%lli", *v );
}

int str_percent( FILE *f, void *p1, struct strmod_s *m )
{
	double *v = p1;
	return fprintf( f, "%6.2f", *v *100 );
}

struct strpart_s *straddpart( struct strpart_s *p, strfnc *fnc, void *p1, struct strmod_s *m )
{
	struct strpart_s *s;
	struct strpart_s *new;

	s = p;
	while ( s && s->next ) {
		s = s->next;
	}

	new = malloc( sizeof( struct strpart_s ) );
	new->fnc  = fnc;
	new->p1   = p1;
	if ( m ) {
		new->m    = malloc( sizeof( struct strmod_s ) );
		assert( m );
		memcpy( new->m, m, sizeof( struct strmod_s ) );
	} else
		new->m = m;
	new->next = NULL;

	if ( s ) {
		s->next = new;
		return p;
	} else {
		return new;
	}
}

int pprintf( FILE *f, struct strpart_s *p )
{
	int w = 0;

	while ( p ) {
		w += p->fnc( f, p->p1, p->m );
		p = p->next;
	}
	return w;
}

