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
 *      ASXMLParser.hpp
 *
 * $CAS$
 */
#ifndef _AS_XML_PARSER_HPP__
#define _AS_XML_PARSER_HPP__ 1

/**
  @file ASXMLParser.hpp
  @brief Application server XML parser
*/

// C Includes
#include <expat.h>
#include <stdio.h>

#include "ASTypes.hpp"

namespace CAS // C++ Application Server
{
// FWD
class ASXMLHandler;

#define C_XML_PARSER_BUFFER 8192

#ifndef XMLCALL
    #define XMLCALL
#endif

/**
  @class XMLParser XMLParser.hpp <XMLParser.hpp>
  @brief XML Parser. Based on libExpat library.
*/
class ASXMLParser
{
public:
	/**
	  @brief Constructor
	  @param pIHandler - pointer to user handler object
	  @param iILevel - recursion level
	*/
	ASXMLParser(ASXMLHandler  * pIHandler,
	            const UINT_32   iILevel = 0);

	/**
	  @brief Initialise stream parser
	*/
	void InitStream();

	/**
	  @brief Parse a block of the stream
	  @param sData - data buffer
	  @param iDataLength - buffer size
	*/
	INT_32 ParseStream(CCHAR_P        sData,
	                   const UINT_32  iDataLength);

	/**
	  @brief Finalise stream parser
	*/
	INT_32 CloseStream();

	/**
	  @brief Parse document
	  @param sData - data buffer
	  @param iDataLength - buffer size
	*/
	INT_32 ParseDocument(CCHAR_P        sData,
	                     const UINT_32  iDataLength);

	/**
	  @brief Get XML handler
	*/
	ASXMLHandler  * GetHandler();

	/**
	  @brief Parse document
	  @param F - file stream
	*/
	INT_32 ParseFile(FILE * F);

	/** @brief A destructor */
	~ASXMLParser();
private:
	// Does not exist
	ASXMLParser(const ASXMLParser  & oRhs);
	ASXMLParser& operator=(const ASXMLParser  & oRhs);

	/** F file stream flag         */
	bool            bInStream;
	/** Tag level                  */
	INT_32          iLevel;
	/** Abort flag                 */
	bool            bAbort;
	/** libExpat XML Parser object */
	XML_Parser      oParser;
	/** User defined XML handler   */
	ASXMLHandler  * pHandler;

	/** Forward map for KOI8-R encoding       */
	static UINT_16  sFWDKOI8_RMap[256];

	/** Forward map for CP866 encoding        */
	static UINT_16  sFWDCP866Map[256];

	/** Forward map for Windows-1251 encoding */
	static UINT_16  sFWDWindows_1251Map[256];

	/** Forward map for CP1251 encoding       */
	static UINT_16  sFWDCP1251Map[256];

	/** Forward map for ISO-8859-5 encoding   */
	static UINT_16  sFWDISO_8859_5Map[256];

	/** Reverse map for 2-byte encodings      */
	static CHAR_8  sRevMap[65536];

	/**
	  @brief Set handler for unknown encoding
	  @param vThis - pointer to XML Parser object
	  @param szEncoding - encoding name
	  @param pEncoding - encoding data
	*/
	static int XMLCALL UnknownEncodingHandler(void            * vThis,
	                                          const XML_Char  * szEncoding,
	                                          XML_Encoding    * pEncoding);

	/**
	  @brief Start of element
	  @param vThis - pointer to XML Parser object
	  @param szName - XML tag name
	  @param szAttr - XML tag attributes
	*/
	static void XMLCALL StartElement(void        * vThis,
	                                 const char  * szName,
	                                 const char ** szAttr);
	/**
	  @brief End of Element
	  @param vThis - pointer to XML Parser object
	  @param szName - XML tag name
	*/
	static void XMLCALL EndElement(void        * vThis,
	                               const char  * szName);

	/**
	  @brief Charactes inside element
	  @param vThis - pointer to XML Parser object
	  @param szData - Characters between tags
	  @param iLength - Characters count
	*/
	static void XMLCALL Characters(void        * vThis,
	                               const char  * szData,
	                               int           iLength);
};

} // namespace CAS
#endif // _AS_XML_PARSER_HPP__
// End.
