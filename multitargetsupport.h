/*
***************************************************************************
*
* Author: Andras Fuchs
*
* Copyright (C) 2015 Andras Fuchs
*
* Email: andras.fuchs@gmail.com
*
***************************************************************************
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
***************************************************************************
*/

#ifndef MULTITARGETSUPPORT_H
#define MULTITARGETSUPPORT_H


#include <stdio.h>
#include <stdarg.h>


#if defined(__APPLE__) || defined(__MACH__) || defined(__APPLE_CC__) || defined(_MSC_VER)

#define fseeko fseek
#define ftello ftell
#define fopeno fopen

#else

#define fseeko fseeko64
#define ftello ftello64
#define fopeno fopen64

#endif

#if defined(Q_OS_WIN32) && !defined(_MSC_VER)

#define snprintf __mingw_snprintf
#define sprintf __mingw_sprintf
#define fprintf __mingw_fprintf

#endif


#if defined(_MSC_VER) && _MSC_VER < 1900

#define snprintf c99_snprintf
#define vsnprintf c99_vsnprintf

__inline int c99_vsnprintf(char *outBuf, size_t size, const char *format, va_list ap)
{
	int count = -1;

	if (size != 0)
		count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
	if (count == -1)
		count = _vscprintf(format, ap);

	return count;
}

__inline int c99_snprintf(char *outBuf, size_t size, const char *format, ...)
{
	int count;
	va_list ap;

	va_start(ap, format);
	count = c99_vsnprintf(outBuf, size, format, ap);
	va_end(ap);

	return count;
}

#endif

#if defined(_MSC_VER)

#include <direct.h>
#define mkdir _mkdir

/* -- Qt's <sys/time.h> polifill -- */

#include <Windows.h>

LARGE_INTEGER getFILETIMEoffset();

int gettimeofday(struct timeval *tp, void *tzp);

/* -- Qt's <sys/time.h> polifill -- */

#endif

#endif