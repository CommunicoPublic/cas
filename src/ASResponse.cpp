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
 *      ASResponse.cpp
 *
 * $CAS$
 */

#include "ASResponse.hpp"

#include <sys/time.h>
#include <stdio.h>
#include <time.h>

namespace CAS // C++ Application Server
{
//
// Constructor
//
ASResponse::ASResponse(): iHTTPResponseCode(200),
                          iContentLength(0)
{
	;;
}

//
// Set HTTP response code
//
void ASResponse::SetHTTPCode(const UINT_32  iCode) { iHTTPResponseCode = iCode; }

//
// Set HTTP status line
//
void ASResponse::SetHTTPStatusLine(const STLW::string & sIStatusLine) { sStatusLine = sIStatusLine; }

//
// Get HTTP status line
//
STLW::string ASResponse::GetHTTPStatusLine() const { return sStatusLine; }

//
// Set cookie
void ASResponse::SetCookie(const STLW::string  & sName,
                           const STLW::string  & sValue,
                           const UINT_32         iExpires,
                           const STLW::string  & sDomain,
                           const STLW::string  & sPath,
                           const bool            bHTTPOnly,
                           const bool            bSecure)
{
	vCookies.push_back(ASCookie(sName, sValue, iExpires, sDomain, sPath, bHTTPOnly, bSecure));
}

//
// Set HTTP response header
//
void ASResponse::SetHeader(const STLW::string  & sName,
                           const STLW::string  & sValue)
{
	vHeaders.push_back(STLW::pair<STLW::string, STLW::string>(sName, sValue));
}

//
// Fire up no-cache flags
//
void ASResponse::SetNoCacheFlags()
{
	vHeaders.push_back(STLW::pair<STLW::string, STLW::string>("Pragma", "no-cache, no-store"));
	vHeaders.push_back(STLW::pair<STLW::string, STLW::string>("Cache-Control", "no-cache"));
	vHeaders.push_back(STLW::pair<STLW::string, STLW::string>("Expires", "Thu, 01 Jan 1970 00:00:01 GMT"));
}

//
// Set expiration time
//
void ASResponse::SetExpiresHeader(const UINT_64 iTimestamp)
{
	static CCHAR_P szMonthNames[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
	static CCHAR_P szWeekDNames[] = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

	struct tm oGMTime;
	tzset();

	time_t iTime = iTimestamp;
	gmtime_r(&iTime, &oGMTime);

	// RFC date format; as strftime '%a, %d %b %Y %T GMT'
	CHAR_8 szBuffer[128 + 1];
	snprintf(szBuffer, 128, "%s, %.2d %s %d %.2d:%.2d:%.2d GMT",
	                        szWeekDNames[oGMTime.tm_wday],
	                        oGMTime.tm_mday,
	                        szMonthNames[oGMTime.tm_mon],
	                        oGMTime.tm_year + 1900,
	                        oGMTime.tm_hour,
	                        oGMTime.tm_min,
	                        oGMTime.tm_sec);
	vHeaders.push_back(STLW::pair<STLW::string, STLW::string>("Expires", szBuffer));

	time_t iCurrentTS = time(NULL);
	snprintf(szBuffer, 128, "max-age=%d", INT_32(iTimestamp - iCurrentTS));
	vHeaders.push_back(STLW::pair<STLW::string, STLW::string>("Cache-Control", szBuffer));
}

//
// Get headers array
//
STLW::vector<STLW::pair<STLW::string, STLW::string> > & ASResponse::GetHeaders() { return vHeaders; }

//
// Get cookies array
//
STLW::vector<ASCookie> & ASResponse::GetCookies() { return vCookies; }

//
// Set content-type
//
STLW::string ASResponse::GetContentType() const { return sContentType; }

//
// Get content-type
//
void ASResponse::SetContentType(const STLW::string & sIContentType) { sContentType = sIContentType; }

} // namespace CAS
// End.
