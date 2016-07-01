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
 *      ASCookie.cpp
 *
 * $CAS$
 */
#include "ASCookie.hpp"

#include <stdio.h>
#include <time.h>

namespace CAS // C++ Application Server
{
#define C_COOKIE_BUF_LEN 8192
#define C_ESCAPE_BUFFER_LEN 8192

static const char sMonthNames[12][4] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" };
static const char aDayNames[7][4]    = { "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat" };

static STLW::string URLEscape(const STLW::string sData)
{
	static const CHAR_8 * szEscape = "0123456789ABCDEF";

	CHAR_8 sBuffer[C_ESCAPE_BUFFER_LEN + 1];

	STLW::string oCDTRetVal;

	UINT_32 iBufferPointer = 0;

	STLW::string::const_iterator itsData = sData.begin();
	while (itsData != sData.end())
	{
		UCHAR_8 chTMP = (UCHAR_8)(*itsData);

		if ((chTMP >= 'a' && chTMP <= 'z') ||
		    (chTMP >= 'A' && chTMP <= 'Z') ||
		    (chTMP >= '0' && chTMP <= '9') ||
		     chTMP == '/' || chTMP == '.' || chTMP == '-' || chTMP == '_')
		{
			sBuffer[iBufferPointer++] = *itsData;
		}
		else if (chTMP == ' ') { sBuffer[iBufferPointer++] = '+'; }
		else
		{
			if (iBufferPointer >= (C_ESCAPE_BUFFER_LEN - 3))
			{
				oCDTRetVal.append(sBuffer, iBufferPointer);
				iBufferPointer = 0;
			}
			sBuffer[iBufferPointer++] = '%';
			sBuffer[iBufferPointer++] = szEscape[((chTMP >> 4) & 0x0F)];
			sBuffer[iBufferPointer++] = szEscape[(chTMP & 0x0F)];
		}

		if (iBufferPointer == C_ESCAPE_BUFFER_LEN)
		{
			oCDTRetVal.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}
		++itsData;
	}

	if (iBufferPointer != 0) { oCDTRetVal.append(sBuffer, iBufferPointer); }

return oCDTRetVal;
}

//
// Constructor
//
ASCookie::ASCookie(const STLW::string  & sName,
                   const STLW::string  & sValue,
                   const UINT_64       & iExpires,
                   const STLW::string  & sDomain,
                   const STLW::string  & sPath,
                   const bool            bHTTPOnly,
                   const bool            bSecure): sCookieBuffer(NULL)
{
	sCookieBuffer = new CHAR_8[8192 + 1];

	INT_32 iPos = snprintf(sCookieBuffer, C_COOKIE_BUF_LEN, "%s=%s", URLEscape(sName).c_str(), URLEscape(sValue).c_str());

	if (iExpires != 0)
	{
		time_t iTime = iExpires;
		const struct tm * pTM = gmtime(&iTime);
		iPos += snprintf(sCookieBuffer + iPos, C_COOKIE_BUF_LEN - iPos, "; expires=%s, %.2d %s %d %.2d:%.2d:%.2d GMT",
		                                                   aDayNames[pTM -> tm_wday],
		                                                   pTM -> tm_mday,
		                                                   sMonthNames[pTM -> tm_mon],
		                                                   pTM -> tm_year + 1900,
		                                                   pTM -> tm_hour,
		                                                   pTM -> tm_min,
		                                                   pTM -> tm_sec);
	}

	if (!sPath.empty()) { iPos += snprintf(sCookieBuffer + iPos, C_COOKIE_BUF_LEN - iPos, "; path=%s", sPath.c_str()); }

	if (!sDomain.empty()) { iPos += snprintf(sCookieBuffer + iPos, C_COOKIE_BUF_LEN - iPos, "; domain=%s", sDomain.c_str()); }

	if (bHTTPOnly) { iPos += snprintf(sCookieBuffer + iPos, C_COOKIE_BUF_LEN - iPos, "; httponly"); }

	if (bSecure) { iPos += snprintf(sCookieBuffer + iPos, C_COOKIE_BUF_LEN - iPos, "; secure"); }
}

//
// Copy constructor
//
ASCookie::ASCookie(const ASCookie & oRhs)
{
	sCookieBuffer = new CHAR_8[8192 + 1];
	memcpy(sCookieBuffer, oRhs.sCookieBuffer, 8192 + 1);
}

//
// Assignment operator
//
ASCookie & ASCookie::operator=(const ASCookie & oRhs)
{
	if (this == &oRhs) { return *this; }

	memcpy(sCookieBuffer, oRhs.sCookieBuffer, 8192 + 1);

return *this;
}

//
// Get Cookie
//
CCHAR_P ASCookie::GetCookie() { return sCookieBuffer; }

//
// A destructor
//
ASCookie::~ASCookie() throw() { delete [] sCookieBuffer; }

} // namespace CAS
// End.
