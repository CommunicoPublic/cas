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
 *      ASLocationURI.cpp
 *
 * $CAS$
 */

#include "ASLocationURI.hpp"
#include "ASPCRE.hpp"
#include "ASUtil.hpp"

#include <stdio.h>

namespace CAS // C++ Application Server
{

//
// Constructor
//
ASLocationURI::ASLocationURI(const STLW::string  & sIURI,
                             const eURIType      & oIURIType): pPCRE(NULL),
                                                               sURI(sIURI),
                                                               oURIType(oIURIType)
{
	if (oURIType == REGEXP_URI) { pPCRE = new PCRE(sURI.c_str()); }
}

//
// Check URI
//
INT_32 ASLocationURI::CheckURI(const STLW::string  & sURICheck,
                               CTPP::CDT           & oURIComponets) const
{
	CHAR_8 szBuffer[1024 + 1];

	if (oURIType == PLAIN_URI)
	{
		if (sURI == sURICheck)
		{
			oURIComponets["$1"]          = sURICheck;
			return 0;
		}
		return -1;
	}

	// Execure regular expression
	INT_32 iMatchNum = pPCRE -> Exec(sURICheck.c_str(), sURICheck.size());

	// No matches
	if (iMatchNum == -1 || iMatchNum == 0) { return -1; }

	PCRE::Match oMatch;
	for(INT_32 iPos = 0; iPos < iMatchNum; ++iPos)
	{
		oMatch = pPCRE -> GetMatch(iPos);

		if (oMatch.match_start != -1 && oMatch.match_end != -1)
		{
			snprintf(szBuffer, 1024, "$%d", iPos);
			const STLW::string sMatch = PCRE::ExtractMatch(sURICheck, oMatch);
			oURIComponets[szBuffer] = sMatch;
		}
	}

	oMatch = pPCRE -> PreMatch();
	if (oMatch.match_start != -1)
	{
		const STLW::string sPreMatch = PCRE::ExtractMatch(sURICheck, oMatch);
		oURIComponets["prematch"]    = sPreMatch;
	}

	oMatch = pPCRE -> PostMatch();
	if (oMatch.match_start != -1)
	{
		const STLW::string sPostMatch = PCRE::ExtractMatch(sURICheck, oMatch);
		oURIComponets["postmatch"] = sPostMatch;
	}

return 0;
}

//
// Get original URI
//
STLW::string ASLocationURI::GetURI() const { return sURI; }

//
// Get URI type
//
ASLocationURI::eURIType ASLocationURI::GetURIType() const { return oURIType; }

//
// A destructor
//
ASLocationURI::~ASLocationURI() throw()
{
	if (pPCRE != NULL) { delete pPCRE; }
}

} // namespace CAS
// End.
