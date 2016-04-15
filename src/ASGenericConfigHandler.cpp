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
 *      ASGenericConfigHandler.cpp
 *
 * $CAS$
 */

// C++ Includes
#include "ASGenericConfigHandler.hpp"
#include "ASConfigErrorHandler.hpp"
#include "STLException.hpp"

#include <CDT.hpp>

#define C_ESCAPE_BUFFER_LEN 8192

namespace CAS // C++ Application Server
{

//
// Unescape hex-encoded value
//
static UCHAR_8 ParseHex(CCHAR_P & sData)
{
	UCHAR_8 ucTMP = 0;
	++sData;
	if      (*sData >= '0' && *sData <= '9') { ucTMP = *sData - '0';      }
	else if (*sData >= 'a' && *sData <= 'f') { ucTMP = *sData - 'a' + 10; }
	else if (*sData >= 'A' && *sData <= 'F') { ucTMP = *sData - 'A' + 10; }
	else { throw STLW::logic_error("Invalid escape sequence"); }
	ucTMP <<= 4;

	++sData;
	if      (*sData >= '0' && *sData <= '9') { ucTMP += *sData - '0';      }
	else if (*sData >= 'a' && *sData <= 'f') { ucTMP += *sData - 'a' + 10; }
	else if (*sData >= 'A' && *sData <= 'F') { ucTMP += *sData - 'A' + 10; }
	else { throw STLW::logic_error("Invalid escape sequence"); }
return ucTMP;
}

//
// Unescape data
//
STLW::string UnescapeData(const STLW::string & sData)
{
	STLW::string sResult;

	UCHAR_8 aBuffer[C_ESCAPE_BUFFER_LEN + 1];
	INT_32 iBufferPos = 0;

	CCHAR_P sBegin  = sData.data();
	CCHAR_P sEnd    = sData.data() + sData.size();

	for(;;)
	{
		// Skip unescaped data
		while (sBegin != sEnd && *sBegin != '\\')
		{
			aBuffer[iBufferPos++] = *sBegin;
			if (iBufferPos == C_ESCAPE_BUFFER_LEN)
			{
				sResult.append((CHAR_P)&aBuffer[0], iBufferPos);
				iBufferPos = 0;
			}

			++sBegin;
		}

		// End of string?
		if (sBegin == sEnd) { break; }

		// if (*sBegin == '\\') ......
		++sBegin;
		if (sBegin == sEnd) { throw STLW::logic_error("Invalid escape sequence"); }

		UCHAR_8 ucData = 0;

		// "\xFF"
		if (*sBegin == 'x' || *sBegin == 'X')
		{
			if ((sEnd - sBegin) < 3) { throw STLW::logic_error("Invalid escape sequence"); }
			ucData = ParseHex(sBegin);
		}
		// "\123"
		else if (*sBegin >= '0' && *sBegin <= '3')
		{
			if ((sEnd - sBegin) < 3) { throw STLW::logic_error("Invalid escape sequence"); }

			UINT_32 ucTMP = *sBegin - '0';
			++sBegin;
			if (*sBegin == 'x' || *sBegin == 'X')
			{
				if ((sEnd - sBegin) < 3) { throw STLW::logic_error("Invalid escape sequence"); }
				ucData = ParseHex(sBegin);
			}
			else
			{
				// Special case for '\0'
				if (!(*sBegin >= '0' && *sBegin <= '7')) { ucData = 0; }
				else
				{
					ucTMP = 8 * ucTMP + (*sBegin - '0');

					++sBegin;
					if (!(*sBegin >= '0' && *sBegin <= '7')) { throw STLW::logic_error("Invalid escape sequence"); }
					ucTMP = 8 * ucTMP + (*sBegin - '0');

					if (ucTMP > 0xFF) { throw STLW::logic_error("Invalid escape sequence"); }

					ucData = ucTMP;
				}
			}
		}
		else if (*sBegin == 'n') { ucData = '\n'; }
		else if (*sBegin == 'r') { ucData = '\r'; }
		else if (*sBegin == 't') { ucData = '\t'; }
		else if (*sBegin == 'a') { ucData = '\a'; }
		else if (*sBegin == 'b') { ucData = '\b'; }
		else if (*sBegin == 'f') { ucData = '\f'; }
		else if (*sBegin == 'v') { ucData = '\v'; }
		else if (*sBegin == 'n') { ucData = '\n'; }
		else if (*sBegin == '\\') { ucData = '\\'; }
		else
		{
			throw STLW::logic_error("Invalid escape sequence");
		}

		aBuffer[iBufferPos++] = ucData;
		if (iBufferPos == C_ESCAPE_BUFFER_LEN)
		{
			sResult.append((CHAR_P)aBuffer, iBufferPos);
			iBufferPos = 0;
		}

		++sBegin;
		if (sBegin == sEnd) { break; }
	}

	if (iBufferPos != 0) { sResult.append((CHAR_P)aBuffer, iBufferPos); }

return sResult;
}

//
// A destructor
//
ASGenericConfigHandler::ASGenericConfigHandler(CTPP::CDT & oICDT): pCDT(&oICDT)
{
	;;
}

//
// Get error description
//
STLW::string ASGenericConfigHandler::GetError() { return sError; }

//
// A destructor
//
ASGenericConfigHandler::~ASGenericConfigHandler() throw()
{
	;;
}

//
// Start of XML Element
//
INT_32 ASGenericConfigHandler::StartElement(CCHAR_P         sElement,
                                            const UINT_32   iLevel,
                                            const XMLAttr * aAttr,
                                            const UINT_32   iAttributes,
                                            const UINT_32   iLine,
                                            const UINT_32   iColumn)
{
	using namespace CTPP;

	// Remove characters
	sCharacters.erase();

	// Get current path
	std::string sLastElement;
	CTPP::CDT * pTMP = GetLastElement(sLastElement);
	// Iterate through attributes
	CTPP::CDT oAttributes(CDT::UNDEF);
	if (iAttributes != 0)
	{
		oAttributes = CDT(CDT::HASH_VAL);
		for (UINT_32 iI = 0; iI < iAttributes; ++iI)
		{
			try
			{
				oAttributes[aAttr[iI].name] = UnescapeData(aAttr[iI].value);
			}
			catch(STLW::logic_error &e)
			{
				sError = ASConfigErrorHandler::InvalidAttributeValue(sElement, aAttr[iI].name, aAttr[iI].value, iLine, iColumn);
				return -1;
			}
		}
	}

	switch(pTMP -> GetType())
	{
		// First occurence
		case CDT::UNDEF:
			pTMP -> operator=(CDT(CDT::HASH_VAL));
			pTMP -> operator[](sElement) = oAttributes;
			break;

		// Not first occurence of array
		case CDT::ARRAY_VAL:
			pTMP -> operator[](pTMP -> Size() - 1)[sElement] = oAttributes;
			break;

		// Not first occurence, HASH_VAL
		default:
			{
				CDT   oCurrentElement    = pTMP -> operator[](sElement);
				CDT & oCurrentElementRef = pTMP -> operator[](sElement);
				switch (oCurrentElement.GetType())
				{
					case CDT::UNDEF:
						pTMP -> operator[](sElement) = oAttributes;
						break;
					case CDT::ARRAY_VAL:
						oCurrentElementRef[oCurrentElement.Size()] = oAttributes;
						break;
					default:

						pTMP -> operator[](sElement) = CDT(CTPP::CDT::ARRAY_VAL);
						pTMP -> operator[](sElement)[0] = oCurrentElement;
						pTMP -> operator[](sElement)[1] = oAttributes;
				}
			}
	}
	// Store path to root
	vCDTPath.push_back(sElement);
//fprintf(stderr, "pTMP = `%s`\n", pTMP -> RecursiveDump().c_str());
return 0;
}

//
// End of XML Element
//
INT_32 ASGenericConfigHandler::EndElement(CCHAR_P        sElement,
                                          const UINT_32  iLevel,
                                          const UINT_32  iLine,
                                          const UINT_32  iColumn)
{
	std::string sLastElement;
	CTPP::CDT * pTMP = GetLastElement(sLastElement);

	if (pTMP -> Size() == 0)
	{
		try
		{
			pTMP -> operator=(UnescapeData(sCharacters));
		}
		catch(STLW::logic_error &e)
		{
			sError = ASConfigErrorHandler::InvalidTagValue(sElement, sCharacters, iLine, iColumn);
			return -1;
		}
	}

	// Remove element from path to root
	STLW::vector<STLW::string>::iterator itvCDTPath = vCDTPath.end();
	--itvCDTPath;
	vCDTPath.erase(itvCDTPath);

return 0;
}

//
// Characters insige a element
//
INT_32 ASGenericConfigHandler::Characters(CCHAR_P        sData,
                                          const UINT_32  iLength,
                                          const UINT_32  iILevel,
                                          const UINT_32  iILine,
                                          const UINT_32  iIColumn)
{
	sCharacters.append(sData, iLength);

return 0;
}

//
// XML Parse Error Handler
//
void ASGenericConfigHandler::ParseError(CCHAR_P        sReason,
                                        const UINT_32  iLevel,
                                        const UINT_32  iLine,
                                        const UINT_32  iColumn)
{
	sError = ASConfigErrorHandler::InternalError(sReason, iLine, iColumn);
}

//
// Get full XML path from vConfigPath
//
CTPP::CDT * ASGenericConfigHandler::GetLastElement(STLW::string & sLastElement)
{
	CTPP::CDT * pTMP = pCDT;
	// Empty object
	if (vCDTPath.size() == 0) { return pCDT; }

	// Not empty object, find end of tree
	UINT_32 iI = 0;
	for (iI = 0; iI < vCDTPath.size(); ++iI)
	{
		// Special case for ARRAY type
		if (pTMP -> GetType() == CTPP::CDT::ARRAY_VAL) { pTMP = &(pTMP -> At(pTMP -> Size() - 1)); }

		pTMP = &(pTMP -> At(vCDTPath[iI]));
	}

	// Special case for ARRAY type
	if (pTMP -> GetType() == CTPP::CDT::ARRAY_VAL) { pTMP = &(pTMP -> At(pTMP -> Size() - 1)); }

	sLastElement = vCDTPath[iI - 1];

return pTMP;
}

} // namespace CAS
// End.
