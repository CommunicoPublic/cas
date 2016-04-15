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
#ifndef _AS_PCRE_HPP__
#define _AS_PCRE_HPP__ 1

/**
  @file ASPCRE.hpp
  @brief PERL-compatible regular expressions support
*/

#include "ASTypes.hpp"
#include "STLString.hpp"

extern "C"
{
struct real_pcre;
typedef struct real_pcre pcre;
}

namespace CAS // C++ Application Server
{

/**
  @class PCRE PCRE.hpp <PCRE.hpp>
  @brief Perl-compatible regular expressions
*/
class PCRE
{
public:
	/**
	  @struct Match PCRE.hpp <PCRE.hpp>
	  @brief Match result
	*/
	struct Match
	{
		/** Match start position in string */
		INT_32    match_start;
		/** Match end position in string */
		INT_32    match_end;
	};

	/**
	  @brief Constructor
	  @param sRE - regular expression
	  @param iMaxMatches - max number of matches
	*/
	PCRE(CCHAR_P        sRE,
	     const UINT_32  iMaxMatches = 10);

	/**
	  @brief Execute regular expression
	  @param szString - string to execute expression in
	  @param iStringLength - string length
	*/
	INT_32 Exec(CCHAR_P        szString,
	            const UINT_32  iStringLength);

	/**
	  @brief The string preceding whatever was matched after successful execution of regular expression
	  @return Match object
	*/
	Match PreMatch();

	/**
	  @brief The string following whatever was matched after successful execution of regular expression
	  @return Match object
	*/
	Match PostMatch();

	/**
	  @brief The string matched by the last successful execution of regular expression
	  @return Match object
	*/
	Match GetMatch(const UINT_32  iMatchNum);

	/**
	  @brief A destructor
	*/
	~PCRE() throw();

	/**
	  @brief Extract match from source string
	*/
	static STLW::string ExtractMatch(const STLW::string  & sText,
	                                 const Match         & oMatch);

private:
	// Does not exist
	PCRE(const PCRE  & oRhs);
	PCRE& operator=(const PCRE  & oRhs);

	/** PCRE object            */
	pcre      * oPCRE;
	/** Error description      */
	CCHAR_P     szError;
	/** Error position         */
	int         iErrOffset;
	/** Result vector          */
	int       * aResultVector;
	/** Max. number of matches */
	int         iMaxMatchesSize;
	/** Number of matches      */
	int         iRc;
	/** String size            */
	int         iStringLength;
};

} // namespace CAS
#endif // _AS_PCRE_HPP__
// End.
