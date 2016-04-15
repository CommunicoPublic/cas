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
 *      ASXMLRPCHandler.hpp
 *
 * $CAS$
 */
#ifndef _AS_XML_RPC_HANDLER_HPP__
#define _AS_XML_RPC_HANDLER_HPP__ 1

/**
  @file ASXMLRPCHandler.hpp
  @brief XML RPC parser
*/

// C++ Includes
#include "ASXMLHandler.hpp"

namespace CAS // C++ Application Server
{

/**
  @class ASXMLRPCHandler ASXMLRPCHandler.hpp <ASXMLRPCHandler.hpp>
  @brief XML RPC parser
*/
class ASXMLRPCHandler:
  public ASXMLHandler
{
public:
	/**
	  @brief A destructor
	*/
	ASXMLRPCHandler(CTPP::CDT & oICDT);

	/**
	  @brief Get error description
	*/
	STLW::string GetError();

	/**
	  @brief A destructor
	*/
	~ASXMLRPCHandler() throw();
private:
	// Does not exist
	ASXMLRPCHandler(const ASXMLRPCHandler  & oRhs);
	ASXMLRPCHandler& operator=(const ASXMLRPCHandler  & oRhs);

	enum eState    { ST_NONE = 0,
	                 ST_NAME,
	                 ST_VALUE_TYPE,
	                 ST_VALUE,
	                 ST_MEMBER,
	                 ST_STRUCT,
	                 ST_DATA,
	                 ST_ARRAY,
	                 ST_PARAM,
	                 ST_PARAMS,
	                 ST_METHOD_NAME,
	                 ST_METHOD_CALL
	               };

	enum eDataType { DT_NONE,
	                 DT_I4,
	                 DT_BOOL,
	                 DT_INTEGER,
	                 DT_DOUBLE,
	                 DT_STRING,
	                 DT_DATETIME,
	                 DT_BASE64,
	                 DT_ARRAY,
	                 DT_STRUCT
	               };

	/**
	  @struct PathRecord ASXMLRPCHandler.hpp <ASXMLRPCHandler.hpp>
	  @brief Parser state
	*/
	struct PathRecord
	{
		/** Current parser state */
		eState       current_state;
		/** Current data type    */
		eDataType    current_datatype;
		/** Current CDT          */
		CTPP::CDT  * current_cdt;
	};

	/** CDT tree path                              */
	STLW::vector<PathRecord> vCDTPath;
	/** XML RPC request                            */
	CTPP::CDT                * pCDT;
	/** Error description                          */
	STLW::string               sError;
	/** Temp. string                               */
	STLW::string               sCharacters;
	/** Current parser state                       */
	eState                     eCurrentState;
	/** Current data type                          */
	eDataType                  eCurrentDataType;
	/** Key name                                   */
	STLW::string               sName;
	/** <value-type> tag found or not              */
	bool                       bValueTypeFound;

	/**
	  @brief Start of XML Element
	  @param sElement -  Tag name
	  @param iLevel - Tag level
	  @param aAttr - tag attributes
	  @param iAttributes - number of attributes
	  @param iLine - Attrubute line
	  @param iColumn - Attrubute position
	*/
	INT_32 StartElement(CCHAR_P         sElement,
	                    const UINT_32   iLevel,
	                    const XMLAttr * aAttr,
	                    const UINT_32   iAttributes,
	                    const UINT_32   iLine,
	                    const UINT_32   iColumn);

	/**
	  @brief End of XML Element
	  @param sElement -  Tag name
	  @param iLevel - Tag level
	  @param iLine - Attrubute line
	  @param iColumn - Attrubute position
	*/
	INT_32 EndElement(CCHAR_P        sElement,
	                  const UINT_32  iLevel,
	                  const UINT_32  iLine,
	                  const UINT_32  iColumn);

	/**
	  @brief Characters insige a element
	  @param sData - Data
	  @param iLength - data length
	  @param iLevel - Tag level
	  @param iLine - Attrubute line
	  @param iColumn - Attrubute position
	*/
	INT_32 Characters(CCHAR_P        sData,
	                  const UINT_32  iLength,
	                  const UINT_32  iLevel,
	                  const UINT_32  iLine,
	                  const UINT_32  iColumn);

	/**
	  @brief XML Parse Error Handler
	  @param sReason - Error Description
	  @param iLevel - Line where error occured
	  @param iLine - Attrubute line
	  @param iColumn - Attrubute position
	*/
	void ParseError(CCHAR_P        sReason,
	                const UINT_32  iLevel,
	                const UINT_32  iLine,
	                const UINT_32  iColumn);
};

} // namespace CAS
#endif // _AS_XML_RPC_HANDLER_HPP__
// End.

