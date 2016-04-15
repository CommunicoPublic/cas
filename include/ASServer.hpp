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
 *      ASServer.hpp
 *
 * $CAS$
 */
#ifndef _AS_SERVER_HPP__
#define _AS_SERVER_HPP__ 1

/**
  @file ASServer.hpp
  @brief Application server
*/
#include "ASLocation.hpp"
#include "ASPool.hpp"

namespace CTPP
{
// FWD
class SyscallFactory;
}

namespace CAS // C++ Application Server
{

// FWD
class ASGlobalConfig;
class ASHostConfig;
class ASLogger;
class ASRequest;
class ASResponse;
class ASResponseWriter;

/**
  @class ASServer ASServer.hpp <ASServer.hpp>
  @brief Application server
*/
class ASServer
{
public:
	/**
	  @struct ASServer::ASRequestContext ASServer.hpp <ASServer.hpp>
	  @brief Application server request context
	*/
	struct ASRequestContext
	{
		/** Location Id               */
		UINT_32       location_id;
		/** Location configuration    */
		ASLocation  & location;
		/** Data model                */
		CTPP::CDT     data;
		/** Inter-module communicator */
		CTPP::CDT     imc;
		/** Request pool              */
		ASPool        request_pool;

		/**
		  @brief Constructor
		  @param iLocationId - Location Id
		  @param oLocation - Location
		*/
		ASRequestContext(const UINT_32    iLocationId,
		                 ASLocation     & oLocation);
	};

	/**
	  @brief Constructor
	  @param pIGlobalConfig - global configuration
	  @param pIHostConfig - host configuration
	  @param pIGlobalPool - global data pool
	  @param pIHostPool - host data pool
	  @param pLogger - logger object
	*/
	ASServer(const ASGlobalConfig  * pIGlobalConfig,
	         ASHostConfig          * pIHostConfig,
	         ASPool                * pIGlobalPool,
	         ASPool                * pIHostPool,
	         ASLogger              * pLogger);

	/**
	  @brief Check URI
	  @param szLocationURI - location URI
	  @param oRequest - Requset object
	  @param pLogger - logger
	  @return 0 - if URI should handled by application server, -1 - otherwise
	*/
	INT_32 CheckLocationURI(CCHAR_P      szLocationURI,
	                        ASRequest  & oRequest,
	                        ASLogger   * pLogger) const;

	/**
	  @brief Get list of accepted parameters
	  @param iLocation - location ID
	  @param oHeaders - list of headers
	  @param oArguments - list of arguments
	  @param oCookies - list of cookies
	  @param pLogger - logger
	  @return 0 - if URI should handled by application server, -1 - otherwise
	*/
	INT_32 GetLocationParameters(const UINT_32  & iLocation,
	                             CTPP::CDT      & oHeaders,
	                             CTPP::CDT      & oArguments,
	                             CTPP::CDT      & oCookies,
	                             ASLogger       * pLogger) const;

	/**
	  @brief Notify objects in global and host pools
	  @param iEvent - Event Id
	  @return 0
	*/
	INT_32 NotifyObjects(const UINT_32 iEvent);

	/**
	  @brief Handle pre-request
	  @param oContext - Request context
	  @param vChunk - chunk of data
	  @param iChunkSize - chunk size
	  @param oRequest - request object
	  @param pLogger - logger
	*/
	INT_32 HandlePreRequest(ASRequestContext  & oContext,
	                        CCHAR_P             vChunk,
	                        const UINT_32       iChunkSize,
	                        ASRequest         & oRequest,
	                        ASLogger          * pLogger);

	/**
	  @brief Handle RFC1867 file upload
	  @param oContext - Request context
	  @param sName - upload field name
	  @param sFullFileName - full file name
	  @param sFileName - file name
	  @param sTempName - temp. name
	  @param oRequest - request object
	  @param pLogger - logger
	*/
	INT_32 HandleRFC1867File(ASRequestContext    & oContext,
	                         const STLW::string  & sName,
	                         const STLW::string  & sFullFileName,
	                         const STLW::string  & sFileName,
	                         const STLW::string  & sTempName,
	                         ASRequest           & oRequest,
	                         ASLogger            * pLogger);

	/**
	  @brief Handle request
	  @param oContext - Request context
	  @param oRequest - Requset object
	  @param pResponseWriter - Application server response output interface
	  @param pLogger - logger
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 HandleRequest(ASRequestContext  & oContext,
	                     ASRequest         & oRequest,
	                     ASResponseWriter  * pResponseWriter,
	                     ASLogger          * pLogger);

	/**
	  @brief Get server configuration
	  @return Pointer to server configuration
	*/
	ASHostConfig * GetConfig();

	/**
	  @brief A destructor
	*/
	~ASServer() throw();

private:
	// Does not exist
	ASServer(const ASServer  & oRhs);
	ASServer& operator=(const ASServer  & oRhs);

	/** Global       configuration */
	const ASGlobalConfig   * pGlobalConfig;
	/** Virtual host configuration */
	ASHostConfig           * pHostConfig;
	/** Global pool of objects     */
	ASPool                 * pGlobalPool;
	/** Data pool for virtual host */
	ASPool                 * pHostPool;

	/**
	  @brief Handle request
	  @param oContext - request context
	  @param oRequest - Requset object
	  @param pResponseWriter - Application server response output interface
	  @param pLogger - logger
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 RealHandleRequest(ASRequestContext  & oContext,
	                         ASRequest         & oRequest,
	                         ASResponseWriter  * pResponseWriter,
	                         ASLogger          * pLogger);

};

} // namespace CAS
#endif // _AS_SERVER_HPP__
// End.
