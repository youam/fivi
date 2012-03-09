/*
 * Copyright (c) 2005-2007 Uli Martens <uli@youam.net>
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

#include <fcntl.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <signal.h>
#include <getopt.h>

#include <glib.h>


#include "fileutils.h"

#include "error.h"
#include "buffer.h"
#include "strutils.h"

#define BUFSIZE		(64*1024)
#define FMT_REGFILE	"ELA: %E | %e  in: %hI @%hi (%P%% of %hS) out: %hO @%ho (%p%% | %hs)"
#define FMT_PIPE	"ELA: %E  in: %hI @%hi  out: %hO @%ho (%p%%)"

//FIXME: this isn't constant
char *program_name = "fivi";


struct fivi_s {
	int fd_i1;
	int fd_o1;
	int fd_i2;
	int fd_o2;

	off_t o_i1;
	off_t o_o1;
	off_t o_i2;
	off_t o_o2;

	GTimer *t;

	off_t size_in;

	int e_status;

	char **command;

	/* output controling stuff */

	char *fmtstr;
	struct strpart_s *fmt;

	char *exitstr;
	struct strpart_s *exitfmt;

	gdouble nextout;

	struct timeval deltat;

	/* rates and estimated values */

	off_t  val_p_rate_in;
	off_t  val_p_rate_out;
	double val_p_ratio;

	/* bytes read on the input side */

	/* input side file sizes */
	/*    val_p_size_in;  -- use o_i1    */
	off_t val_r_size_in;
	/*    val_t_size_in;  -- use size_in */

	/* processed percentage (wrt input size) */
	double val_p_progress;
	double val_r_progress;
	/*     val_t_progress;  -- uhm. 100% */
	
	/* output side file sizes */
	/*    val_p_size_out; -- use o_o2    */
	off_t val_r_size_out;
	off_t val_t_size_out;

	/* time since program start, remaining, total */
	double val_p_time;
	double val_r_time;
	double val_t_time;
};


static struct option long_options[] = {
	{"delay",	1, 0, 'd'},
	{"exit",	1, 0, 'e'},
	{"format",	1, 0, 'f'},
	{"help",	0, 0, 'h'},
	{0,		0, 0, 0}
};


void usage( FILE *f )
{
	fprintf( f, "Copyright (C) 2005-2007 by Uli Martens <uli@youam.net>\n" );
	fprintf( f, "\n" );
	fprintf( f, "Usage: %s [options] <command> [command options] < input > output\n", program_name );
	fprintf( f, "Options:\n" );
	fprintf( f, "  -d --delay=SECONDS   output progress information every SECONDS seconds (0.1)\n" );
	fprintf( f, "  -f --format=STRING   use STRING to format the progress information\n" );
	fprintf( f, "        default for regular files is\n" );
	fprintf( f, "        \"%s\"\n", FMT_REGFILE );
	fprintf( f, "        and\n" );
	fprintf( f, "        \"%s\"\n", FMT_PIPE );
	fprintf( f, "        for pipes\n" );
	fprintf( f, "  -e --exit=STRING     at exit, print information in this format.\n" );
	fprintf( f, "        defaults to --format's value\n" );
	fprintf( f, "  -h --help            print this help text\n" );
}

struct strpart_s *parse_fmt( struct fivi_s *b, char *fmt );
int getargs( struct fivi_s *fivi, int argc, char *argv[] )
{
	int c;

	while ( 1 ) {
		int option_index = 0;


		c = getopt_long( argc, argv, "+e:d:f:", long_options, &option_index );
		if ( c == -1 )
			break;

		switch ( c ) {
			case 'd':
				{
				char *x = optarg;
				long int s = 0;
				long int us = 0;

				while ( *x  &&  '0' <= *x && *x <= '9' ) {
					s = s*10 + (*x-'0');
					x++;
				}
				if ( *x == '.' ) {
					x++;
					if ( '0' <= *x && *x <= '9' ) {
						us = (*x-'0')*100;
						x++;
					} else {
						fprintf( stderr, "hey, i want some char after '.'!\n");
					}
					if ( *x != '\0' ) {
						fprintf( stderr, "less than 0.1s is not supported\n" );
					}
				}
				fivi->deltat.tv_sec  = s;
				fivi->deltat.tv_usec = us;
				}
				break;
			case 'e':
				asprintf( &fivi->exitstr, "\e[2K%s\n", optarg );
				if ( !fivi->exitstr ) {
					fatal_msg( 0, "strdup error\n" );
				}
				break;
			case 'f':
				fivi->fmtstr = strdup( optarg );
				if ( !fivi->fmtstr ) {
					fatal_msg( 0, "strdup error\n" );
				}
				break;
			case 'h':
				usage( stdout );
				exit(0);
			default:
				fprintf( stderr, "?? getopt returned character code 0%o ??\n", c );
				usage( stderr );
				assert( 1 == 2 );
		}
	}
	//fprintf( stderr, "debug: optind is %i, argc is %i\n", optind, argc) ;
	if ( optind >= argc ) {
		//XXX no command given
		return -1;
	}
	//FIXME copy this
	fivi->command = &argv[optind];
	return 0;
}


