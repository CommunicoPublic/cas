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
 *      ASResponse.hpp
 *
 * $CAS$
 */
#ifndef _AS_RESPONSE_HPP__
#define _AS_RESPONSE_HPP__ 1

/**
  @file ASResponse.hpp
  @brief Application server response object
*/
#include "ASCookie.hpp"

#include "STLVector.hpp"
#include "STLPair.hpp"

namespace CTPP
{
// FWD
class OutputCollector;
}

namespace CAS // C++ Application Server
{

/**
  @class ASResponse ASResponse.hpp <ASResponse.hpp>
  @brief Application server response object
*/
class ASResponse
{
public:
	/**
	  @brief Constructor
	*/
	ASResponse();

	/**
	  @brief Set HTTP response code
	  @param iCode - RFC 2616 HTTP recponse code
	*/
	void SetHTTPCode(const UINT_32  iCode);

	/**
	  @brief Set content length
	*/
	inline void SetContentLength(const UINT_64 iIContentLength) { iContentLength = iIContentLength; }

	/**
	  @brief get Content length
	*/
	inline UINT_64 GetContentLength() const { return iContentLength; }

	/**
	  @brief Set HTTP status line
	  @param sIStatusLine - status line to set
	*/
	void SetHTTPStatusLine(const STLW::string  & sIStatusLine);

	/**
	  @brief Set cookie
	  @param sName - Cookie name
	  @param sValue - Cookie value
	  @param iExpires - Cookie expiry time (unix time stamp)
	  @param sDomain - Cookie domain
	  @param sPath - Cookie path
	  @param bHTTPOnly - HTTP only flag
	  @param bSecure - Secure flag
	*/
	void SetCookie(const STLW::string  & sName,
	               const STLW::string  & sValue,
	               const UINT_32         iExpires = 0,
	               const STLW::string  & sDomain = "",
		       const STLW::string  & sPath = "/",
	               const bool            bHTTPOnly = false,
	               const bool            bSecure = false);

	/**
	  @brief Set HTTP response header
	  @param sName - header name
	  @param sValue - header value
	*/
	void SetHeader(const STLW::string  & sName,
	               const STLW::string  & sValue);

	/**
	  @brief Fire up no-cache flags
	*/
	void SetNoCacheFlags();

	/**
	  @brief Set expiration time
	  @param iTimestamp - Unix timestamp; expiration date
	*/
	void SetExpiresHeader(const UINT_64 iTimestamp);

	/**
	  @brief Get HTTP response code
	  @return HTTP response code
	*/
	inline UINT_32 GetHTTPResponseCode() const { return iHTTPResponseCode; }

	/**
	  @brief Get HTTP status line
	  @return HTTP status line or empty string if status line not set
	*/
	STLW::string GetHTTPStatusLine() const;

	/**
	  @brief Get headers array
	*/
	STLW::vector<STLW::pair<STLW::string, STLW::string> > & GetHeaders();

	/**
	  @brief Get cookies array
	*/
	STLW::vector<ASCookie> & GetCookies();

	/**
	  @brief Get content-type
	*/
	STLW::string GetContentType() const;

	/**
	  @brief Set content-type
	  @param sIContentType - content type
	*/
	void SetContentType(const STLW::string & sIContentType);

	// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 2XX codes

	/**
	  @brief HTTP OK
	*/
	inline void OK() { SetHTTPCode(200); }

//
//  TODO: Other 2XX codes
//

	// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 3XX codes

	/**
	  @brief HTTP Redirect
	  @param sURL - New location
	*/
	inline void Redirect(const STLW::string  & sURL)
	{
		SetHTTPCode(302);
		SetHeader("Location", sURL);
	}

	/**
	  @brief HTTP Permanent Redirect
	  @param sURL - New location
	*/
	inline void RedirectPermanent(const STLW::string  & sURL)
	{
		SetHTTPCode(301);
		SetHeader("Location", sURL);
	}

//
//  TODO: Other 3XX codes
//

	// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 4XX codes

	/**
	  @brief RFC 2617 Basic authentication
	*/
	inline void AuthenticateBasic(const STLW::string & sRealm)
	{
		SetHTTPCode(401);
		STLW::string sTMP("Basic realm=\"");
		sTMP.append(sRealm);
		sTMP.append("\"");
		SetHeader("WWW-Authenticate", sTMP);
	}

	/**
	  @brief HTTP Forbidden
	*/
	inline void Forbidden() { SetHTTPCode(403); }

	/**
	  @brief HTTP Document Not Found
	*/
	inline void NotFound() { SetHTTPCode(404); }

//
//  TODO: Other 4XX codes
//

	// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// 5XX codes

	/**
	  @brief HTTP Internal server error
	*/
	inline void InternalServerError() { SetHTTPCode(500); }

//
//  TODO: Other 5XX codes
//

private:
	// Does not exist
	ASResponse(const ASResponse  & oRhs);
	ASResponse& operator=(const ASResponse  & oRhs);

	/** HTTP response code         */
	UINT_32                                                iHTTPResponseCode;
	/** Content length             */
	UINT_64                                                iContentLength;
	/** HTTP status line           */
	STLW::string                                           sStatusLine;
	/** Content type               */
	STLW::string                                           sContentType;
	/** HTTP Headers               */
	STLW::vector<STLW::pair<STLW::string, STLW::string> >  vHeaders;
	/** HTTP cookies               */
	STLW::vector<ASCookie>                                 vCookies;
};

} // namespace CAS
#endif // _AS_RESPONSE_HPP__
// End.
