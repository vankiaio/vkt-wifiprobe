/**
 @file lib/time.h

 @brief Common Linux/POSIX time functions

 @par Copyright &copy; 2015 Mike Gore, GPL License

 @par You are free to use this code under the terms of GPL
   please retain a copy of this notice in any code you use it in.

This is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option)
any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/*
 * Update: Add get ISO string method by Huang Rui <vowstar@gmail.com>
 * 2016-01-30 10:55:58
 */

#ifndef __TIME_H__
#define __TIME_H__

#if defined(GLOBAL_DEBUG_ON)
#define TIME_DEBUG_ON
#endif
#if defined(TIME_HAL_DEBUG_ON)
#define TIME_DEBUG(format, ...) os_printf(format, ##__VA_ARGS__)
#else
#define TIME_DEBUG(format, ...)
#endif

#ifndef MEMSPACE
#define MEMSPACE ICACHE_FLASH_ATTR
#endif

///@brief type of EPOCH result.
typedef uint32_t time_t;

///@brief type of clockid_t.
typedef uint16_t clockid_t;

///@brief POSIX struct tm.
struct tm 
{
    int tm_sec;    /*<  Seconds.     [0-60] (1 leap second) */
    int tm_min;    /*<  Minutes.     [0-59] */ 
    int tm_hour;   /*<  Hours.       [0-23] */
    int tm_mday;   /*<  Day.         [1-31] */
    int tm_mon;    /*<  Month.       [0-11] */
    int tm_year;   /*<  Year - 1900. */
    int tm_wday;   /*<  Day of week. [0-6] */
    int tm_yday;   /*<  Days in year.[0-365] */
    int tm_isdst;  /*<  DST.         [-1/0/1] */
    int32_t tm_gmtoff; /*<  GMT offset in seconds [+/- 86400] */
};  
///@brief POSIX struct tm typedef.
typedef struct tm tm_t;

///@brief POSIX timeval.
struct timeval
{
    time_t       tv_sec;   /*< seconds */
    uint32_t     tv_usec;  /*< microseconds */
};
///@brief POSIX timeval typedef.
typedef struct timeval tv_t;

///@brief POSIX timezone.
struct timezone
{
    int tz_minuteswest;  /*< minutes west of Greenwich */
    int tz_dsttime;      /*< type of DST correction */
};
///@brief POSIX timezone typedef.
typedef struct timezone tz_t;

///@brief POSIX timespec.
struct timespec
{
    time_t   tv_sec;   /*< seconds */
    long     tv_nsec;  /*< nanoseconds */
};
///@brief POSIX timespec typedef.
typedef struct timespec ts_t;

/// @brief  System Clock Time
extern volatile ts_t __clock;

/// @brief  System Time Zone
extern tz_t __tzone;

/* lib/time.c */
MEMSPACE char *std_tm_wday_to_ascii ( int i );
MEMSPACE char *std_tm_mon_to_ascii ( int i );
MEMSPACE int std_time_to_tm ( time_t epoch , int32_t offset , tm_t *t );
MEMSPACE time_t std_timegm ( tm_t *t );
MEMSPACE char *std_asctime_r ( tm_t *t , char *buf );
MEMSPACE char *std_asctime ( tm_t *t );
MEMSPACE char *std_ctime_r ( time_t *t , char *buf );
MEMSPACE char *std_ctime ( time_t *tp );
MEMSPACE char *std_ctime_gm ( time_t *tp );
MEMSPACE tm_t *std_gmtime_r ( time_t *tp , tm_t *result );
MEMSPACE tm_t *std_gmtime ( time_t *tp );
MEMSPACE tm_t *std_localtime_r ( time_t *t , tm_t *result );
MEMSPACE tm_t *std_localtime ( time_t *tp );
MEMSPACE time_t std_mktime ( tm_t *t );
MEMSPACE int std_gettimezone ( tz_t *tz );
MEMSPACE int std_settimezone ( tz_t *tz );
MEMSPACE int std_gettimeofday ( tv_t *tv , tz_t *tz );
MEMSPACE time_t std_time ( time_t *t );
MEMSPACE int std_settimeofday ( tv_t *tv , tz_t *tz );
MEMSPACE void std_setclock ( uint32_t seconds , uint32_t us );
MEMSPACE int fromisostring (ts_t * ts, char *buf);
MEMSPACE int toisostring (char *buf, ts_t * ts);
MEMSPACE char *getisostring(void);
MEMSPACE int setisostring (char *buf);

#endif /* __TIME_H__ */
