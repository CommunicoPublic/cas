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
 *      ASPCRE.hpp
 *
 * $CAS$
 */
#include "ASPCRE.hpp"
#include "ASException.hpp"

#include <pcre.h>

namespace CAS // C++ Application Server
{

//
// Constructor
//
PCRE::PCRE(CCHAR_P sRE, const UINT_32  iMaxMatches): aResultVector(NULL)
{
	iMaxMatchesSize = iMaxMatches * 3;

	oPCRE = pcre_compile(sRE, 0, &szError, &iErrOffset, 0);

	if (oPCRE == NULL) { throw LogicError(szError); }

	aResultVector = new int[iMaxMatchesSize];
}

//
// Execute regular expression
//
INT_32 PCRE::Exec(CCHAR_P szString, const UINT_32  iIStringLength)
{
	iStringLength = iIStringLength;
	// Clear result vector
	memset(aResultVector, 0, iMaxMatchesSize * sizeof(int));
	iRc = pcre_exec(oPCRE, 0, szString, iStringLength, 0, 0, aResultVector, iMaxMatchesSize);

	if (iRc < 0)
	{
		// No matches, zero number of results
		if (iRc == PCRE_ERROR_NOMATCH) { return 0; }
		// Error occured
		return -1;
	}

return iRc;
}

//
// Get Pre-match
//
PCRE::Match PCRE::PreMatch()
{
	Match oMatch = { -1, -1};

	if (iRc < 0) { return oMatch; }

	oMatch.match_start = 0;
	oMatch.match_end   = aResultVector[0];

return oMatch;
}

//
// Get Post-match
//
PCRE::Match PCRE::PostMatch()
{
	Match oMatch = { -1, -1};

	if (iRc < 0) { return oMatch; }

	oMatch.match_start = aResultVector[1];
	oMatch.match_end   = iStringLength;

return oMatch;
}

//
// Get match by number
//
PCRE::Match PCRE::GetMatch(const UINT_32  iMatchNum)
{
	Match oMatch = { -1, -1};

	if (iRc < 0 || int(iMatchNum) > iRc) { return oMatch; }

	oMatch.match_start = aResultVector[iMatchNum * 2];
	oMatch.match_end   = aResultVector[iMatchNum * 2 + 1];

return oMatch;
}

//
// Destructor
//
PCRE::~PCRE() throw()
{
	free(oPCRE);
	delete [] aResultVector;
}

//
// Extract match from source string
//
STLW::string PCRE::ExtractMatch(const STLW::string & sText, const Match & oMatch)
{
	if (oMatch.match_start == -1 || oMatch.match_end == -1) { return ""; }

return STLW::string(sText, oMatch.match_start, oMatch.match_end - oMatch.match_start);
}

} // namespace CAS
// End.
