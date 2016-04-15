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
 *      ASGlobalConfigHandler.hpp
 *
 * $CAS$
 */
#ifndef _AS_GLOBAL_CONFIG_HANDLER_HPP__
#define _AS_GLOBAL_CONFIG_HANDLER_HPP__ 1

/**
  @file ASGlobalConfigHandler.hpp
  @brief Global CAS XML configuration parser
*/

// C++ Includes
#include "ASXMLHandler.hpp"
#include "STLSet.hpp"

namespace CAS // C++ Application Server
{

// FWD
class ASGlobalConfig;
class ASGenericConfigHandler;
class ASObjectConfig;

/**
  @class ASGlobalConfigHandler ASGlobalConfigHandler.hpp <ASGlobalConfigHandler.hpp>
  @brief Base abstract class for user handler
*/
class ASGlobalConfigHandler:
  public ASXMLHandler
{
public:
	/**
	  @brief Constructor
	  @param oIGLobalConfig - global configuration
	*/
	ASGlobalConfigHandler(ASGlobalConfig & oIGLobalConfig);

	/**
	  @brief Constructor
	  @param oIGLobalConfig - global configuration
	  @param vIIncludeDirs - list of include directories
	*/
	ASGlobalConfigHandler(ASGlobalConfig                    & oIGLobalConfig,
	                      const STLW::vector<STLW::string>  & vIIncludeDirs);

	/**
	  @brief Get error description
	*/
	STLW::string GetError();

	/**
	  @brief A destructor
	*/
	~ASGlobalConfigHandler() throw();
private:
	// Does not exist
	ASGlobalConfigHandler(const ASGlobalConfigHandler  & oRhs);
	ASGlobalConfigHandler& operator=(const ASGlobalConfigHandler  & oRhs);

	/** Global configuration */
	ASGlobalConfig             & oGlobalConfig;
	/** List of configuration include
	              directories */
	STLW::vector<STLW::string>   vIncludeDirs;
	/** Module configuration */
	ASObjectConfig             * pModule;
	/** Configuration parser */
	ASGenericConfigHandler     * pGenericHandler;
	/** Temp. string         */
	STLW::string                 sCharacters;
	/** Error description    */
	STLW::string                 sError;

	/**
	  @brief Include wildcard directive
	  @param sWildcard - wildcard
	  @param sElement -  Tag name
	  @param iLevel - Tag level
	  @param iLine - Attrubute line
	  @param iColumn - Attrubute position
	*/
	INT_32 IncludeWildcard(CCHAR_P          sWildcard,
	                       CCHAR_P          sElement,
	                       const UINT_32    iLevel,
	                       const UINT_32    iLine,
	                       const UINT_32    iColumn);

	/**
	  @brief Find files by mask
	*/
	INT_32 FindFiles(CCHAR_P                    sWildcard,
	                 STLW::set<STLW::string>  & oFiles);

	/**
	  @brief Include file directive
	  @param sFileName - file to include
	  @param sElement -  Tag name
	  @param iLevel - Tag level
	  @param iLine - Attrubute line
	  @param iColumn - Attrubute position
	*/
	INT_32 IncludeFile(CCHAR_P          sFileName,
	                   CCHAR_P          sElement,
	                   const UINT_32    iLevel,
	                   const UINT_32    iLine,
	                   const UINT_32    iColumn);

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
	INT_32 EndElement(CCHAR_P         sElement,
	                  const UINT_32   iLevel,
	                  const UINT_32   iLine,
	                  const UINT_32   iColumn);

	/**
	  @brief Characters insige a element
	  @param sData - Data
	  @param iLength - data length
	  @param iLevel - Tag level
	  @param iLine - Attrubute line
	  @param iColumn - Attrubute position
	*/
	INT_32 Characters(CCHAR_P         sData,
	                  const UINT_32   iLength,
	                  const UINT_32   iLevel,
	                  const UINT_32   iLine,
	                  const UINT_32   iColumn);

	/**
	  @brief XML Parse Error Handler
	  @param sReason - Error Description
	  @param iLevel - Line where error occured
	  @param iColumn - Column where error occured
	  @param iLine - Attrubute line
	  @param iColumn - Attrubute position
	*/
	void ParseError(CCHAR_P         sReason,
	                const UINT_32   iLevel,
	                const UINT_32   iLine,
	                const UINT_32   iColumn);

};

} // namespace CAS
#endif // _AS_GLOBAL_CONFIG_HANDLER_HPP__
// End.
