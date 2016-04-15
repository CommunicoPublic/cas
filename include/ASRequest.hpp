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
 *      ASRequest.hpp
 *
 * $CAS$
 */
#ifndef _AS_REQUEST_HPP__
#define _AS_REQUEST_HPP__ 1

/**
  @file ASRequest.hpp
  @brief Application server request object
*/
#include "ASFilePool.hpp"

#include <sys/time.h>
#include <time.h>

namespace CAS // C++ Application Server
{

/**
  @struct ASAcceptXHeader ASRequest.hpp <ASRequest.hpp>
  @brief Application server Accept-X header placeholder, see RFC 2616 "14 Header Field Definitions"
*/
struct ASAcceptXHeader
{
	/** Value */
	STLW::string   value;
	/** */
	W_FLOAT        q;
};

/**
  @struct ASServerTime ASRequest.hpp <ASRequest.hpp>
  @brief Application server time
*/
struct ASServerTime
{
	/** Server time */
	struct timeval    timestamp;
	/** Server local time (parsed) */
	struct tm         localtime;
	/** Server GMT time (parsed)   */
	struct tm         gmtime;
};

/**
  @struct ASRequest ASRequest.hpp <ASRequest.hpp>
  @brief Application server request object
*/
struct ASRequest
{
	static const UINT_32 OPTIONS;
	static const UINT_32 GET;
	static const UINT_32 HEAD;
	static const UINT_32 POST;
	static const UINT_32 TRACE;
	static const UINT_32 CONNECT;
	static const UINT_32 PROPFIND;
	static const UINT_32 PROPPATCH;
	static const UINT_32 MKCOL;
	static const UINT_32 DELETE;
	static const UINT_32 PUT;
	static const UINT_32 COPY;
	static const UINT_32 MOVE;
	static const UINT_32 LOCK;
	static const UINT_32 UNLOCK;

	enum RequestType { HTTP_REQUEST, HTTP_RFC1867_REQUEST, XMLRPC_REQUEST, JSONRPC_REQUEST, BSONRPC_REQUEST };

	/** Name of current location */
	STLW::string                     location_name;
	/** Request method           */
	UINT_32                          request_method;
	/** Request type             */
	RequestType                      request_type;
	/** Request URI              */
	STLW::string                     uri;
	/** Unparsed requeset URI    */
	STLW::string                     unparsed_uri;
	/** Host name                */
	STLW::string                     host;
	/** Port                     */
	UINT_32                          port;
	/** Remote IP address        */
	STLW::string                     remote_ip;
	/** List of URI components   */
	CTPP::CDT                        uri_components;
	/** Raw headers */
	STLW::string                     raw_headers;
	/** List of headers          */
	CTPP::CDT                        headers;
	/** List of cookies          */
	CTPP::CDT                        cookies;
	/** List of arguments        */
	CTPP::CDT                        arguments;
	/** Referrer, if present     */
	STLW::string                     referer;
	/** Content-Length           */
	INT_64                           content_length;
	/** Content-Type             */
	STLW::string                     content_type;
	/** User Agent               */
	STLW::string                     user_agent;
	/** Accept header            */
	STLW::vector<ASAcceptXHeader>    accept;
	/** Accept-Language header   */
	STLW::vector<ASAcceptXHeader>    accept_language;
	/** Accept-Encoding header   */
	STLW::vector<ASAcceptXHeader>    accept_encoding;
	/** Username (Basic Auth)    */
	STLW::string                     user;
	/** Password (Basic Auth)    */
	STLW::string                     password;
	/** List of uploaded files   */
	ASFilePool                       files;
	/** Server time              */
	ASServerTime                     server_time;
	/** Server host name         */
	STLW::string                     system_hostname;
	/** Server local IP          */
	STLW::string                     local_ip;
	/** Raw POST data            */
	STLW::string                     raw_post_data;

	/**
	  @brief Convert method name to string
	*/
	static CCHAR_P MethodToStr(const UINT_32 iMethod);

	/**
	  @brief Convert request type to string
	*/
	static CCHAR_P RequestToStr(const RequestType eRequestType);
};

} // namespace CAS
#endif // _AS_REQUEST_HPP__
// End.
