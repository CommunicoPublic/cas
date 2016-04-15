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
 *      ASCookie.hpp
 *
 * $CAS$
 */
#ifndef _AS_COOKIE_HPP__
#define _AS_COOKIE_HPP__ 1

#include "ASTypes.hpp"
#include "STLString.hpp"

/**
  @file ASCookie.hpp
  @brief HTTP Cookie support
*/

namespace CAS // C++ Application Server
{

/**
  @class ASCookie ASCookie.hpp <ASCookie.hpp>
  @brief HTTP Cookie
*/
class ASCookie
{
public:
	/**
	  @brief Constructor
	  @param sName - Cookie name
	  @param sValue - Cookie value
	  @param iExpires - Cookie expiration time (unit time stamp)
	  @param sDomain - Cookie domain
	  @param sPath - Cookie path
	  @param bHTTPOnly - HTTP only flag
	  @param bSecure - Secure flag
	*/
	ASCookie(const STLW::string  & sName,
	         const STLW::string  & sValue,
	         const UINT_64       & iExpires = 0,
	         const STLW::string  & sDomain = "",
	         const STLW::string  & sPath = "/",
	         const bool            bHTTPOnly = false,
	         const bool            bSecure = false);

	/**
	  @brief Copy constructor
	  @param oRhs - object to copy
	*/
	ASCookie(const ASCookie & oRhs);

	/**
	  @brief Assignment operator
	  @param oRhs - object to copy
	*/
	ASCookie & operator=(const ASCookie & oRhs);

	/**
	  @brief Get cookie
	  @return Cookie string
	*/
	CCHAR_P GetCookie();

	/**
	  @brief A drestructor
	*/
	~ASCookie() throw();

private:
	/** Cookie body       */
	CHAR_P    sCookieBuffer;
};

} // namespace CAS
#endif // _AS_COOKIE_HPP__
// End.
