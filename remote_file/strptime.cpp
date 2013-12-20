/*
* Copyright (c) 1999 Kungliga Tekniska Hogskolan
* (Royal Institute of Technology, Stockholm, Sweden).
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions
* are met:
*
* 1. Redistributions of source code must retain the above copyright
*    notice, this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* 3. Neither the name of KTH nor the names of its contributors may be
*    used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY KTH AND ITS CONTRIBUTORS ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL KTH OR ITS CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. */

#include "strptime.h"

#ifdef WIN32


#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

static const char *abb_weekdays[] = {
	"Sun",
	"Mon",
	"Tue",
	"Wed",
	"Thu",
	"Fri",
	"Sat",
	"",
};

static const char *full_weekdays[] = {
	"Sunday",
	"Monday",
	"Tuesday",
	"Wednesday",
	"Thursday",
	"Friday",
	"Saturday",
	"",
};

static const char *abb_month[] = {
	"Jan",
	"Feb",
	"Mar",
	"Apr",
	"May",
	"Jun",
	"Jul",
	"Aug",
	"Sep",
	"Oct",
	"Nov",
	"Dec",
	"",
};

static const char *full_month[] = {
	"January",
	"February",
	"Mars",
	"April",
	"May",
	"June",
	"July",
	"August",
	"September",
	"October",
	"November",
	"December",
	"",
};

static const char *ampm[] = {
	"am",
	"pm",
	"",
};

int strncasecmp(const char *s1, const char *s2, size_t n);

/*
* Try to match `*buf' to one of the strings in `strs'.  Return the
* index of the matching string (or -1 if none).  Also advance buf.
*/

static int
match_string(const char **buf, const char **strs) {
	int i = 0;
	for (i = 0; strs[i] != NULL; ++i) {
		int len = (int)strlen(strs[i]);
		if (strncasecmp(*buf, strs[i], len) == 0) {
			*buf += len;
			return i;
		}
	}
	return -1;
}

/*
* tm_year is relative this year */

const int tm_year_base = 1900;

/*
* Return TRUE iff `year' was a leap year.
*/

static int
is_leap_year(int year) {
	return (year % 4) == 0 && ((year % 100) != 0 || (year % 400) == 0);
}

/*
* Return the weekday [0,6] (0 = Sunday) of the first day of `year'
*/

static int
first_day(int year) {
	int ret = 4;

	for (; year > 1970; --year)
		ret = (ret + 365 + is_leap_year(year) ? 1 : 0) % 7;
	return ret;
}

/*
* Set `timeptr' given `wnum' (week number [0, 53])
*/

static void
set_week_number_sun(struct tm *timeptr, int wnum) {
	int fday = first_day(timeptr->tm_year + tm_year_base);

	timeptr->tm_yday = wnum * 7 + timeptr->tm_wday - fday;
	if (timeptr->tm_yday < 0) {
		timeptr->tm_wday = fday;
		timeptr->tm_yday = 0;
	}
}

/*
* Set `timeptr' given `wnum' (week number [0, 53])
*/

static void
set_week_number_mon(struct tm *timeptr, int wnum) {
	int fday = (first_day(timeptr->tm_year + tm_year_base) + 6) % 7;

	timeptr->tm_yday = wnum * 7 + (timeptr->tm_wday + 6) % 7 - fday;
	if (timeptr->tm_yday < 0) {
		timeptr->tm_wday = (fday + 1) % 7;
		timeptr->tm_yday = 0;
	}
}

/*
* Set `timeptr' given `wnum' (week number [0, 53])
*/

static void
set_week_number_mon4(struct tm *timeptr, int wnum) {
	int fday = (first_day(timeptr->tm_year + tm_year_base) + 6) % 7;
	int offset = 0;

	if (fday < 4)
		offset += 7;

	timeptr->tm_yday = offset + (wnum - 1) * 7 + timeptr->tm_wday - fday;
	if (timeptr->tm_yday < 0) {
		timeptr->tm_wday = fday;
		timeptr->tm_yday = 0;
	}
}

/*
*
*/

