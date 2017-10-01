/*************************************************/
/* profil.h - Definitions for execution profiler */
/*	(c) G. Wheeler 1988, 1989, 1990		 */
/* This code may be freely used, provided that	 */
/* this header is left intact and in place.	 */
/*************************************************/

#include <dos.h>

/*************************************************

The Execution Profiler is controlled by the
following macros:

        ZAP_UP()    - Speed up the clock
        ZAP_DOWN()  - Slow down the clock to 18.2 times per second

	PROFILE(f)  - Declares and initialises
			counters for function f.
	PROFSTART(f)- Begins an execution time
			measurement
	PROFSTOP(f) - Ends an execution time
			measurement.
	PROFSEE(f)  - Allows functions in other
			files to examine profile
	PROFSHOW(f) - Shows cumulative execution
			profile and measurement
			count.

These macros require the ANSI preprocessor
recommendations for 'stringising'(#) and
concatenating (##) macro arguments.

Typically, a profiled function will have the form:

int x(...)
{
	PROFILE(x);
	PROFSTART(x);
	....
	PROFSTOP(x);
}

while the main() function will have the form:

main()
{
	PROFSEE(x);
	ZAP_UP();
	...
	ZAP_DOWN();
	PROFSHOW(x);
}

Obviously the main() function may contain
as many PROFSEEs and PROFSHOWs as necessary,
but only one call each to ZAP_UP() and ZAP_DOWN().

The file containing the main() function should
also have a #define INCLUDE_PROFSHOW before the
#include "profil.h".

Written for TurboC.
**************************************************/

#ifdef PROFL_ON

#define CNT0MODE 0x40	/* Timer 0 mode	*/
#define CNT1MODE 0x41	/* Timer 1 mode	*/
#define CNT2MODE 0x42	/* Timer 2 mode	*/
#define CTCMODE	0x43	/* Timer modes	*/

#define MODE1	0x02	/* Hardware one-shot	*/
#define MODE2	0x04	/* Rate Generator	*/
#define MODE3	0x06	/* Square Wave Mode	*/
#define CNT0	0x00	/* Counter 0		*/
#define CNT1	0x40    /* Counter 1		*/
#define CNT2	0x80    /* Counter 2            */
#define LSBMSB	0x30	/* r/w lsb then msb	*/

/* DOS timer tick count location */

static unsigned long far *tick=(unsigned long far *)0x0000046CL;

#define RESOLUTION	1000l	/* 3 decimal places `accuracy' */
#define CLK_FREQ	(1000l)

/* Depending on the new clock frequency, set the frequency dividers.	*/
/* New values can be freely added. In some cases, 10000 is too high.	*/
/* I actually found 2350 to be a good value.				*/

#if CLK_FREQ==1000l
#define CLK_MSB		0x04
#define CLK_LSB		0xA9
#elif CLK_FREQ==10000l
#define CLK_MSB		0x00
#define CLK_LSB		0x77
#elif CLK_FREQ==2350l
#define CLK_MSB		0x02
#define CLK_LSB		0x00
#endif

#define ZAP_UP()        outp(CTCMODE,CNT0|LSBMSB|MODE2);outp(CNT0MODE,CLK_LSB);outp(CNT0MODE,CLK_MSB)
#define ZAP_DOWN()	outp(CTCMODE,CNT0|LSBMSB|MODE2);outp(CNT0MODE,0xFF);outp(CNT0MODE,0xFF)


#ifdef INCLUDE_PROFSHOW

void _prof_show(char *fname, long ticks, long cnt)
{
	long ave = ticks/cnt;
	printf("%-20.20s %5ld.%03ld    %3ld.%03ld    %8ld\n",\
				fname,ticks/CLK_FREQ,(RESOLUTION*(ticks%CLK_FREQ))/CLK_FREQ,\
				ave/CLK_FREQ,RESOLUTION*(ave%CLK_FREQ)/CLK_FREQ,cnt);
}

#endif

#define get_tick()	(*tick)
#define PROFILE(f)	long _etm_##f=0l, _tmp_##f, _cnt_##f=0
#define PROFSTART(f)	_cnt_##f++;_tmp_##f=get_tick()
#define PROFCSTART(f)	_cnt_##f++,_tmp_##f=get_tick()
#define PROFSTOP(f)	_etm_##f+=get_tick()-_tmp_##f
#define PROFSEE(f)	extern long _etm_##f, _cnt_##f
#define PROFHEAD()      printf("\n\nFunction             Total (s)    Average   Count\n=================================================\n\n")
#define PROFSHOW(f)	if (_cnt_##f) _prof_show(#f,_etm_##f, _cnt_##f)

#else

#define ZAP_UP()
#define ZAP_DOWN()
#define get_tick()
#define PROFILE(f)
#define PROFSTART(f)
#define PROFCSTART(f)
#define PROFSTOP(f)
#define PROFSEE(f)
#define PROFHEAD()
#define PROFSHOW(f)

#endif


