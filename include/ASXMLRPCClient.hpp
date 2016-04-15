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
 *      ASXMLRPCClient.hpp
 *
 * $CAS$
 */
#ifndef _AS_XML_RPC_CLIENT_HPP__
#define _AS_XML_RPC_CLIENT_HPP__ 1

#include "ASTypes.hpp"
#include "STLString.hpp"

/**
  @file ASXMLRPCClient.hpp
  @brief HTTP XML RPC client
*/
namespace CAS // C++ Application Server
{

/**
  @class ASXMLRPCClient ASXMLRPCClient.hpp <ASXMLRPCClient.hpp>
  @brief HTTP Cookie
*/
class ASXMLRPCClient
{
public:
	/**
	  @brief Constructor
	  @param sURL - XML RPC URL
	  @param iPort - Port
	  @param iConnTimeout - Connection timeout
	  @param iIOTimeout - Input/Output timeout
	  @param oHeaders - Set of HTTP headers
	  @param sServer - server name
	*/
	ASXMLRPCClient(const STLW::string  & sURL,
	               const UINT_32         iPort,
	               const UINT_32         iConnTimeout,
	               const UINT_32         iIOTimeout,
	               const CTPP::CDT     & oHeaders = CTPP::CDT(),
	               const STLW::string  & sServer = "");

	/**
	  @brief Call XML-RPC server
	  @param sMethod - RPC method name
	  @param oRequest - Request data
	  @param oResponse - Response data
	*/
	INT_32 Call(const STLW::string  & sMethod,
	            const CTPP::CDT     & oRequest,
	            CTPP::CDT           & oResponse);

	/**
	  @brief A drestructor
	*/
	~ASXMLRPCClient() throw();

private:
	// Does not exists
	ASXMLRPCClient(const ASXMLRPCClient & oRhs);
	ASXMLRPCClient & operator =(const ASXMLRPCClient & oRhs);

	/** cURL object         */
	void               * vCURL;

	/** Number of instances */
	static INT_64       iInstances;

	/**
	  @brief Initialize static members
	*/
	static void Init();

	/**
	  @brief Destrocy static methods
	*/
	static void Destroy();

};

} // namespace CAS
#endif // _AS_XML_RPC_CLIENT_HPP__
// End.
