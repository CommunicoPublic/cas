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
 *      ASSAPIUtil.cpp
 *
 * $CAS$
 */
// C++ Includes
#include "ASSAPIUtil.hpp"

#define C_ESCAPE_BUFFER_LEN 8192

//
// Store pair of key => value into CDT object
//
void StorePair(CTPP::CDT     & oRetVal,
               STLW::string  & sKey,
               STLW::string  & sValue)
{
	// Search value with same key
	CTPP::CDTIterator itoRetVal = oRetVal.Find(sKey);

	// If not found, store as scalar
	if (itoRetVal == oRetVal.End()) { oRetVal[sKey] = sValue; }
	// If found, store as vector of values
	else
	{
		// Push value into array
		if (itoRetVal -> second.GetType() == CTPP::CDT::ARRAY_VAL) { itoRetVal -> second.PushBack(sValue); }
		else
		{
			CTPP::CDT oTMP(CTPP::CDT::ARRAY_VAL);
			oTMP.PushBack(itoRetVal -> second);
			oTMP.PushBack(sValue);
			itoRetVal -> second = oTMP;
		}
	}
	// Clear buffers
	sKey.erase();
	sValue.erase();
}

//
// Unescape cookie
//
static void UnescapeCookie(CCHAR_P       & szString,
                           CHAR_8          chDelimiter,
                           STLW::string  & oRetVal,
                           CHAR_P          sBuffer)
{
	UINT_32 iBufferPointer = 0;
	UCHAR_8 ucSymbol       = 0;
	UCHAR_8 ucTMP          = 0;

	// Iterate through buffer
	while (*szString != '\0' && *szString != chDelimiter && *szString != ' ' && *szString != ';')
	{
		// Buffer overflow
		if (iBufferPointer == C_ESCAPE_BUFFER_LEN)
		{
			oRetVal.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}

		// Change '+' to space
		if      (*szString == '+') { sBuffer[iBufferPointer++] = ' '; }
		// Store all unescaped symbols
		else if (*szString != '%') { sBuffer[iBufferPointer++] = *szString; }
		else
		{
			++szString;

			ucSymbol = *szString;
			// Unescape correct sequence
			if      (ucSymbol >= 'A' && ucSymbol <= 'F') { ucTMP = ((ucSymbol - 'A' + 10) << 4); }
			else if (ucSymbol >= 'a' && ucSymbol <= 'f') { ucTMP = ((ucSymbol - 'a' + 10) << 4); }
			else if (ucSymbol >= '0' && ucSymbol <= '9') { ucTMP =  (ucSymbol - '0')      << 4;  }
			// Store '%' symbol to the buffer
			else
			{
				sBuffer[iBufferPointer++] = '%';
				continue;
			}

			++szString;
			// Unescape correct sequence
			if      (*szString >= 'A' && *szString <= 'F') { ucTMP += *szString - 'A' + 10; }
			else if (*szString >= 'a' && *szString <= 'f') { ucTMP += *szString - 'a' + 10; }
			else if (*szString >= '0' && *szString <= '9') { ucTMP += *szString - '0';      }
			// Store '%' and next symbol to the buffer
			else
			{
				sBuffer[iBufferPointer++] = '%';
				sBuffer[iBufferPointer++] = ucSymbol;
				continue;
			}

			// Okay, symbol successfully unescaped
			sBuffer[iBufferPointer++] = (UCHAR_8)ucTMP;
		}

		++szString;
	}

// Append buffer to result
oRetVal.append(sBuffer, iBufferPointer);
}

//
// Parse cookies foo=bar; baz=bar+baz/boo
//
void ParseCookies(CCHAR_P      szString,
                  CTPP::CDT  & oRetVal)
{
	if (szString == NULL) { return; }

	CHAR_8 sBuffer[C_ESCAPE_BUFFER_LEN + 4];

	STLW::string sKey;
	STLW::string sValue;

	for(;;)
	{
		// Skip spaces
		while (szString != '\0' && *szString == ' ') { ++szString; }
		// Return if EOL found
		if (*szString == '\0') { return; }

		// Parse key
		UnescapeCookie(szString, '=', sKey, sBuffer);

		// Skip spaces
		while (szString != '\0' && *szString == ' ') { ++szString; }
		// Store key and return
		if (*szString == '\0') { StorePair(oRetVal, sKey, sValue); return; }

		// Check '='
		if (*szString != '=') { return; }

		++szString;

		// Skip spaces
		while (szString != '\0' && *szString == ' ') { ++szString; }
		// Return if EOL found
		if (*szString == '\0') { StorePair(oRetVal, sKey, sValue); return; }

		// Parse value
		UnescapeCookie(szString, ';', sValue, sBuffer);

		// Skip spaces
		while (szString != '\0' && *szString == ' ') { ++szString; }

		// Store key and return
		if (*szString == '\0')
		{
			StorePair(oRetVal, sKey, sValue);
			return;
		}

		// check ';'
		if (*szString == ';') { StorePair(oRetVal, sKey, sValue); }
		++szString;

		sKey.erase();
		sValue.erase();
	}
}

// End.
