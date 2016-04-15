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
 *      FastCGI.cpp
 *
 * $CAS$
 */

#include "FastCGI.hpp"

namespace CAS
{
/*
 * http://www.fastcgi.com/devkit/doc/fcgi-spec.html
 * 5. Application Record Types
 */

/*
                                  WS->App    management  stream

        FCGI_GET_VALUES           x          x
        FCGI_GET_VALUES_RESULT               x
        FCGI_UNKNOWN_TYPE                    x

        FCGI_BEGIN_REQUEST        x
        FCGI_ABORT_REQUEST        x
        FCGI_END_REQUEST
        FCGI_PARAMS               x                      x
        FCGI_STDIN                x                      x
        FCGI_DATA                 x                      x
        FCGI_STDOUT                                      x
        FCGI_STDERR                                      x
*/

// Identifies the FastCGI protocol version. This specification documents FCGI_VERSION_1. */
const unsigned char FastCGIHeader::FCGI_VERSION           = 1;

// 5.1 FCGI_BEGIN_REQUEST
const unsigned char FastCGIHeader::FCGI_BEGIN_REQUEST     = 1;
// 5.4 FCGI_ABORT_REQUEST
const unsigned char FastCGIHeader::FCGI_ABORT_REQUEST     = 2;
// 5.5 FCGI_END_REQUEST
const unsigned char FastCGIHeader::FCGI_END_REQUEST       = 3;
// 5.2 Name-Value Pair Stream: FCGI_PARAMS
const unsigned char FastCGIHeader::FCGI_PARAMS            = 4;
// 5.3 Byte Streams: FCGI_STDIN, FCGI_DATA, FCGI_STDOUT, FCGI_STDERR
const unsigned char FastCGIHeader::FCGI_STDIN             = 5;
// 5.3 Byte Streams: FCGI_STDIN, FCGI_DATA, FCGI_STDOUT, FCGI_STDERR
const unsigned char FastCGIHeader::FCGI_STDOUT            = 6;
// 5.3 Byte Streams: FCGI_STDIN, FCGI_DATA, FCGI_STDOUT, FCGI_STDERR
const unsigned char FastCGIHeader::FCGI_STDERR            = 7;
// 5.3 Byte Streams: FCGI_STDIN, FCGI_DATA, FCGI_STDOUT, FCGI_STDERR
const unsigned char FastCGIHeader::FCGI_DATA              = 8;
// 4.1 FCGI_GET_VALUES, FCGI_GET_VALUES_RESULT
const unsigned char FastCGIHeader::FCGI_GET_VALUES        = 9;
// 4.1 FCGI_GET_VALUES, FCGI_GET_VALUES_RESULT
const unsigned char FastCGIHeader::FCGI_GET_VALUES_RESULT = 10;
// 4.2 FCGI_UNKNOWN_TYPE
const unsigned char FastCGIHeader::FCGI_UNKNOWN_TYPE      = 11;
// FCGI_MAXTYPE = FCGI_UNKNOWN_TYPE
const unsigned char FastCGIHeader::FCGI_MAXTYPE           = 11;

// 6.2 Responder
const unsigned char FastCGIBeginRequest::FCGI_RESPONDER   = 1;
// 6.3 Authorizer
const unsigned char FastCGIBeginRequest::FCGI_AUTHORIZER  = 2;
// 6.4 Filter
const unsigned char FastCGIBeginRequest::FCGI_FILTER      = 3;

// flags & FCGI_KEEP_CONN: If zero, the application closes the
// connection after responding to this request. If not zero, the
// application does not close the connection after responding to
// this request; the Web server retains responsibility for the connection
const unsigned char FastCGIBeginRequest::FCGI_KEEP_CONN   = 1;

// FCGI_REQUEST_COMPLETE: normal end of request
const unsigned char FastCGIEndRequest::FCGI_REQUEST_COMPLETE = 0;
// FCGI_CANT_MPX_CONN: rejecting a new request
const unsigned char FastCGIEndRequest::FCGI_CANT_MPX_CONN    = 1;
// FCGI_OVERLOADED: rejecting a new request
const unsigned char FastCGIEndRequest::FCGI_OVERLOADED       = 2;
// FCGI_UNKNOWN_ROLE: rejecting a new request
const unsigned char FastCGIEndRequest::FCGI_UNKNOWN_ROLE     = 3;

} // namespace CAS
// End.
