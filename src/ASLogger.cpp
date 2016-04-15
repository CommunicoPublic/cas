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
 *      ASLogger.cpp
 *
 * $CAS$
 */
#include "ASLogger.hpp"

#include <stdio.h>

namespace CAS // C++ Application Server
{

//
// Constructor
//
ASLogger::ASLogger(const UINT_32  iIBasePriority): iBasePriority(iIBasePriority) { ;; }

//
// Change base priority
//
void ASLogger::SetPriority(const UINT_32  iNewPriority) { iBasePriority = iNewPriority; }

//
// Log message with formatted parameters
//
INT_32 ASLogger::LogMessage(const UINT_32  iPriority,
                            CCHAR_P        szFormat, ...)
{
	va_list aArgList;
	va_start(aArgList, szFormat);
	INT_32 iRC = LogMessage(iPriority, szFormat, aArgList);
	va_end(aArgList);

return iRC;
}

//
// Log message with formatted parameters
//
INT_32 ASLogger::WriteLog(const UINT_32        iPriority,
                         const STLW::string  & sMessage)
{
	return WriteLog(iPriority, sMessage.c_str(), sMessage.size());
}

//
// Log message with formatted parameters
//
INT_32 ASLogger::LogMessage(const UINT_32  iPriority,
                            CCHAR_P        szFormat,
                            va_list        aArgList)
{
	// Message not logged, incorrect priority level
	if (iPriority > iBasePriority) { return -1; }

	{
		va_list vaTMP;
		va_copy(vaTMP, aArgList);

		CHAR_8 szStaticBuffer[C_START_MESSAGE_LEN];
		// Format value
		const INT_32 iFmtLen = vsnprintf(szStaticBuffer, C_START_MESSAGE_LEN - 1, szFormat, vaTMP);
		if (iFmtLen < C_START_MESSAGE_LEN)
		{
			// Write to logger
			WriteLog(iPriority, szStaticBuffer, iFmtLen);
			return 0;
		}
	}

	INT_32 iResultLen = C_START_MESSAGE_LEN;
	CHAR_P szResult   = NULL;
	for(;;)
	{
		va_list vaTMP;
		va_copy(vaTMP, aArgList);

		// TODO: Improve performance
		iResultLen <<= 1;
		szResult = (CHAR_P)malloc(iResultLen);
		// Format value
		const INT_32 iFmtLen = vsnprintf(szResult, iResultLen - 1, szFormat, vaTMP);
		if (iFmtLen < iResultLen)
		{
			// Write to logger
			WriteLog(iPriority, szResult, iFmtLen);
			free(szResult);
			return 0;
		}

		free(szResult);
	}
}

//
//log message with formatted parameters and AS_LOG_EMERG priority
//
INT_32 ASLogger::Emerg(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_EMERG, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_ALERT priority
//
INT_32 ASLogger::Alert(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_ALERT, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_CRIT priority
//
INT_32 ASLogger::Crit(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_CRIT, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_ERR priority
//
INT_32 ASLogger::Err(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_ERR, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_ERR priority
//
INT_32 ASLogger::Error(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_ERR, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_WARNING priority
//
INT_32 ASLogger::Warn(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_WARNING, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_WARNING priority
//
INT_32 ASLogger::Warning(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_WARNING, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_NOTICE priority
//
INT_32 ASLogger::Notice(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_NOTICE, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_INFO priority
//
INT_32 ASLogger::Info(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_INFO, szFormat, aTMP); va_end(aTMP); return iRC; }

//
//log message with formatted parameters and AS_LOG_DEBUG priority
//
INT_32 ASLogger::Debug(CCHAR_P szFormat, ...) { va_list aTMP; va_start(aTMP, szFormat); INT_32 iRC = LogMessage(AS_LOG_DEBUG, szFormat, aTMP); va_end(aTMP); return iRC; }

//
// A destructor
//
ASLogger::~ASLogger() throw()
{
	;;
}

} // namespace CAS
// End.
