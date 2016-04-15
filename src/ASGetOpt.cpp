/*
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
 *      ASGetOpt.cpp
 *
 * $CAS$
 */
#include "ASGetOpt.hpp"

namespace CAS // C++ Application Server
{

//
// Compare strings.
//
CCHAR_P StrStr(CCHAR_P sX, CCHAR_P sY)
{
	while (*sX == *sY)
	{
		++sY; ++sX;
		if (*sX == '\0') { return sY; }
	}
	return NULL;
}

//
// Parse command-line options
//
INT_32 GetOpt(const Options * aOptions, UINT_32 & iArgNum, const UINT_32  iMaxArgNum, CHAR_P * aArgList, CCHAR_P & szArgValue)
{
	szArgValue = NULL;
	// Error?
	if (aOptions == NULL || aArgList == NULL || iArgNum >= iMaxArgNum || aArgList[iArgNum] == NULL) { return -1; }

	// Not an option
	CHAR_P szOption = aArgList[iArgNum];
	if (*szOption != '-') { return '?'; }

	// Empty option
	++szOption;
	if (*szOption == '\0') { return '?'; }

	// Short option
	if (*szOption != '-')
	{
		while (aOptions -> short_opt != '\0')
		{
			// Match found
			if (aOptions -> short_opt == *szOption)
			{
				switch (aOptions -> has_arg)
				{
					case C_NO_ARG:
						++szOption;
						if (*szOption != '\0') { return '?'; }
						++iArgNum;
						return aOptions -> short_opt;

					case C_HAS_ARG:
						++szOption;
						++iArgNum;
						// Get argument
						if (*szOption != '\0')
						{
							szArgValue = szOption;
							return aOptions -> short_opt;
						}
						// Get next argument
						else
						{
							// Error
							if (iArgNum == iMaxArgNum) { return '?'; }
							szArgValue = aArgList[iArgNum];
							++iArgNum;
							return aOptions -> short_opt;
						}

					case C_OPT_ARG:
						++iArgNum;
						szArgValue = ++szOption;
						return aOptions -> short_opt;

					// Error?
					default:
						return '?';
				}
			}

			// Check next option
			++aOptions;
		}
	}
	// Long Option
	else
	{
		// Empty option
		++szOption;
		if (*szOption == '\0') { return '?'; }

		// Check options
		while (aOptions -> short_opt != '\0')
		{
			CCHAR_P szSubstring = StrStr(aOptions -> long_opt, szOption);
			if (szSubstring != NULL)
			{
				switch (aOptions -> has_arg)
				{
					case C_NO_ARG:
						if (*szSubstring != '\0') { return '?'; }
						++iArgNum;
						return aOptions -> short_opt;

					case C_HAS_ARG:
						++szOption;
						// Get argument
						if (*szSubstring == '=')
						{
							++iArgNum;
							++szSubstring;
							szArgValue = szSubstring;
							return aOptions -> short_opt;
						}
						// Get next argument
						else
						{
							// Error
							if (iArgNum + 1 == iMaxArgNum) { return '?'; }
							++iArgNum;
							szArgValue = aArgList[iArgNum];
							++iArgNum;
							return aOptions -> short_opt;
						}

					case C_OPT_ARG:
						// Check option
						if (*szSubstring == '=')
						{
							++iArgNum;
							++szSubstring;
							szArgValue = szSubstring;
							return aOptions -> short_opt;
						}
					// Error?
					default:
						return '?';
				}
			}

			// Check next option
			++aOptions;
		}
	}

return '?';
}

} // namespace CAS
// End.
