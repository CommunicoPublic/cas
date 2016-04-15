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
 *      ASDataFilter.cpp
 *
 * $CAS$
 */
#include "ASDataFilter.hpp"
#include "ASDebugHelper.hpp"
#include "ASPCRE.hpp"

namespace CAS // C++ Application Server
{

//
// Check subnet with mask
//
INT_32 ASDataFilter::IsSubnet(CCHAR_P szSubnet)
{
	UINT_32 iOctetNum = 0;
	for (; iOctetNum < 4; ++iOctetNum)
	{
		UINT_32 iOctet = 0;
		for(;;)
		{
			if (*szSubnet >= '0' && *szSubnet <= '9') { iOctet = iOctet * 10 + (*szSubnet - '0'); }
			else { break; }
			++szSubnet;
		}

		if (iOctet > 255) { return -1; }

		if (*szSubnet != '.') { break; }
		++szSubnet;
	}

	// Subnet?
	if (*szSubnet == '/')
	{
		UINT_32 iBits = 0;
		++szSubnet;
		for(;;)
		{
			if (*szSubnet >= '0' && *szSubnet <= '9') { iBits = iBits * 10 + (*szSubnet - '0'); }
			else { break; }
			++szSubnet;
		}
		if (iBits > 32) { return -1; }
	}

	if (*szSubnet != '\0') { return -1; }

return 0;
}

//
// Check IP address
//
INT_32 ASDataFilter::IsIPAddress(CCHAR_P szIP)
{
	UINT_32 iOctetNum = 0;
	for (; iOctetNum < 4; ++iOctetNum)
	{
		UINT_32 iOctet = 0;
		for(;;)
		{
			if (*szIP >= '0' && *szIP <= '9') { iOctet = iOctet * 10 + (*szIP - '0'); }
			else { break; }
			++szIP;
		}

		if (iOctet > 255) { return -1; }

		if (*szIP != '.') { break; }
		++szIP;
	}

	if (iOctetNum != 3) { return -1; }

return 0;
}

//
// Check IP belongs to specified subnet
//
INT_32 ASDataFilter::InSubnetRange(CCHAR_P szIP, CCHAR_P szSubnet)
{
	UINT_32 iStartIP = 0;
	UINT_32 iEndIP   = 0;
	UINT_32 iIP      = 0;

	UINT_32 iOctetNum = 0;
	for (; iOctetNum < 4; ++iOctetNum)
	{
		UINT_32 iOctet = 0;
		for(;;)
		{
			if (*szSubnet >= '0' && *szSubnet <= '9') { iOctet = iOctet * 10 + (*szSubnet - '0'); }
			else { break; }
			++szSubnet;
		}

		if (iOctet > 255) { return -2; }
		iStartIP = (iStartIP << 8) + iOctet;

		if (*szSubnet != '.') { break; }
		++szSubnet;
	}
	iStartIP = iStartIP << (8 * (3 - iOctetNum));

	// Subnet?
	if (*szSubnet == '/')
	{
		UINT_32 iBits = 0;
		++szSubnet;
		for(;;)
		{
			if (*szSubnet >= '0' && *szSubnet <= '9') { iBits = iBits * 10 + (*szSubnet - '0'); }
			else { break; }
			++szSubnet;
		}
		if (iBits > 32) { return -2; }
		iEndIP = iStartIP | (0xFFFFFFFFUL >> iBits);
	}

	iOctetNum = 0;
	for (; iOctetNum < 4; ++iOctetNum)
	{
		UINT_32 iOctet = 0;
		for(;;)
		{
			if (*szIP >= '0' && *szIP <= '9') { iOctet = iOctet * 10 + (*szIP - '0'); }
			else { break; }
			++szIP;
		}

		if (iOctet > 255) { return -2; }
		iIP = (iIP << 8) + iOctet;

		if (*szIP != '.') { break; }
		++szIP;
	}

	if (iOctetNum != 3) { return -2; }

	if (iIP >= iStartIP && iIP <= iEndIP) { return 0; }

return -1;
}

//
// Check integer value
//
INT_32 ASDataFilter::IsInteger(CCHAR_P szValue)
{
	for (;;)
	{
		if (!(*szValue >= '0' && *szValue <= '9')) { break; }

		++szValue;
	}
	if (*szValue == '\0') { return 0; }

return -1;
}

//
// Check floating point value
//
INT_32 ASDataFilter::IsFloat(CCHAR_P szValue)
{
	;;
return -1;
}

//
// Check floating point value
//
INT_32 ASDataFilter::MatchRegexp(CCHAR_P szRE, CCHAR_P szIP)
{
	try
	{
		PCRE oPCRE(szRE);

		// Execure regular expression
		INT_32 iMatchNum = oPCRE.Exec(szIP, strlen(szIP));

		// No matches
		if (iMatchNum == -1 || iMatchNum == 0) { return -1; }
		return 0;
	}
	catch(...) { ;; }

return -1;
}

} // namespace CAS
// End.
