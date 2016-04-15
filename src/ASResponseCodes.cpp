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
 *      ASResponseCodes.cpp
 *
 * $CAS$
 */

#include "ASResponseCodes.hpp"

namespace CAS // C++ Application Server
{

//
// Get HTTP response code
//
CCHAR_P GetResponseCode(const UINT_32  iResponseCode)
{
	switch (iResponseCode)
	{
		case 100: return "100 Continue";
		case 101: return "101 Switching Protocols";
		case 200: return "200 OK";
		case 201: return "201 Created";
		case 202: return "202 Accepted";
		case 203: return "203 Non-Authoritative Information";
		case 204: return "204 No Content";
		case 205: return "205 Reset Content";
		case 206: return "206 Partial Content";
		case 300: return "300 Multiple Choices";
		case 301: return "301 Moved Permanently";
		case 302: return "302 Found";
		case 303: return "303 See Other";
		case 304: return "304 Not Modified";
		case 305: return "305 Use Proxy";
		case 307: return "307 Temporary Redirect";
		case 400: return "400 Bad Request";
		case 401: return "401 Unauthorized";
		case 402: return "402 Payment Required";
		case 403: return "403 Forbidden";
		case 404: return "404 Not Found";
		case 405: return "405 Method Not Allowed";
		case 406: return "406 Not Acceptable";
		case 407: return "407 Proxy Authentication Required";
		case 408: return "408 Request Timeout";
		case 409: return "409 Conflict";
		case 410: return "410 Gone";
		case 411: return "411 Length Required";
		case 412: return "412 Precondition Failed";
		case 413: return "413 Request Entity Too Large";
		case 414: return "414 Request-URI Too Long";
		case 415: return "415 Unsupported Media Type";
		case 416: return "416 Requested Range Not Satisfiable";
		case 417: return "417 Expectation Failed";
		case 418: return "418 I'm a teapot";
		case 422: return "422 Unprocessable Entity (WebDAV) (RFC 4918)";
		case 423: return "423 Locked (WebDAV) (RFC 4918)";
		case 424: return "424 Failed Dependency (WebDAV) (RFC 4918)";
		case 425: return "425 Unordered Collection (RFC 3648)";
		case 426: return "426 Upgrade Required (RFC 2817)";
		case 449: return "449 Retry With";
		case 450: return "450 Blocked by Windows Parental Controls";
		case 500: return "500 Internal Server Error";
		case 501: return "501 Not Implemented";
		case 502: return "502 Bad Gateway";
		case 503: return "503 Service Unavailable";
		case 504: return "504 Gateway Timeout";
		case 505: return "505 HTTP Version Not Supported";
		case 506: return "506 Variant Also Negotiates (RFC 2295)";
		case 507: return "507 Insufficient Storage (WebDAV) (RFC 4918)";
		case 509: return "509 Bandwidth Limit Exceeded (Apache bw/limited extension)";
		case 510: return "510 Not Extended (RFC 2774)";
		case 600: return "600 Malformed URI";
		case 601: return "601 Connection Timed";
		case 602: return "602 Unknown Error";
		case 603: return "603 Could Not Parse Reply";
		case 604: return "604 Protocol Not Supported";
		default:
			return NULL;
	}
}

} // namespace CAS
// End.
