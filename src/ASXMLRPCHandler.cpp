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
 *      ASXMLRPCHandler.cpp
 *
 * $CAS$
 */

// C++ Includes
#include "ASXMLRPCHandler.hpp"

#include <stdio.h>
#include <strings.h>

#include <CDT.hpp>
#include <CTPP2Util.hpp>

#include "ASConfigErrorHandler.hpp"

namespace CAS // C++ Application Server
{

//
// A destructor
//
ASXMLRPCHandler::ASXMLRPCHandler(CTPP::CDT & oICDT): pCDT(&oICDT), eCurrentState(ST_NONE), eCurrentDataType(DT_NONE)
{
	;;
}

//
// Get error description
//
STLW::string ASXMLRPCHandler::GetError() { return sError; }

//
// A destructor
//
ASXMLRPCHandler::~ASXMLRPCHandler() throw()
{
	;;
}

//
// Start of XML Element
//
INT_32 ASXMLRPCHandler::StartElement(CCHAR_P          sElement,
                                     const UINT_32    iILevel,
                                     const XMLAttr  * aAttr,
                                     const UINT_32    iAttributes,
                                     const UINT_32    iILine,
                                     const UINT_32    iIColumn)
{
	using namespace CTPP;

	// Store stack
	PathRecord oTMP = { eCurrentState, eCurrentDataType, pCDT };
	vCDTPath.push_back(oTMP);

	// methodCall ONLY
	if (iILevel == 0)
	{
		// Server
		if (strcasecmp("methodCall", sElement) != 0 &&
		// Client
		    strcasecmp("methodResponse", sElement) != 0)
		{
			sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn);
			return -1;
		}

		eCurrentState = ST_METHOD_CALL;
	}
	// methodName, params
	else if (iILevel == 1)
	{
		if      (strcasecmp("methodName", sElement) == 0) { eCurrentState = ST_METHOD_NAME; }
		else if (strcasecmp("params",     sElement) == 0)
		{
			eCurrentState = ST_PARAMS;
			CDT & oTMP = pCDT -> operator[]("params");
			oTMP = CDT(CDT::ARRAY_VAL);
			pCDT = &oTMP;
		}
		else
		{
			sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn);
			return -1;
		}
	}
	// param
	else if (iILevel == 2)
	{
		if      (strcasecmp("param", sElement) == 0)
		{
			eCurrentState = ST_PARAM;
			pCDT -> PushBack(CDT());
			pCDT = &(pCDT -> At(pCDT -> Size() - 1));
		}
		else
		{
			sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn);
			return -1;
		}
	}
	// Unspecified flow of tags below
	else
	{
		if      (eCurrentState == ST_NAME)
		{
			;; // Do nothing
		}
		else if (eCurrentState == ST_VALUE_TYPE)
		{
			;; // Do nothing
			bValueTypeFound = false;
		}
		else if (eCurrentState == ST_VALUE)
		{
			eCurrentState = ST_VALUE_TYPE;
			if      (strcasecmp("i4",               sElement) == 0) { eCurrentDataType = DT_I4;       }
			else if (strcasecmp("int",              sElement) == 0) { eCurrentDataType = DT_INTEGER;  }
			else if (strcasecmp("integer",          sElement) == 0) { eCurrentDataType = DT_INTEGER;  }
			else if (strcasecmp("boolean",          sElement) == 0) { eCurrentDataType = DT_BOOL;     }
			else if (strcasecmp("bool",             sElement) == 0) { eCurrentDataType = DT_BOOL;     }
			else if (strcasecmp("double",           sElement) == 0) { eCurrentDataType = DT_DOUBLE;   }
			else if (strcasecmp("float",            sElement) == 0) { eCurrentDataType = DT_DOUBLE;   }
			else if (strcasecmp("string",           sElement) == 0) { eCurrentDataType = DT_STRING;   }
			else if (strcasecmp("dateTime",         sElement) == 0) { eCurrentDataType = DT_DATETIME; }
			else if (strcasecmp("dateTime.iso8601", sElement) == 0) { eCurrentDataType = DT_DATETIME; }
			else if (strcasecmp("base64",           sElement) == 0) { eCurrentDataType = DT_BASE64;   }
			else if (strcasecmp("struct",           sElement) == 0) { eCurrentDataType = DT_STRUCT; eCurrentState = ST_STRUCT; }
			else if (strcasecmp("array",            sElement) == 0) { eCurrentDataType = DT_ARRAY;  eCurrentState = ST_ARRAY;  }
			else
			{
				sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn);
				return -1;
			}
			bValueTypeFound = true;
		}
		else if (eCurrentState == ST_MEMBER)
		{
			if      (strcasecmp("name",  sElement) == 0) { eCurrentState = ST_NAME; }
			else if (strcasecmp("value", sElement) == 0)
			{
				eCurrentState   = ST_VALUE;
				if (eCurrentDataType == DT_STRUCT)
				{
					bValueTypeFound = false;

					if(pCDT -> GetType() != CDT::HASH_VAL)
					{
						pCDT -> operator=(CDT(CDT::HASH_VAL));
					}
					CDT & oTMP = pCDT -> operator[](sName);
					pCDT = &oTMP;
				}
			}
			else { sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn); return -1; }
		}
		else if (eCurrentState == ST_STRUCT)
		{
			if (strcasecmp("member", sElement) == 0) { eCurrentState = ST_MEMBER; }
			else { sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn); return -1; }
		}
		else if (eCurrentState == ST_DATA)
		{
			if (strcasecmp("value", sElement) == 0)
			{
				eCurrentState   = ST_VALUE;

				if (eCurrentDataType == DT_ARRAY)
				{
					bValueTypeFound = false;

					if (pCDT -> GetType() != CDT::ARRAY_VAL)
					{
						pCDT -> operator=(CDT(CDT::ARRAY_VAL));
					}
					pCDT -> PushBack(CDT());
					pCDT = &(pCDT -> At(pCDT -> Size() - 1));
				}
			}
			else { sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn); return -1; }
		}
		else if (eCurrentState == ST_ARRAY)
		{
			if (strcasecmp("data", sElement) == 0) { eCurrentState = ST_DATA; }
			else { sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn); return -1; }
		}
		else if (eCurrentState == ST_PARAM)
		{
			if      (strcasecmp("value", sElement) == 0)
			{
				eCurrentState   = ST_VALUE;
			}
			else { sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn); return -1; }
		}
		else
		{
			sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn);
			return -1;
		}
	}

	// Remove characters
	sCharacters.erase();

