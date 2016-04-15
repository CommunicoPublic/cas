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
 *      ASPoolTest.cpp
 *
 * $CAS$
 */

// C++ Includes
#include <ASPCRE.hpp>
#include <CDT.hpp>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace CAS; // C++ Application Server
using namespace CTPP;


int main(int argc, char ** argv)
{
	CHAR_8 szBuffer[1024 + 1];
	if (argc != 3) { fprintf(stderr, "usage: %s regexp string\n", argv[0]); return EX_USAGE; }

	try
	{
		PCRE oPCRE(argv[1]);
		CDT oURIComponets(CDT::HASH_VAL);

		STLW::string sURICheck(argv[2]);

		// Execure regular expression
		INT_32 iMatchNum = oPCRE.Exec(sURICheck.c_str(), sURICheck.size());

		// No matches
		if (iMatchNum == -1 || iMatchNum == 0)
		{
			fprintf(stderr, "No matches\n");
			return EX_OK;
		}

		PCRE::Match oMatch;
		for(INT_32 iPos = 0; iPos < iMatchNum; ++iPos)
		{
			oMatch = oPCRE.GetMatch(iPos);

			if (oMatch.match_start != -1 && oMatch.match_end != -1)
			{
				snprintf(szBuffer, 1024, "$%d", iPos);
				oURIComponets[szBuffer] = PCRE::ExtractMatch(sURICheck, oMatch);
			}
		}
		oMatch = oPCRE.PreMatch();
		if (oMatch.match_start != -1) { oURIComponets["prematch"]  = PCRE::ExtractMatch(sURICheck, oMatch); }

		oMatch = oPCRE.PostMatch();
		if (oMatch.match_start != -1) { oURIComponets["postmatch"] = PCRE::ExtractMatch(sURICheck, oMatch); }

		fprintf(stderr, "Result: %s", oURIComponets.RecursiveDump().c_str());
	}
	catch(CTPP::CTPPLogicError &e) { fprintf(stderr, "ERROR: %s\n", e.what());   }
	catch(...)                     { fprintf(stderr, "ERROR: Unknown error!\n"); }

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

return EX_OK;
}
// End.