int do_status( int force, struct fivi_s *b )
{
	double tslice;

	b->val_p_time = g_timer_elapsed( b->t, NULL );

	if ( !force && b->val_p_time < b->nextout ) {
		return 0;
	}

	tslice = ( b->deltat.tv_sec + ((float)b->deltat.tv_usec/1000) );
	b->nextout = ( ( b->val_p_time / tslice ) +1 ) * tslice;

	b->val_p_ratio    = (float)b->o_o2 / b->o_i1;
	if ( b->size_in != -1 ) {
	b->val_p_progress = (float)b->o_i1 / b->size_in;
	b->val_r_progress = 1 - b->val_p_progress;
	b->val_t_time     = (b->val_p_time / b->val_p_progress );
	b->val_r_time     =  b->val_t_time - b->val_p_time;
	b->val_r_size_in  =  b->size_in - b->o_i1;
	b->val_t_size_out =  b->size_in * b->val_p_ratio;
	b->val_r_size_out =  b->val_t_size_out - b->o_o2;
	}
	b->val_p_rate_in  = (b->val_p_time == 0.0 ? 0 : (b->o_i1 / b->val_p_time) );
	b->val_p_rate_out = (b->val_p_time == 0.0 ? 0 : (b->o_o2 / b->val_p_time) );

	pprintf( stderr, b->fmt );

	return 1;
}


struct strpart_s *parse_fmt( struct fivi_s *b, char *fmt )
{
	/* XXX fmt MUST be available as long as this template is used */

	struct strmod_s mod;
	struct strpart_s *str = NULL;

	int can_est = ( b->size_in != -1 );

	while ( *fmt ) {
		switch ( *fmt ) {
			case '%':
				fmt++;
				strmod_clear( &mod );
				switch ( *fmt ) {
					case '\0':
						//FIXME
						assert( 1 == 2 );
						break;

					case 'h':
						mod.humanize = 1;
						fmt++;
						break;

					default:
						/* do nothing, this is no modifier, it's a command */
						while ( 0 ) {};
				}

				switch ( *fmt ) {
					case '%':
						str = straddpart( str, str_char,    fmt,                NULL );
						break;

					case 'T':
						if ( !can_est ) { str = straddpart( str, str_str, "n/a", NULL ); break; }
						str = straddpart( str, str_time,    &b->val_t_time,     &mod );
						break;
					case 'E':
						str = straddpart( str, str_time,    &b->val_p_time,     &mod );
						break;
					case 'e':
						if ( !can_est ) { str = straddpart( str, str_str, "n/a", NULL ); break; }
						str = straddpart( str, str_time,    &b->val_r_time,     &mod );
						break;
					case 'I':
						str = straddpart( str, str_off_t,   &b->o_i1,           &mod );
						break;
					case 'i':
						str = straddpart( str, str_off_t,   &b->val_p_rate_in,  &mod );
						str = straddpart( str, str_str,     "B/s",              NULL );
						break;
					case 'O':
						str = straddpart( str, str_off_t,   &b->o_o2,           &mod );
						break;
					case 'o':
						str = straddpart( str, str_off_t,   &b->val_p_rate_out, &mod );
						str = straddpart( str, str_str,     "B/s",              NULL );
						break;
					case 'S':
						if ( !can_est ) { str = straddpart( str, str_str, "n/a", NULL ); break; }
						str = straddpart( str, str_off_t,   &b->size_in,        &mod );
						break;
					case 'r':
						if ( !can_est ) { str = straddpart( str, str_str, "n/a", NULL ); break; }
						str = straddpart( str, str_off_t,   &b->val_r_size_in,  &mod );
						break;
					case 'R':
						if ( !can_est ) { str = straddpart( str, str_str, "n/a", NULL ); break; }
						str = straddpart( str, str_off_t,   &b->val_r_size_out, &mod );
						break;
					case 's':
						if ( !can_est ) { str = straddpart( str, str_str, "n/a", NULL ); break; }
						str = straddpart( str, str_off_t,   &b->val_t_size_out, &mod );
						break;
					case 'X':
						if ( !can_est ) { str = straddpart( str, str_str, "n/a", NULL ); break; }
						str = straddpart( str, str_percent, &b->val_r_progress, &mod );
						break;
					case 'P':
						if ( !can_est ) { str = straddpart( str, str_str, "n/a", NULL ); break; }
						str = straddpart( str, str_percent, &b->val_p_progress, &mod );
						break;
					case 'p':
						str = straddpart( str, str_percent, &b->val_p_ratio,    &mod );
						break;
					//case 'a':
					//	str = straddpart( str, str_int,     &b->e_status,       &mod );
					//	break;
					default:
						//FIXME design question: die or return NULL?
						//FIXME printable characters, only
						fatal_msg( 0, "unknown format directive %%%c", *fmt );
						//return NULL;
				}
				break;
			default:
				str = straddpart( str, str_char, fmt, NULL );
		}
		fmt++;
	}
	str = straddpart( str, str_str, "\e[K\r", NULL );
	return str;
}



