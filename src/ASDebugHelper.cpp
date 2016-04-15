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
 *      ASDebugHelper.cpp
 *
 * $CAS$
 */

#include "ASDebugHelper.hpp"
#include "ASLogger.hpp"

#include <stdio.h>

namespace CAS // C++ Application Server
{

// Recursion level
INT_32 ASDebugHelper::iLevel = 0;

//
// Constructor
//
ASDebugHelper::ASDebugHelper(ASLogger            * pILogger,
                             const STLW::string  & sIData): pLogger(pILogger),
                                                            sData(sIData),
                                                            iFlag(-1)
{
	STLW::string sTMP;
	for(INT_32 iI = 0 ; iI < iLevel; ++iI) { sTMP.append("    "); }
	sTMP.append(">> ");
	sTMP.append(sData);

	pLogger -> WriteLog(AS_LOG_INFO, sTMP.c_str(), sTMP.size());
	++iLevel;
}

//
// Write message to log
//
UINT_32 ASDebugHelper::WriteLog(CCHAR_P szMessage, ...)
{
	iFlag = 0;
	STLW::string sTMP;
	for(INT_32 iI = 0 ; iI < iLevel; ++iI) { sTMP.append("    "); }
	sTMP.append("|| ");
	sTMP.append(szMessage);

	va_list aArgList;
	va_start(aArgList, szMessage);
	const UINT_32 iRC = pLogger -> LogMessage(AS_LOG_INFO, sTMP.c_str(), aArgList);
	va_end(aArgList);

return iRC;
}

//
// Destructor
//
ASDebugHelper::~ASDebugHelper() throw()
{
	--iLevel;
	if (iFlag != -1)
	{
		STLW::string sTMP;
		for(INT_32 iI = 0 ; iI < iLevel; ++iI) { sTMP.append("    "); }
		sTMP.append("<< ");
		sTMP.append(sData);

		pLogger -> WriteLog(AS_LOG_INFO, sTMP.c_str(), sTMP.size());
	}
}

//
// Initialize reporter
//
void ASDebugHelper::InitReporter() { ASDebugHelper::iLevel = 0; }

} // namespace CAS
// End.
