/*-
 * Copyright (c) 2005 - 2016 CAS Dev Team
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 4. Neither the name of the CAS Dev. Team nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *      FastCGILoggerSyslog.cpp
 *
 * $CAS$
 */
#include "FastCGILoggerSyslog.hpp"

#include <stdio.h>
#include <syslog.h>

#ifndef C_LOGGER_START_MESSAGE_LEN
#define C_LOGGER_START_MESSAGE_LEN 512
#endif // C_LOGGER_START_MESSAGE_LEN

// http://wiki.opencsw.org/porting-faq#toc18
#ifdef sun
    #define LOG_PERROR 0x00
    #define LOG_AUTHPRIV (10<<3)
    #define LOG_FTP      (11<<3)
#endif

namespace CAS // C++ Application Server
{
//
// A constructor
//
LoggerSyslog::LoggerSyslog(FILE       * oIF,
                           const bool   bITransferLog): F(oIF),
                                                        bTransferLog(bITransferLog)
{
	iOptions = LOG_NDELAY | LOG_PID;

	sIdent = "cas-fcgid";
	if (bTransferLog)
	{
		iFacility = LOG_LOCAL1;
	}
	else
	{
		iFacility = LOG_LOCAL0;
	}

	openlog(sIdent.c_str(), iOptions, iFacility);
}

//
// Re-open log
//
void LoggerSyslog::Reopen()
{
	closelog();
	openlog(sIdent.c_str(), iOptions, iFacility);
}

//
// Set client properties
//
void LoggerSyslog::SetProps(CCHAR_P  szClientIP,
                            INT_32   iIClientPort)
{
	sClientIP   = szClientIP;
	iClientPort = iIClientPort;
}

//
// Write message to log file
//
INT_32 LoggerSyslog::WriteLog(const UINT_32  iPriority,
                              CCHAR_P        szString,
                              const UINT_32  iStringLen)
{
	if (bTransferLog)
	{
		if (F != NULL) { fprintf(F, "%.*s\n", iStringLen, szString); }

		syslog(LOG_NOTICE, "%.*s", iStringLen, szString);
		return 0;
	}

	if (sClientIP.empty())
	{
		if (F != NULL) { fprintf(F, "[%s] %.*s\n", GetPriority(iPriority), iStringLen, szString); }

		syslog(iPriority, "[%s] %.*s", GetPriority(iPriority), iStringLen, szString);
		return 0;
	}

	if (iClientPort != 0)
	{
		if (F != NULL) { fprintf(F, "[%s] %s:%u: %.*s\n", GetPriority(iPriority), sClientIP.c_str(), iClientPort, iStringLen, szString); }

		syslog(iPriority, "[%s] %s:%u: %.*s", GetPriority(iPriority), sClientIP.c_str(), iClientPort, iStringLen, szString);
		return 0;
	}

	if (F != NULL) { fprintf(F, "[%s] %s: %.*s\n", GetPriority(iPriority), sClientIP.c_str(), iStringLen, szString); }

	syslog(iPriority, "[%s] %s: %.*s", GetPriority(iPriority), sClientIP.c_str(), iStringLen, szString);
return 0;
}

//
// Get logger priority
CCHAR_P LoggerSyslog::GetPriority(const INT_32 iPriority)
{
	switch(iPriority)
	{
		case AS_LOG_EMERG:   return "emerg";
		case AS_LOG_ALERT:   return "alert";
		case AS_LOG_CRIT:    return "crit";
		case AS_LOG_ERR:     return "error";
		case AS_LOG_WARNING: return "warn";
		case AS_LOG_NOTICE:  return "notice";
		case AS_LOG_INFO:    return "info";
		case AS_LOG_DEBUG:   return "debug";
	}
return "-";
}

//
// A destructor
//
LoggerSyslog::~LoggerSyslog() throw()
{
	closelog();
}

} // namespace CAS
// End.