char *
strptime(const char *buf, const char *fmt, struct tm *timeptr) {
	char c;

	for (; (c = *fmt) != '\0'; ++fmt) {
		char *s;
		int ret;

		if (isspace(c)) {
			while (isspace(*buf))
				++buf;
		}
		else if (c == '%' && fmt[1] != '\0') {
			c = *++fmt;
			if (c == 'E' || c == 'O')
				c = *++fmt;
			switch (c) {
			case 'A':
				ret = match_string(&buf, full_weekdays);
				if (ret < 0)
					return NULL;
				timeptr->tm_wday = ret;
				break;
			case 'a':
				ret = match_string(&buf, abb_weekdays);
				if (ret < 0)
					return NULL;
				timeptr->tm_wday = ret;
				break;
			case 'B':
				ret = match_string(&buf, full_month);
				if (ret < 0)
					return NULL;
				timeptr->tm_mon = ret;
				break;
			case 'b':
			case 'h':
				ret = match_string(&buf, abb_month);
				if (ret < 0)
					return NULL;
				timeptr->tm_mon = ret;
				break;
			case 'C':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_year = (ret * 100) - tm_year_base;
				buf = s;
				break;
			case 'c':
				abort();
			case 'D': /* %m/%d/%y */
				s = strptime(buf, "%m/%d/%y", timeptr);
				if (s == NULL)
					return NULL;
				buf = s;
				break;
			case 'd':
			case 'e':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_mday = ret;
				buf = s;
				break;
			case 'H':
			case 'k':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_hour = ret;
				buf = s;
				break;
			case 'I':
			case 'l':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				if (ret == 12)
					timeptr->tm_hour = 0;
				else
					timeptr->tm_hour = ret;
				buf = s;
				break;
			case 'j':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_yday = ret - 1;
				buf = s;
				break;
			case 'm':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_mon = ret - 1;
				buf = s;
				break;
			case 'M':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_min = ret;
				buf = s;
				break;
			case 'n':
				if (*buf == '\n')
					++buf;
				else
					return NULL;
				break;
			case 'p':
				ret = match_string(&buf, ampm);
				if (ret < 0)
					return NULL;
				if (timeptr->tm_hour == 0) {
					if (ret == 1)
						timeptr->tm_hour = 12;
				}
				else
					timeptr->tm_hour += 12;
				break;
			case 'r': /* %I:%M:%S %p */
				s = strptime(buf, "%I:%M:%S %p", timeptr);
				if (s == NULL)
					return NULL;
				buf = s;
				break;
			case 'R': /* %H:%M */
				s = strptime(buf, "%H:%M", timeptr);
				if (s == NULL)
					return NULL;
				buf = s;
				break;
			case 'S':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_sec = ret;
				buf = s;
				break;
			case 't':
				if (*buf == '\t')
					++buf;
				else
					return NULL;
				break;
			case 'T': /* %H:%M:%S */
			case 'X':
				s = strptime(buf, "%H:%M:%S", timeptr);
				if (s == NULL)
					return NULL;
				buf = s;
				break;
			case 'u':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_wday = ret - 1;
				buf = s;
				break;
			case 'w':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_wday = ret;
				buf = s;
				break;
			case 'U':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				set_week_number_sun(timeptr, ret);
				buf = s;
				break;
			case 'V':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				set_week_number_mon4(timeptr, ret);
				buf = s;
				break;
			case 'W':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				set_week_number_mon(timeptr, ret);
				buf = s;
				break;
			case 'x':
				s = strptime(buf, "%Y:%m:%d", timeptr);
				if (s == NULL)
					return NULL;
				buf = s;
				break;
			case 'y':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				if (ret < 70)
					timeptr->tm_year = 100 + ret;
				else
					timeptr->tm_year = ret;
				buf = s;
				break;
			case 'Y':
				ret = strtol(buf, &s, 10);
				if (s == buf)
					return NULL;
				timeptr->tm_year = ret - tm_year_base;
				buf = s;
				break;
			//case 'Z':
			//	abort();
			case '\0':
				--fmt;
				/* FALLTHROUGH */
			case '%':
				if (*buf == '%')
					++buf;
				else
					return NULL;
				break;
			default:
				if (*buf == '%' || *++buf == c)
					++buf;
				else
					return NULL;
				break;
			}
		}
		else {
			if (*buf == c)
				++buf;
			else
				return NULL;
		}
	}
	return (char *)buf;
}


