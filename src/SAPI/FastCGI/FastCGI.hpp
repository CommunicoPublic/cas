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
 *      FastCGI.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_HPP__
#define _FAST_CGI_HPP__ 1

namespace CAS // C++ Application Server
{
/**
  @struct FastCGIHeader FastCGI.hpp <FastCGI.hpp>
  @brief A FastCGI record consists of a fixed-length prefix followed by a variable number of content and padding bytes.

  http://www.fastcgi.com/devkit/doc/fcgi-spec.html
*/
struct FastCGIHeader
{
	/** Identifies the FastCGI protocol version. This specification documents FCGI_VERSION_1. */
	static const unsigned char FCGI_VERSION;
	/** 5.1 FCGI_BEGIN_REQUEST                                            */
	static const unsigned char FCGI_BEGIN_REQUEST;
	/** 5.4 FCGI_ABORT_REQUEST                                            */
	static const unsigned char FCGI_ABORT_REQUEST;
	/** 5.5 FCGI_END_REQUEST                                              */
	static const unsigned char FCGI_END_REQUEST;
	/** 5.2 Name-Value Pair Stream: FCGI_PARAMS                           */
	static const unsigned char FCGI_PARAMS;
	/** 5.3 Byte Streams: FCGI_STDIN, FCGI_DATA, FCGI_STDOUT, FCGI_STDERR */
	static const unsigned char FCGI_STDIN;
	/** 5.3 Byte Streams: FCGI_STDIN, FCGI_DATA, FCGI_STDOUT, FCGI_STDERR */
	static const unsigned char FCGI_STDOUT;
	/** 5.3 Byte Streams: FCGI_STDIN, FCGI_DATA, FCGI_STDOUT, FCGI_STDERR */
	static const unsigned char FCGI_STDERR;
	/** 5.3 Byte Streams: FCGI_STDIN, FCGI_DATA, FCGI_STDOUT, FCGI_STDERR */
	static const unsigned char FCGI_DATA;
	/** 4.1 FCGI_GET_VALUES, FCGI_GET_VALUES_RESULT                       */
	static const unsigned char FCGI_GET_VALUES;
	/** 4.1 FCGI_GET_VALUES, FCGI_GET_VALUES_RESULT                       */
	static const unsigned char FCGI_GET_VALUES_RESULT;
	/** 4.2 FCGI_UNKNOWN_TYPE                                             */
	static const unsigned char FCGI_UNKNOWN_TYPE;
	/** FCGI_MAXTYPE = FCGI_UNKNOWN_TYPE                                  */
	static const unsigned char FCGI_MAXTYPE;

	unsigned char version;
	unsigned char type;
	unsigned char request_id_b1;
	unsigned char request_id_b0;
	unsigned char content_length_b1;
	unsigned char content_length_b0;
	unsigned char padding_length;
	unsigned char reserved;

	inline FastCGIHeader(): version(0),
	                        type(0),
	                        request_id_b1(0),
	                        request_id_b0(0),
	                        content_length_b1(0),
	                        content_length_b0(0),
	                        padding_length(0),
	                        reserved(0)
	{
		;;
	}
};

/**
  @struct FastCGIBeginRequest FastCGI.hpp <FastCGI.hpp>
  @brief The Web server sends a FCGI_BEGIN_REQUEST record to start a request.

  http://www.fastcgi.com/devkit/doc/fcgi-spec.html
*/
struct FastCGIBeginRequest
{
	/** 6.2 Responder                          */
	static const unsigned char FCGI_RESPONDER;
	/** 6.3 Authorizer                         */
	static const unsigned char FCGI_AUTHORIZER;
	/** 6.4 Filter                             */
	static const unsigned char FCGI_FILTER;
	/** flags & FCGI_KEEP_CONN: If zero, the application closes the
	    connection after responding to this request. If not zero, the
	    application does not close the connection after responding to
	    this request; the Web server retains responsibility for
	                                                      the connection  */
	static const unsigned char FCGI_KEEP_CONN;

	unsigned char role_b1;
	unsigned char role_b0;
	unsigned char flags;
	unsigned char reserved[5];

	inline FastCGIBeginRequest(): role_b1(0),
	                              role_b0(0),
	                              flags(0)
	{
		;;
	}
};

/**
  @struct FastCGIBeginRequestRec FastCGI.hpp <FastCGI.hpp>
  @brief Request record

  http://www.fastcgi.com/devkit/doc/fcgi-spec.html
*/
struct FastCGIBeginRequestRec
{
	FastCGIHeader        header;
	FastCGIBeginRequest  body;
};

/**
  @struct FastCGIEndRequestc FastCGI.hpp <FastCGI.hpp>
  @brief The application sends a FCGI_END_REQUEST record to terminate a request, either because the application has processed the request or because the application has rejected the request.

  http://www.fastcgi.com/devkit/doc/fcgi-spec.html
*/
struct FastCGIEndRequest
{
	/** FCGI_REQUEST_COMPLETE: normal end of request */
	static const unsigned char FCGI_REQUEST_COMPLETE;
	/** FCGI_CANT_MPX_CONN: rejecting a new request  */
	static const unsigned char FCGI_CANT_MPX_CONN;
	/** FCGI_OVERLOADED: rejecting a new request     */
	static const unsigned char FCGI_OVERLOADED;
	/** FCGI_UNKNOWN_ROLE: rejecting a new request   */
	static const unsigned char FCGI_UNKNOWN_ROLE;

	unsigned char app_status_b3;
	unsigned char app_status_b2;
	unsigned char app_status_b1;
	unsigned char app_status_b0;
	unsigned char protocol_status;
	unsigned char reserved[3];

	inline FastCGIEndRequest(): app_status_b3(0),
	                            app_status_b2(0),
	                            app_status_b1(0),
	                            app_status_b0(0),
	                            protocol_status(0)
	{
		;;
	}
};

/**
  @struct FastCGIEndRequestRec FastCGI.hpp <FastCGI.hpp>
  @brief End of request record

  http://www.fastcgi.com/devkit/doc/fcgi-spec.html
*/
struct FastCGIEndRequestRec
{
	FastCGIHeader        header;
	FastCGIEndRequest    body;
};

/**
  @struct FastCGIUnknownTypeBodyc FastCGI.hpp <FastCGI.hpp>
  @brief The set of management record types is likely to grow in future versions of this protocol. To provide for this evolution, the protocol includes the FCGI_UNKNOWN_TYPE management record. When an application receives a management record whose type T it does not understand, the application responds with {FCGI_UNKNOWN_TYPE, 0, {T}}.

  http://www.fastcgi.com/devkit/doc/fcgi-spec.html
*/
struct FastCGIUnknownTypeBody
{
    unsigned char type;
    unsigned char reserved[7];
};

/**
  @struct FastCGIUnknownTypeRec FastCGI.hpp <FastCGI.hpp>
  @brief Unknown type record

  http://www.fastcgi.com/devkit/doc/fcgi-spec.html
*/
struct FastCGIUnknownTypeRec
{
	FastCGIHeader             header;
	FastCGIUnknownTypeBody    body;
};

} // namespace CAS
#endif // _FAST_CGI_HPP__
// End.

