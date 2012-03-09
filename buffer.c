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
#include "buffer.h"


int buf_init( struct buffer_s *buf, size_t size )
{
	buf->data = malloc( size );
	buf->size = size;
	buf->start = 0;
	buf->end = 0;
	return 0;
}

int buf_full( struct buffer_s *buf )
{
	return (buf->start + 1 == buf->end) || (buf->start == buf->size);
}

int buf_empty( struct buffer_s *buf )
{
	return (buf->start == buf->end);
}

size_t buf_space( struct buffer_s *buf )
{
	size_t space;
//	if ( b1_start + 1 == b1_end ) {
//		/* we're full */
//		fprintf( stderr, "may read ZERO bytes\n");
//		m = 0;
//	} else
	if ( buf->start < buf->end ) {
		/* the buffer is wrapped */
		space = buf->end - buf->start -1;
	} else {
		/* can read until end of buffer */
		space = buf->size - buf->start;
	}
	return space;
}

void *buf_space_offset( struct buffer_s *buf )
{
	return buf->data + buf->start;
}

void buf_reserve( struct buffer_s *buf, size_t num )
{
	buf->start += num;
}

size_t buf_data( struct buffer_s *buf )
{
	size_t m = 0;

	if ( buf->end < buf->start ) {
		/* buffer is straight */
		m = buf->start - buf->end;
	} else if ( buf->start < buf->end ) {
		/* buffer is wrapped. write till right end,
		 * wrap, write the rest next time */
		m = buf->size - buf->end;
	}
	return m;
}

void *buf_data_offset( struct buffer_s *buf )
{
	return buf->data + buf->end;
}

void buf_trash( struct buffer_s *buf, size_t num )
{
	buf->end += num;
	if ( buf->start == buf->size && buf->end != 0 )
		buf->start = 0;
	if ( buf->end == buf->size )
		buf->end = 0;
}