#define closefd(FD)	( close( FD ), (FD) = -1 )

int dualcopy( struct fivi_s *b )
{
	int maxfd = -1;

	struct buffer_s buf1;
	struct buffer_s buf2;

	fcntl_flag_clr( b->fd_i1, O_NONBLOCK );
	fcntl_flag_set( b->fd_o1, O_NONBLOCK );
	fcntl_flag_clr( b->fd_i2, O_NONBLOCK );
	fcntl_flag_set( b->fd_o2, O_NONBLOCK );

	buf_init( &buf1, BUFSIZE );
	buf_init( &buf2, BUFSIZE );

	if ( maxfd < b->fd_i1 ) maxfd = b->fd_i1;
	if ( maxfd < b->fd_o1 ) maxfd = b->fd_o1;
	if ( maxfd < b->fd_i2 ) maxfd = b->fd_i2;
	if ( maxfd < b->fd_o2 ) maxfd = b->fd_o2;

	while ( b->fd_i1 != -1 || b->fd_o1 != -1 || b->fd_i2 != -1 || b->fd_o2 != -1 ) {
		fd_set rset;
		fd_set wset;
		int n;
		struct timeval tv;

		tv.tv_sec =  0;
		tv.tv_usec = 100;

		FD_ZERO( &rset );
		FD_ZERO( &wset );

		// check whether we can read on the input side
		if ( b->fd_i1 != -1 && !buf_full( &buf1 ) )
			FD_SET( b->fd_i1, &rset );

		// if the input side is done, close the reading pipe for the child
		if ( b->fd_i1 == -1 && buf_empty( &buf1 ) && b->fd_o1 != -1 )
			closefd( b->fd_o1 );

		// check whether we can write on the input side
		if ( b->fd_o1 != -1 && !buf_empty( &buf1 ) )
			FD_SET( b->fd_o1, &wset );

		// check whether we can read on the output side
		if ( b->fd_i2 != -1 && !buf_full( &buf2 ) )
			FD_SET( b->fd_i2, &rset );

		// if the output side is done, close the stdout pipe
		if ( b->fd_i2 == -1 && buf_empty( &buf2 ) && b->fd_o2 != -1 )
			closefd( b->fd_o2 );

		// check whether we have something to write to stdout
		if ( b->fd_o2 != -1 && !buf_empty( &buf2 ) )
			FD_SET( b->fd_o2, &wset );

		if ( -1 == b->fd_i1 && -1 == b->fd_o1 && -1 == b->fd_i2 && -1 == b->fd_o2 )
			// ok, all files closed; looks like we're done
			continue;

		//FIXME no output when child dies
		do_status( 0, b );

		n = select( maxfd+1, &rset, &wset, NULL, &tv );
		if ( n < 0 ) {
			switch ( errno ) {
				case EINTR:
					/* caught a signal, just loop again */
					errno = 0;
					continue;
				default:
					fatal_err( 125, errno, "select returned" );
			}
			errno = 0;
		}

		if ( b->fd_o1 != -1 && FD_ISSET( b->fd_o1, &wset ) ) {
			ssize_t p;

			//FIXME make sure there is data to write
			p = write( b->fd_o1, buf_data_offset( &buf1 ), buf_data( &buf1 ) );
			if ( p < 0 ) {
				switch ( errno ) {
					case EPIPE:
						fprintf( stderr, "DEBUG: caught EPIPE while trying to write to the filter\n" );
						// looks like our child died...
						//XXX race condition when we get the EPIPE and we've allready got all our input.
						// no way to pass the EPIPE through in that case
						if ( b->fd_i1 != -1 )
							closefd( b->fd_i1 );
						closefd( b->fd_o1 );
						break;
					case EAGAIN:
					case EINTR:
						/* go on, but skip the trashing */
						break;
					default:
						/* some different error. just die. */
						fatal_err( 125, errno, "write to child failed" );
				}
				errno = 0;
			} else {
				buf_trash( &buf1, p );
				b->o_o1 += p;
			}
		}

		if ( b->fd_i1 != -1 && FD_ISSET( b->fd_i1, &rset ) ) {
			ssize_t p;

			p = read( b->fd_i1, buf_space_offset( &buf1 ), buf_space( &buf1 ) );
			if ( p > 0 ) {
				b->o_i1 += p;
				buf_reserve( &buf1, p );
			} else if ( p == 0 ) {
				//XXX is this really EOF? what about network pipes?
				closefd( b->fd_i1 );
			} else {
				switch( errno ) {
					case EINTR:
						break;
					default:
						fatal_err( 125, errno, "read from input failed" );
				}
				errno = 0;
			}
		}

		//FIXME make sure again that FD_ISSET can only be set when something is in the buffer
		if ( b->fd_o2 != -1 && FD_ISSET( b->fd_o2, &wset ) ) {
			ssize_t p;

			p = write( b->fd_o2, buf_data_offset( &buf2 ), buf_data( &buf2 ) );
			if ( p < 0 ) {
				switch ( errno ) {
					case EPIPE:
						// output pipe is broken...
						//XXX race condition when we get the EPIPE and we've allready got all out input.
						// no way to pass the EPIPE through to the child in that case
						if ( b->fd_i2 != -1 )
							closefd( b->fd_i2 );
						closefd( b->fd_o2 );
						break;
					case EAGAIN:
					case EINTR:
						break;
					default:
						fatal_err( 125, errno, "write to output failed" );
				}
				errno = 0;
			} else {
				buf_trash( &buf2, p );
				b->o_o2 += p;
			}
		}

		if ( b->fd_i2 != -1 && FD_ISSET( b->fd_i2, &rset ) ) {
			ssize_t p;

			p = read( b->fd_i2, buf_space_offset( &buf2), buf_space( &buf2 ) );
			if ( p > 0 ) {
				b->o_i2 += p;
				buf_reserve( &buf2, p );
			} else if ( p == 0 ) {
				//XXX is this really EOF? what about network pipes?
				closefd( b->fd_i2 );
			} else {
				switch ( errno ) {
					case EINTR:
						break;
					default:
						fatal_err( 125, errno, "read from child failed" );
				}
				errno = 0;
			}
		}
	}

	do_status( 1, b );
	//FIXME
	return 0;
}



