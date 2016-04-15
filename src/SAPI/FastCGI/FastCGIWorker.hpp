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
 *      FastCGIWorker.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_WORKER_HPP__
#define _FAST_CGI_WORKER_HPP__ 1

#include "ASRequest.hpp"
#include "ASServer.hpp"
#include <CDT.hpp>

namespace CAS // C++ Application Server
{
// FWD
class ASRequestParser;
class FastCGIIO;
struct WorkerContext;

/**
  @class WorkerProcess FastCGIWorker.hpp <FastCGIWorker.hpp>
  @brief FastCGI worker process
*/
class WorkerProcess
{
public:
	/**
	  @brief Constructor
	  @param oIWorkerContext - FastCGI worker context
	*/
	WorkerProcess(WorkerContext  & oIWorkerContext);

	/**
	  @brief Handle FastCGI connection
	*/
	INT_32 HandleConnection(const INT_32  iClientSocket);

	/**
	  @brief A destructor
	*/
	~WorkerProcess() throw();
private:
	// Does not exist
	WorkerProcess(const WorkerProcess & oRhs);
	WorkerProcess& operator=(const WorkerProcess & oRhs);

	/** Worker context             */
	WorkerContext  & oWorkerContext;
	/** Read buffer                */
	CHAR_P           vReadBuffer;

	/**
	  @brief Get parameter length
	*/
	INT_32 GetParamLen(UCCHAR_P  & sFrame,
	                   UCCHAR_P    sEndBuffer);

	/**
	  @brief Parse params
	*/
	INT_32 ParseParams(const void    * vReadBuffer,
	                   const UINT_32    iFullLen,
	                   CTPP::CDT      & mParams);

	/**
	  @brief Create POST request
	*/
	ASRequestParser * CreateParser(ASRequest                   & oASRequest,
	                               STLW::vector<STLW::string>  & vFileList,
	                               const INT_32                  iLocationId,
	                               ASServer::ASRequestContext  * pRequestContext);

	/**
	  @brief Unknown role response
	*/
	INT_32 UnknownRole(FastCGIIO    & oFastCGIIO,
	                   const INT_32   iReqId,
	                   const INT_32   iRole);

	/**
	  @brief Build FCGI_GET_VALUES_RESULT response
	*/
	INT_32 BuildGetValueResponse(CCHAR_P          szKey,
	                             const UINT_32    iValue,
	                             CHAR_P         & sBuffer,
	                             UINT_32        & iBufferLen);

	/**
	  @brief Build key-value header
	*/
	INT_32 BuildKeyValueHeader(const UINT_32    iKeyLen,
	                           const UINT_32    iValLen,
	                           CHAR_P         & sBuffer,
	                           UINT_32        & iBufferLen);

	/**
	 @brief Format query string
	*/
	void FormatQueryString(const ASRequest::RequestType  eRequestType,
	                       const UINT_32                 iRequestMethod,
	                       CCHAR_P                       szUnparsedURI,
	                       CHAR_P                        sURI);
};

} // namespace CAS
#endif // _FAST_CGI_WORKER_HPP__
// End.
