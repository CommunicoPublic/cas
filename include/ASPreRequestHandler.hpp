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
 *      ASPreRequestHandler.hpp
 *
 * $CAS$
 */
#ifndef _AS_PRE_REQUEST_HANDLER_HPP__
#define _AS_PRE_REQUEST_HANDLER_HPP__ 1

/**
  @file ASPreRequestHandler.hpp
  @brief Application server pre-request handler
*/
#include "ASLogger.hpp"
#include "ASModule.hpp"
#include "ASPool.hpp"

namespace CAS // C++ Application Server
{

// FWD
class ASRequest;

#define HOOK_OK         0             // Success
#define HOOK_DONE       1             // No further processing need
#define HOOK_ERROR     -1             // Error occured

/**
  @class ASPreRequestHandler ASPreRequestHandler.hpp <ASPreRequestHandler.hpp>
  @brief Application server request hook
*/
class ASPreRequestHandler:
  public ASModule
{
public:
	/**
	  @brief Initialize handler
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oLocationConfig - handler configuration
	  @param pContextData - context data
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured
	*/
	virtual INT_32 InitLocation(ASPool       & oGlobalPool,
	                            ASPool       & oVhostPool,
	                            CTPP::CDT    & oLocationConfig,
	                            ASObject   * & pContextData,
	                            ASLogger     & oLogger);

	/**
	  @brief Pre-request handler
	  @param vChunk - Request chunk (HTTP POST)
	  @param iChunkSize - request object
	  @param oRequest - request object
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oRequestPool - requeset data pool
	  @param oLocationConfig - location configuration
	  @param pContextData - context data
	  @param oIMC - Inter-Module communication object
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured, HANDLER_DONE - if no futher processing need
	*/
	virtual INT_32 DataChunk(CCHAR_P          vChunk,
	                         const UINT_32    iChunkSize,
	                         ASRequest      & oRequest,
	                         ASPool         & oGlobalPool,
	                         ASPool         & oVhostPool,
	                         ASPool         & oRequestPool,
	                         CTPP::CDT      & oLocationConfig,
	                         ASObject       * pContextData,
	                         CTPP::CDT      & oIMC,
	                         ASLogger       & oLogger);

	/**
	  @brief RFC 1867 upload file
	  @param sName - HTML form input name
	  @param sFullFileName - file name with path info
	  @param sFileName - uploaded file name
	  @param sTempName - temporary file name
	  @param oRequest - request object
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oRequestPool - requeset data pool
	  @param oLocationConfig - location configuration
	  @param pContextData - context data
	  @param oIMC - Inter-Module communication object
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured, HANDLER_DONE - if no futher processing need
	*/
	virtual INT_32 RFC1867File(const STLW::string  & sName,
	                           const STLW::string  & sFullFileName,
	                           const STLW::string  & sFileName,
	                           const STLW::string  & sTempName,
	                           ASRequest           & oRequest,
	                           ASPool              & oGlobalPool,
	                           ASPool              & oVhostPool,
	                           ASPool              & oRequestPool,
	                           CTPP::CDT           & oLocationConfig,
	                           ASObject            * pContextData,
	                           CTPP::CDT           & oIMC,
	                           ASLogger            & oLogger);


	/**
	  @brief Shutdown location hook
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oLocationConfig - handler configuration
	  @param pContextData - context data
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured
	*/
	virtual INT_32 ShutdownLocation(ASPool       & oGlobalPool,
	                                ASPool       & oVhostPool,
	                                CTPP::CDT    & oLocationConfig,
	                                ASObject   * & pContextData,
	                                ASLogger     & oLogger);

	/**
	  @brief A virtual destructor
	*/
	virtual ~ASPreRequestHandler() throw();

private:
	/**
	  @brief Get object type
	  @return human-readable type of object
	*/
	CCHAR_P GetModuleType() const;
};

} // namespace CAS
#endif // _AS_PRE_REQUEST_HANDLER_HPP__
// End.