int strncasecmp(const char *s1, const char *s2, size_t n)
{
	if (n == 0)
		return 0;

	while (n-- != 0 && tolower(*s1) == tolower(*s2))
	{
		if (n == 0 || *s1 == '\0' || *s2 == '\0')
			break;
		s1++;
		s2++;
	}

	return tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2);
}

#endif /* WIN32 */

#if 0

#include <ctype.h>
#include <string.h>
#include <time.h>


/*
* We do not implement alternate representations. However, we always
* check whether a given modifier is allowed for a certain conversion.
*/
#define ALT_E          0x01
#define ALT_O          0x02
//#define LEGAL_ALT(x)       { if (alt_format & ~(x)) return (0); }
#define LEGAL_ALT(x)       { ; }
#define TM_YEAR_BASE   (1970)

static   int conv_num(const char **, int *, int, int);
static int strncasecmp(char *s1, char *s2, size_t n);

static const char *day[7] = {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday"
};
static const char *abday[7] = {
	"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
};
static const char *mon[12] = {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
};
static const char *abmon[12] = {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};
static const char *am_pm[2] = {
	"AM", "PM"
};


char * strptime(const char *buf, const char *fmt, struct tm *tm)
{
	char c;
	const char *bp;
	size_t len = 0;
	int alt_format, i, split_year = 0;

	bp = buf;

	while ((c = *fmt) != '\0')
	{
		/* Clear `alternate' modifier prior to new conversion. */
		alt_format = 0;

		/* Eat up white-space. */
		if (isspace(c))
		{
			while (isspace(*bp))
				bp++;

			fmt++;
			continue;
		}

		if ((c = *fmt++) != '%')
			goto literal;


	again:        switch (c = *fmt++)
	{
	case '%': /* "%%" is converted to "%". */
		literal :
		if (c != *bp++)
			return (0);
		break;

		/*
		* "Alternative" modifiers. Just set the appropriate flag
		* and start over again.
		*/
	case 'E': /* "%E?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_E;
		goto again;

	case 'O': /* "%O?" alternative conversion modifier. */
		LEGAL_ALT(0);
		alt_format |= ALT_O;
		goto again;

		/*
		* "Complex" conversion rules, implemented through recursion.
		*/
	case 'c': /* Date and time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%x %X", tm)))
			return (0);
		break;

	case 'D': /* The date as "%m/%d/%y". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

	case 'R': /* The time as "%H:%M". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M", tm)))
			return (0);
		break;

	case 'r': /* The time in 12-hour clock representation. */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%I:%M:%S %p", tm)))
			return (0);
		break;

	case 'T': /* The time as "%H:%M:%S". */
		LEGAL_ALT(0);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'X': /* The time, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%H:%M:%S", tm)))
			return (0);
		break;

	case 'x': /* The date, using the locale's format. */
		LEGAL_ALT(ALT_E);
		if (!(bp = strptime(bp, "%m/%d/%y", tm)))
			return (0);
		break;

		/*
		* "Elementary" conversion rules.
		*/
	case 'A': /* The day of week, using the locale's form. */
	case 'a':
		LEGAL_ALT(0);
		for (i = 0; i < 7; i++)
		{
			/* Full name. */
			len = strlen(day[i]);
			if (strncasecmp((char *)(day[i]), (char *)bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abday[i]);
			if (strncasecmp((char *)(abday[i]), (char *)bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 7)
			return (0);

		tm->tm_wday = i;
		bp += len;
		break;

	case 'B': /* The month, using the locale's form. */
	case 'b':
	case 'h':
		LEGAL_ALT(0);
		for (i = 0; i < 12; i++)
		{
			/* Full name. */

			len = strlen(mon[i]);
			if (strncasecmp((char *)(mon[i]), (char *)bp, len) == 0)
				break;

			/* Abbreviated name. */
			len = strlen(abmon[i]);
			if (strncasecmp((char *)(abmon[i]), (char *)bp, len) == 0)
				break;
		}

		/* Nothing matched. */
		if (i == 12)
			return (0);

		tm->tm_mon = i;
		bp += len;
		break;

	case 'C': /* The century number. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year)
		{
			tm->tm_year = (tm->tm_year % 100) + (i * 100);
		}
		else {
			tm->tm_year = i * 100;
			split_year = 1;
		}
		break;

	case 'd': /* The day of month. */
	case 'e':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_mday, 1, 31)))
			return (0);
		break;

	case 'k': /* The hour (24-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'H':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 0, 23)))
			return (0);
		break;

	case 'l': /* The hour (12-hour clock representation). */
		LEGAL_ALT(0);
		/* FALLTHROUGH */
	case 'I':
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_hour, 1, 12)))
			return (0);
		if (tm->tm_hour == 12)
			tm->tm_hour = 0;
		break;

	case 'j': /* The day of year. */
		LEGAL_ALT(0);
		if (!(conv_num(&bp, &i, 1, 366)))
			return (0);
		tm->tm_yday = i - 1;
		break;

	case 'M': /* The minute. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_min, 0, 59)))
			return (0);
		break;

	case 'm': /* The month. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &i, 1, 12)))
			return (0);
		tm->tm_mon = i - 1;
		break;

		//            case 'p': /* The locale's equivalent of AM/PM. */
		//                LEGAL_ALT(0);
		//                /* AM? */
		//                if (strcasecmp(am_pm[0], bp) == 0) 
		//                {
		//                    if (tm->tm_hour > 11)
		//                        return (0);
		//
		//                    bp += strlen(am_pm[0]);
		//                    break;
		//                }
		//                /* PM? */
		//                else if (strcasecmp(am_pm[1], bp) == 0) 
		//                {
		//                    if (tm->tm_hour > 11)
		//                        return (0);
		//
		//                    tm->tm_hour += 12;
		//                    bp += strlen(am_pm[1]);
		//                    break;
		//                }
		//
		//                /* Nothing matched. */
		//                return (0);

	case 'S': /* The seconds. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_sec, 0, 61)))
			return (0);
		break;

	case 'U': /* The week of year, beginning on sunday. */
	case 'W': /* The week of year, beginning on monday. */
		LEGAL_ALT(ALT_O);
		/*
		* XXX This is bogus, as we can not assume any valid
		* information present in the tm structure at this
		* point to calculate a real value, so just check the
		* range for now.
		*/
		if (!(conv_num(&bp, &i, 0, 53)))
			return (0);
		break;

	case 'w': /* The day of week, beginning on sunday. */
		LEGAL_ALT(ALT_O);
		if (!(conv_num(&bp, &tm->tm_wday, 0, 6)))
			return (0);
		break;

	case 'Y': /* The year. */
		LEGAL_ALT(ALT_E);
		if (!(conv_num(&bp, &i, 0, 9999)))
			return (0);

		tm->tm_year = i - TM_YEAR_BASE;
		break;

	case 'y': /* The year within 100 years of the epoch. */
		LEGAL_ALT(ALT_E | ALT_O);
		if (!(conv_num(&bp, &i, 0, 99)))
			return (0);

		if (split_year)
		{
			tm->tm_year = ((tm->tm_year / 100) * 100) + i;
			break;
		}
		split_year = 1;
		if (i <= 68)
			tm->tm_year = i + 2000 - TM_YEAR_BASE;
		else
			tm->tm_year = i + 1900 - TM_YEAR_BASE;
		break;

		/*
		* Miscellaneous conversions.
		*/
	case 'n': /* Any kind of white-space. */
	case 't':
		LEGAL_ALT(0);
		while (isspace(*bp))
			bp++;
		break;


	default: /* Unknown/unsupported conversion. */
		return (0);
	}


	}

	/* LINTED functional specification */
	return ((char *)bp);
}


static int conv_num(const char **buf, int *dest, int llim, int ulim)
{
	int result = 0;

	/* The limit also determines the number of valid digits. */
	int rulim = ulim;

	if (**buf < '0' || **buf > '9')
		return (0);

	do {
		result *= 10;
		result += *(*buf)++ - '0';
		rulim /= 10;
	} while ((result * 10 <= ulim) && rulim && **buf >= '0' && **buf <= '9');

	if (result < llim || result > ulim)
		return (0);

	*dest = result;
	return (1);
}

int strncasecmp(char *s1, char *s2, size_t n)
{
	if (n == 0)
		return 0;

	while (n-- != 0 && tolower(*s1) == tolower(*s2))
	{
		if (n == 0 || *s1 == '\0' || *s2 == '\0')
			break;
		s1++;
		s2++;
	}

	return tolower(*(unsigned char *)s1) - tolower(*(unsigned char *)s2);
}


#endif
