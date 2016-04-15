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
 *      ASApache13SAPI.cpp
 *
 * $CAS$
 */
// C++ Includes
#include "ASApache13SAPI.hpp"
#include "ASLoggerFile.hpp"
#include "ASResponse.hpp"
#include "ASResponseCodes.hpp"

#ifdef CR
#undef CR
#endif

#include "httpd.h"
#include "http_log.h"
#include "http_protocol.h"

namespace CAS // C++ Application Server
{

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ASLoggerApacheST
//

//
// Constructor
//
ASLoggerApacheST::ASLoggerApacheST(server_rec * rsIRequest): rsRequest(rsIRequest)
{
	SetPriority(rsRequest -> loglevel);
}

//
// Write message to log file
//
INT_32 ASLoggerApacheST::WriteLog(const UINT_32  iPriority, CCHAR_P szString, const UINT_32  iStringLen)
{
	int iApachePriority = AS_LOG_EMERG;
	switch (iPriority)
	{
		case AS_LOG_EMERG:   iApachePriority = APLOG_EMERG;   break;
		case AS_LOG_ALERT:   iApachePriority = APLOG_ALERT;   break;
		case AS_LOG_CRIT:    iApachePriority = APLOG_CRIT;    break;
		case AS_LOG_ERR:     iApachePriority = APLOG_ERR;     break;
		case AS_LOG_WARNING: iApachePriority = APLOG_WARNING; break;
		case AS_LOG_NOTICE:  iApachePriority = APLOG_NOTICE;  break;
		case AS_LOG_INFO:    iApachePriority = APLOG_INFO;    break;
		case AS_LOG_DEBUG:   iApachePriority = APLOG_DEBUG;   break;
		default:
			;;
	}
	ap_log_error(APLOG_MARK, APLOG_NOERRNO | iApachePriority, rsRequest, "CAS: %.*s", iStringLen, szString);

return 0;
}

//
// A destructor
//
ASLoggerApacheST::~ASLoggerApacheST() throw() { ;; }

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ASLoggerApache
//

//
// Constructor
//
ASLoggerApache::ASLoggerApache(request_rec * rsIRequest): rsRequest(rsIRequest)
{
	SetPriority(rsRequest -> server -> loglevel);
}

//
// Write message to log file
//
INT_32 ASLoggerApache::WriteLog(const UINT_32  iPriority, CCHAR_P szString, const UINT_32  iStringLen)
{
	int iApachePriority = AS_LOG_EMERG;
	switch (iPriority)
	{
		case AS_LOG_EMERG:   iApachePriority = APLOG_EMERG;   break;
		case AS_LOG_ALERT:   iApachePriority = APLOG_ALERT;   break;
		case AS_LOG_CRIT:    iApachePriority = APLOG_CRIT;    break;
		case AS_LOG_ERR:     iApachePriority = APLOG_ERR;     break;
		case AS_LOG_WARNING: iApachePriority = APLOG_WARNING; break;
		case AS_LOG_NOTICE:  iApachePriority = APLOG_NOTICE;  break;
		case AS_LOG_INFO:    iApachePriority = APLOG_INFO;    break;
		case AS_LOG_DEBUG:   iApachePriority = APLOG_DEBUG;   break;
		default:
			;;
	}
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | iApachePriority, rsRequest, "CAS: %.*s", iStringLen, szString);

return 0;
}

//
// A destructor
//
ASLoggerApache::~ASLoggerApache() throw() { ;; }


// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ApacheOutputCollector
//

//
// Constructor
//
ApacheOutputCollector::ApacheOutputCollector(request_rec * rsIRequest): rsRequest(rsIRequest)
{
	;;
}

//
// Collect data
//
INT_32 ApacheOutputCollector::Collect(const void * vData, const UINT_32 iDataLength)
{
	ap_rwrite(vData, iDataLength, rsRequest);

return 0;
}

//
// A destructor
//
ApacheOutputCollector::~ApacheOutputCollector() throw() { ;; }


// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ASApacheResponseWriter
//

//
// Constructor
//
ASApacheResponseWriter::ASApacheResponseWriter(request_rec * rsIRequest): rsRequest(rsIRequest), oOutputCollector(rsRequest)
{
	;;
}

//
// Write HTTP header
//
INT_32 ASApacheResponseWriter::WriteHeader(ASResponse & oResponse)
{
	// Set status line
	if (oResponse.GetHTTPStatusLine().empty())
	{
		rsRequest -> status = oResponse.GetHTTPResponseCode();
		rsRequest -> status_line = GetResponseCode(rsRequest -> status);
	}
	else
	{
		rsRequest -> status = oResponse.GetHTTPResponseCode();
		rsRequest -> status_line = oResponse.GetHTTPStatusLine().c_str();
	}

	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, rsRequest, "%s", rsRequest -> status_line);
	// Set content type
	const STLW::string & sContentType = oResponse.GetContentType();
	if (!sContentType.empty())
	{
		rsRequest -> content_type = ap_pstrdup(rsRequest -> pool, sContentType.data());
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, rsRequest, "Content-type: %s", rsRequest -> content_type);
	}

	ap_table_add(rsRequest -> headers_out, "X-Powered-By", "C++ Application Server v" AS_VERSION "(" AS_IDENT ")");

	// Write headers
	STLW::vector<STLW::pair<STLW::string, STLW::string> > & vHeaders = oResponse.GetHeaders();
	STLW::vector<STLW::pair<STLW::string, STLW::string> >::const_iterator itvHeaders = vHeaders.begin();
	while (itvHeaders != vHeaders.end())
	{
		if (itvHeaders -> first.size() != 0 && itvHeaders -> second.size() != 0)
		{
//			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, rsRequest, "%s: %s", itvHeaders -> first.data(), itvHeaders -> second.data());
			ap_table_add(rsRequest -> headers_out, itvHeaders -> first.data(), itvHeaders -> second.data());
		}
		++itvHeaders;
	}

	// Write Cookies
	STLW::vector<ASCookie> & vCookies = oResponse.GetCookies();
	STLW::vector<ASCookie>::iterator itvCookies = vCookies.begin();
	while(itvCookies != vCookies.end())
	{
		ap_table_add(rsRequest -> headers_out, "Set-Cookie", itvCookies -> GetCookie());
//		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, rsRequest, "Set-Cookie: %s", itvCookies -> GetCookie());
		++itvCookies;
	}

	// Send HTTP header to client
	ap_send_http_header(rsRequest);

return 0;
}

//
// Get output data collector
//
CTPP::OutputCollector * ASApacheResponseWriter::GetBodyOutputCollector() { return &oOutputCollector; }

//
// A destructor
//
ASApacheResponseWriter::~ASApacheResponseWriter() throw()
{
	;;
}

} // namespace CAS
// End.
