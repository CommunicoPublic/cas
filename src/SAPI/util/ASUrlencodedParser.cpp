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
 *      ASUrlencodedParser.cpp
 *
 * $CAS$
 */

// C++ Includes
#include "ASUrlencodedParser.hpp"

#include "ASSAPIUtil.hpp"

namespace CAS // C++ Application Server
{

#define C_ESCAPE_BUFFER_LEN 8192

// FSM states
#define  C_PARSE_KEY      0x00010000
#define  C_PARSE_VALUE    0x00020000

#define  C_PARSE_ESCAPED1 0x00000001
#define  C_PARSE_ESCAPED2 0x00000002

//
// Constructor
//
UrlencodedParser::UrlencodedParser(CTPP::CDT & oIData): oData(oIData),
                                                        iState(0)
{
	;;
}

//
// Escape value
//
void UrlencodedParser::EscapeValue(STLW::string   & sData,
                                   CHAR_8           chDelimiter,
                                   CHAR_8         * sBuffer,
                                   CCHAR_P        & szString,
                                   CCHAR_P          szStringEnd)
{
	UINT_32 iBufferPointer = 0;
	UCHAR_8 ucSymbol       = 0;
	UCHAR_8 ucTMP          = 0;
	UINT_32 iLeftBytes     = 0;

	// First state
	if ((iState & C_PARSE_ESCAPED1) != 0)
	{
		// Reset parser state
		iState &= 0xFFFF0000;
		goto PARSE_ESCAPED1;
	}

	// Second state
	if ((iState & C_PARSE_ESCAPED2) != 0)
	{
		// Reset parser state
		iState &= 0xFFFF0000;

		ucSymbol = chSymbol;

		if      (ucSymbol >= 'A' && ucSymbol <= 'F') { ucTMP = ((ucSymbol - 'A' + 10) << 4); }
		else if (ucSymbol >= 'a' && ucSymbol <= 'f') { ucTMP = ((ucSymbol - 'a' + 10) << 4); }
		else if (ucSymbol >= '0' && ucSymbol <= '9') { ucTMP =  (ucSymbol - '0')      << 4;  }
		else
		{
			sBuffer[iBufferPointer++] = '%';
			goto PARSE_PLAIN;
		}

		goto PARSE_ESCAPED2;
	}

PARSE_PLAIN:
	// Iterate through buffer
	while (szString != szStringEnd && *szString != chDelimiter && *szString != '&' && *szString != ';')
	{
		// Buffer overflow
		if (iBufferPointer == C_ESCAPE_BUFFER_LEN)
		{
			sData.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}

		// Change '+' to space
		if      (*szString == '+') { sBuffer[iBufferPointer++] = ' ';       iState &= 0xFFFF0000; }
		// Store all unescaped symbols
		else if (*szString != '%') { sBuffer[iBufferPointer++] = *szString; iState &= 0xFFFF0000; }
		else
		{
			iLeftBytes = szStringEnd - szString;
			// Unexpected end of string
			if (iLeftBytes > 2)
			{
				++szString;
PARSE_ESCAPED1:
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
PARSE_ESCAPED2:
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
			else
			{
				// First state
				if (iLeftBytes == 1)
				{
					iState |= C_PARSE_ESCAPED1;
					++szString;
				}
				// Second state
				else
				{
					iState |= C_PARSE_ESCAPED2;
					++szString;
					chSymbol = *szString;
					++szString;
				}
				break;
			}
		}

		++szString;
	}

// Append buffer to result
sData.append(sBuffer, iBufferPointer);
}

//
// Split string "key1=value1&key2=value2..."
//
void UrlencodedParser::ParseChunk(CCHAR_P  szString,
                                  CCHAR_P  szStringEnd)
{
	CHAR_8 sBuffer[C_ESCAPE_BUFFER_LEN + 4];

	// Jump to value parsing section
	if ((iState & C_PARSE_VALUE) != 0) { goto PARSE_VALUE; }

	// Split string
	for(;;)
	{
		// Skip void sequences
		while (szString != szStringEnd && (*szString == '&' || *szString == ';')) { ++szString; }

		// Reset state
		iState &= 0x0000FFFF;
		iState |= C_PARSE_KEY;

		// Escape key
		EscapeValue(sKey, '=', sBuffer, szString, szStringEnd);
		// End of stream, with current state "C_PARSE_KEY"
		if (szString == szStringEnd) { return; }

		// No value given, store empty key
		if (*szString == '&' || *szString == ';')
		{
			StorePair(oData, sKey, sValue);
			continue;
		}
		++szString;

PARSE_VALUE:
		// New state is "C_PARSE_VALUE"
		iState &= 0x0000FFFF;
		iState |= C_PARSE_VALUE;
		// Escape value
		EscapeValue(sValue, '&', sBuffer, szString, szStringEnd);

		// End of stream, with current state "C_PARSE_VALUE"
		if (szString == szStringEnd) { return; }

		// Store pair if '&' symbol found
		if (*szString == '&' || *szString == ';') { StorePair(oData, sKey, sValue); }
		++szString;
	}
}

//
// Get parser state
//
INT_32 UrlencodedParser::GetState() const
{
	return 0;
}

//
// End of parsing process
//
void UrlencodedParser::ParseDone()
{
	if (sKey.size()) { StorePair(oData, sKey, sValue); }
}

//
// Get error description
//
STLW::string UrlencodedParser::GetError() const { return ""; }

//
// A destructor
//
UrlencodedParser::~UrlencodedParser() throw()
{
	;;
}

} // namespace CAS
// End.