int run_command( struct fivi_s *fivi )
{
	int f_reader[2];
	int f_writer[2];
	pid_t pid;

	int status;
	int r;

	struct stat s;
	struct sigaction sig;

	if ( fivi->fd_i1 == -1 )
		fivi->fd_i1 = STDIN_FILENO;

	if ( fivi->fd_o2 == -1 )
		fivi->fd_o2 = STDOUT_FILENO;

	r = fstat( fivi->fd_i1, &s );
	assert( r == 0 );
	if ( S_ISREG( s.st_mode ) ) {
		fivi->size_in = s.st_size;
		if ( !fivi->fmtstr )
			fivi->fmtstr = FMT_REGFILE;

	} else {
		fivi->size_in = -1;
		if ( !fivi->fmtstr )
			fivi->fmtstr = FMT_PIPE;

	}

	fivi->fmt = parse_fmt( fivi, fivi->fmtstr );
	assert( fivi->fmt );

	if ( !fivi->exitstr ) {
		fivi->exitstr = "\n";
	}
	fivi->exitfmt = parse_fmt( fivi, fivi->exitstr );
	assert( fivi->exitfmt );

	r = pipe( f_reader );
	if ( r != 0 )
		fatal_err( 125, errno, "internal error" );

	r = pipe( f_writer );
	if ( r != 0 )
		fatal_err( 125, errno, "internal error" );

	pid = fork();
	if ( pid < 0 )
		/* fork failed */
		fatal_err( 125, errno, "internal error" );
	else if ( pid == 0 ) {
		/* we're the child */
		closefd( f_reader[1] ); /* close the writing half of the reader */
		closefd( f_writer[0] ); /* close the reading half of the writer */
		//XXX STDIN/STDOUT?!
		if ( f_reader[0] != STDIN_FILENO ) {
			if ( dup2( f_reader[0], STDIN_FILENO ) != STDIN_FILENO ) {
				fprintf( stderr, "can't dup2 f_reader to stdin\n" );
			}
			closefd( f_reader[0] );
		}
		if ( f_writer[1] != STDOUT_FILENO ) {
			if ( dup2( f_writer[1], STDOUT_FILENO ) != STDOUT_FILENO ) {
				fprintf( stderr, "can't dup2 f_writer to stdout\n" );
			}
			closefd( f_writer[1] );
		}
		execvp( fivi->command[0], fivi->command );

		// we can reach this point only if the exec() failed, so error
		// checking is only about _which_ error, not _if_ there is
		// one...

		fatal_err( errno == ENOENT ? 127 : 126, errno, "cannot run %s", fivi->command[0] );
	}

	closefd( f_reader[0] ); /* close the reading half of the reader */
	closefd( f_writer[1] ); /* close the writing half of the writer */

	fivi->fd_o1 = f_reader[1];
	fivi->fd_i2 = f_writer[0];

	sig.sa_handler = SIG_IGN;
	sigemptyset( &sig.sa_mask );
	sig.sa_flags = 0;
	r = sigaction( SIGPIPE, &sig, NULL );
	assert( r == 0 );

	dualcopy( fivi );

	if ( wait( &status ) != pid ) {
		//XXX childrens children?
		fprintf( stderr, "not my child dying\n" );
	}

	fivi->e_status = status;
	if ( fivi->exitfmt ) {
		pprintf( stderr, fivi->exitfmt );
	}
	if ( WIFEXITED( status ) ) {
		fprintf( stderr, "normal termination, exit status = %i\n", WEXITSTATUS( status ) );
		return WEXITSTATUS( status );
	} else if ( WIFSIGNALED( status ) ) {
		fprintf( stderr, "abnormal termination, signal number = %i, status = %i\n", WTERMSIG( status ), WEXITSTATUS( status ) );
		return 128 + WTERMSIG( status );
	} else
		assert( 1 == 2 );
}