return 0;
}

//
// End of XML Element
//
INT_32 ASXMLRPCHandler::EndElement(CCHAR_P        sElement,
                                   const UINT_32  iILevel,
                                   const UINT_32  iILine,
                                   const UINT_32  iIColumn)
{
	using namespace CTPP;

	if      (eCurrentState == ST_NAME)
	{
		sName         = sCharacters;
	}
	else if (eCurrentState == ST_VALUE_TYPE)
	{
		switch (eCurrentDataType)
		{
			case DT_I4:
			case DT_INTEGER:
				{
					long long iLL = 0;
					sscanf(sCharacters.c_str(), "%lli", &iLL);
					pCDT -> operator=(INT_64(iLL));
				}
				break;

			case DT_BOOL:
				{
					if       (strcasecmp("true", sCharacters.c_str()) == 0 ||
					          strcasecmp("1",    sCharacters.c_str()) == 0) { pCDT -> operator=(1); }
					else if  (strcasecmp("false",sCharacters.c_str()) == 0 ||
					          strcasecmp("0",    sCharacters.c_str()) == 0) { pCDT -> operator=(0); }
					else     { sError = ASConfigErrorHandler::InvalidTagValue(sElement, sCharacters, iILine, iIColumn); return -1; }
				}
				break;

			case DT_DOUBLE:
				{
					double dVal = 0;
					sscanf(sCharacters.c_str(), "%lg", &dVal);
					pCDT -> operator=(W_FLOAT(dVal));
				}
				break;

			case DT_STRING:
				pCDT -> operator=(sCharacters);
				break;

			case DT_DATETIME:
				pCDT -> operator=(sCharacters); // TBD: create parser for datetime format
				break;

			case DT_BASE64:
				pCDT -> operator=(Base64Decode(sCharacters));
				break;

			default:
				sError = ASConfigErrorHandler::InvalidTag(sElement, iILine, iIColumn);
				return -1;
		}
	}
	else if (eCurrentState == ST_VALUE)
	{
		// XML-RPC: If no type is indicated, the type is string.
		if (!bValueTypeFound)
		{
			pCDT -> operator=(sCharacters);
			bValueTypeFound = true;
		}
		eCurrentDataType = DT_NONE;
	}
	else if (eCurrentState == ST_MEMBER)
	{
		// Do nothing
	}
	else if (eCurrentState == ST_ARRAY)
	{
		// Push value to array
	}
	else if (eCurrentState == ST_STRUCT)
	{
		// Do nothing
	}
	else if (eCurrentState == ST_METHOD_CALL)
	{
		// Do nothing
	}
	// Store method name
	else if (eCurrentState == ST_METHOD_NAME)
	{
		CDT & oTMP = pCDT -> operator[]("method");
		// One method in list?
		if (oTMP.GetType() == CDT::UNDEF)          { oTMP = sCharacters;         }
		// Array of methods?
		else if (oTMP.GetType() != CDT::ARRAY_VAL) { oTMP.PushBack(sCharacters); }
		// Make array
		else
		{
			const CDT oStored = oTMP;
			oTMP = CDT(CDT::ARRAY_VAL);
			oTMP.PushBack(oStored);
			oTMP.PushBack(sCharacters);
		}
	}
	sCharacters.erase();

	// Restore current root
	PathRecord & oTMP  = vCDTPath[iILevel];

	eCurrentDataType = oTMP.current_datatype;
	eCurrentState    = oTMP.current_state;
	pCDT             = oTMP.current_cdt;
	vCDTPath.pop_back();

return 0;
}

//
// Characters insige a element
//
INT_32 ASXMLRPCHandler::Characters(CCHAR_P        sData,
                                   const UINT_32  iLength,
                                   const UINT_32  iILevel,
                                   const UINT_32  iILine,
                                   const UINT_32  iIColumn)
{
	if (eCurrentState == ST_NAME ||
	    eCurrentState == ST_VALUE_TYPE ||
	    (eCurrentState == ST_VALUE && !bValueTypeFound) ||
	    eCurrentState == ST_METHOD_NAME)
	{
		sCharacters.append(sData, iLength);
	}

return 0;
}

//
// XML Parse Error Handler
//
void ASXMLRPCHandler::ParseError(CCHAR_P        sReason,
                                 const UINT_32  iLevel,
                                 const UINT_32  iLine,
                                 const UINT_32  iColumn)
{
	sError = ASConfigErrorHandler::InternalError(sReason, iLine, iColumn);
}

} // namespace CAS
// End.
