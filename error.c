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
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#include "error.h"

void fatal_err( int status, int errnum, const char *message, ... )
{
	va_list args;

	fflush( stdout );

	fprintf( stderr, "%s: ", program_name );

	va_start( args, message );
	vfprintf( stderr, message, args );
	va_end( args );

	if ( errnum )
		fprintf( stderr, ": %s", strerror( errnum ) );

	fprintf( stderr, "\n" );

	fflush( stderr );

	if ( status )
		exit( status );

	exit( 125 );
}

void fatal_msg( int status, const char *message, ... )
{
	va_list args;

	fflush( stdout );

	fprintf( stderr, "%s: ", program_name );

	va_start( args, message );
	vfprintf( stderr, message, args );
	va_end( args );

	fprintf( stderr, "\n" );

	fflush( stderr );

	if ( !status )
		exit( 125 );

	exit( status );
}
