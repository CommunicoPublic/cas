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
 *      ASLocationURI.hpp
 *
 * $CAS$
 */
#ifndef _AS_LOCATION_URI_HPP__
#define _AS_LOCATION_URI_HPP__ 1

/**
  @file ASLocationURI.hpp
  @brief Application server location URI checker
*/

#include "ASTypes.hpp"
#include "STLString.hpp"

namespace CTPP
{
// FWD
class CDT;
}

namespace CAS // C++ Application Server
{
// FWD
class PCRE;

/**
  @class ASLocationURI ASLocationURI.hpp <ASLocationURI.hpp>
  @brief Application server location URI checker
*/
class ASLocationURI
{
public:
	/**
	  @enum eURIType ASLocationURI.hpp <ASLocationURI.hpp>
	  @brief Describes type of URI
	*/
	enum eURIType { PLAIN_URI, REGEXP_URI };

	/**
	  @brief Constructor
	  @param sIURI - URI as plaintext or Perl-compatible regular expression
	  @param oIURIType - URI type: "plain" or "regexp"
	*/
	ASLocationURI(const STLW::string & sIURI, const eURIType & oIURIType);

	/**
	  @brief Check URI
	  @param sURI - URI to check
	  @param oURIComponets - List of URI components
	  @return 0 - if URI matches, -1 - otherwise
	*/
	INT_32 CheckURI(const STLW::string  & sURI,
	                CTPP::CDT           & oURIComponets) const;

	/**
	  @brief Get original URI
	*/
	STLW::string GetURI() const;

	/**
	  @brief Get URI type
	*/
	eURIType GetURIType() const;

	/**
	  @brief A destructor
	*/
	~ASLocationURI() throw();
private:
	/** Reguilar expression */
	PCRE           * pPCRE;
	/** URI                 */
	STLW::string     sURI;
	/** URI type            */
	eURIType         oURIType;

	/**
	  @brief Default constructor
	*/
	ASLocationURI();

	/**
	  @brief Copy constructor
	*/
	ASLocationURI(const ASLocationURI & oRhs);

	/**
	  @brief Copy operator =
	*/
	ASLocationURI & operator=(const ASLocationURI & oRhs);
};

} // namespace CAS
#endif // _AS_LOCATION_URI_HPP__
// End.
