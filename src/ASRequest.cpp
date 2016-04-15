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
 *      ASRequest.cpp
 *
 * $CAS$
 */

#include "ASRequest.hpp"

namespace CAS // C++ Application Server
{
const UINT_32 ASRequest::OPTIONS   = 0x00000001;
const UINT_32 ASRequest::GET       = 0x00000002;
const UINT_32 ASRequest::HEAD      = 0x00000004;
const UINT_32 ASRequest::POST      = 0x00000008;
const UINT_32 ASRequest::TRACE     = 0x00000010;
const UINT_32 ASRequest::CONNECT   = 0x00000020;
const UINT_32 ASRequest::PROPFIND  = 0x00000040;
const UINT_32 ASRequest::PROPPATCH = 0x00000080;
const UINT_32 ASRequest::MKCOL     = 0x00000100;
const UINT_32 ASRequest::DELETE    = 0x00000200;
const UINT_32 ASRequest::PUT       = 0x00000400;
const UINT_32 ASRequest::COPY      = 0x00000800;
const UINT_32 ASRequest::MOVE      = 0x00001000;
const UINT_32 ASRequest::LOCK      = 0x00002000;
const UINT_32 ASRequest::UNLOCK    = 0x00004000;

//
// Convert method name to string
//
CCHAR_P ASRequest::MethodToStr(const UINT_32 iMethod)
{
	switch(iMethod)
	{
		case OPTIONS:   return "OPTIONS";
		case GET:       return "GET";
		case HEAD:      return "HEAD";
		case POST:      return "POST";
		case TRACE:     return "TRACE";
		case CONNECT:   return "CONNECT";
		case PROPFIND:  return "PROPFIND";
		case PROPPATCH: return "PROPPATCH";
		case MKCOL:     return "MKCOL";
		case DELETE:    return "DELETE";
		case PUT:       return "PUT";
		case COPY:      return "COPY";
		case MOVE:      return "MOVE";
		case LOCK:      return "LOCK";
		case UNLOCK:    return "UNLOCK";
	}
return "-";
}

//
// Convert request type to string
//
CCHAR_P ASRequest::RequestToStr(const RequestType eRequestType)
{
	switch(eRequestType)
	{
		case HTTP_REQUEST:          return "HTTP";
		case HTTP_RFC1867_REQUEST:  return "RFC1867";
		case XMLRPC_REQUEST:        return "XMLRPC";
		case JSONRPC_REQUEST:       return "JSONRPC";
		case BSONRPC_REQUEST:       return "BSONRPC";
	}
return "-";
}

} // namespace CAS
// End.
