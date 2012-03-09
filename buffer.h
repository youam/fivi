#ifndef BUFFER_H
#define BUFFER_H
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

struct buffer_s {
	void *data;
	size_t size;

	size_t start;
	size_t end;
};

int buf_init( struct buffer_s *buf, size_t size );
int buf_full( struct buffer_s *buf );
int buf_empty( struct buffer_s *buf );
size_t buf_space( struct buffer_s *buf );
void *buf_space_offset( struct buffer_s *buf );
size_t buf_data( struct buffer_s *buf );
void *buf_data_offset( struct buffer_s *buf );
void buf_reserve( struct buffer_s *buf, size_t num );
void buf_trash( struct buffer_s *buf, size_t num );

#endif