int main( int argc, char *argv[] )
{
	int r;
	struct fivi_s fivi;

	fivi.fd_i1          = -1;
	fivi.fd_o2          = -1;
	fivi.fd_o1          = -1;
	fivi.fd_i2          = -1;

	fivi.o_i1           = 0;
	fivi.o_o1           = 0;
	fivi.o_i2           = 0;
	fivi.o_o2           = 0;

	fivi.e_status       = 0;

	fivi.t              = g_timer_new();
	fivi.nextout        = 0;

	fivi.size_in        = -1;

	fivi.fmtstr         = NULL;
	fivi.fmt            = NULL;

	fivi.exitstr        = NULL;
	fivi.exitfmt        = NULL;

	fivi.deltat.tv_sec  = 0;
	fivi.deltat.tv_usec = 100;

	fivi.val_p_time     = -1;
	fivi.val_p_rate_in  = -1;
	fivi.val_p_rate_out = -1;
	fivi.val_p_progress = -1;
	fivi.val_p_ratio    = -1;
	fivi.val_r_time     = -1;
	fivi.val_t_time     = -1;
	fivi.val_r_size_out = -1;
	fivi.val_t_size_out = -1;

	fivi.command        = NULL;

	r = getargs( &fivi, argc, argv );
	assert( r == 0 );

	r = run_command( &fivi );

	return r;
}
