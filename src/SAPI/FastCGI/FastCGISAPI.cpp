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
 *      FastCGISAPI.cpp
 *
 * $CAS$
 */
// C++ Includes
#include "FastCGI.hpp"
#include "FastCGIIO.hpp"
#include "FastCGISAPI.hpp"
#include "FastCGIScoreboard.hpp"
#include "FastCGIWorkerContext.hpp"

#include "ASDebugHelper.hpp"
#include "ASLoggerFile.hpp"
#include "ASResponse.hpp"
#include "ASResponseCodes.hpp"

#ifndef C_HTTP_LINE_BUFFER_SIZE
#define C_HTTP_LINE_BUFFER_SIZE 8192
#endif // C_HTTP_LINE_BUFFER_SIZE

namespace CAS // C++ Application Server
{
// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class OutputCollector
//

//
// Constructor
//
FastCGIOutputCollector::FastCGIOutputCollector(FastCGIIO     & oIFastCGIIO,
                                               const INT_32    iIReqId): oFastCGIIO(oIFastCGIIO),
                                                                         iReqId(iIReqId),
                                                                         iOutputBufferSize(0xFFF8),
                                                                         vOutputBuffer(NULL),
                                                                         iOutputBufferPos(0)
{
	vOutputBuffer = malloc(iOutputBufferSize);
}

//
// Collect data
//
INT_32 FastCGIOutputCollector::Collect(const void     * vData,
                                       const UINT_32    iDataLen)
{
	if (iOutputBufferPos + iDataLen >= iOutputBufferSize)
	{
		if (iOutputBufferPos != 0)
		{
			const INT_32 iBytes = oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, vOutputBuffer, iOutputBufferPos);
			if (iBytes == -1) { return -1; }

			iOutputBufferPos = 0;
		}

		if (iDataLen >= iOutputBufferSize)
		{
			return oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, vData, iDataLen);
		}
	}

	memcpy(CHAR_P(vOutputBuffer) + iOutputBufferPos, vData, iDataLen);
	iOutputBufferPos += iDataLen;
return 0;
}

//
// Flush buffer
//
INT_32 FastCGIOutputCollector::Flush()
{
	if (iOutputBufferPos == 0) { return 0; }

	const INT_32 iBytes = oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, vOutputBuffer, iOutputBufferPos);
	iOutputBufferPos = 0;

return iBytes;
}

//
// A destructor
//
FastCGIOutputCollector::~FastCGIOutputCollector() throw()
{
	free(vOutputBuffer);
}

// ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class FastCGIResponseWriter
//

//
// Constructor
//
FastCGIResponseWriter::FastCGIResponseWriter(WorkerContext  & oIWorkerContext,
                                             FastCGIIO      & oIFastCGIIO,
                                             const INT_32     iReqId): oWorkerContext(oIWorkerContext),
                                                                       oFastCGIIO(oIFastCGIIO),
                                                                       oOutputCollector(oFastCGIIO, iReqId)
{
	;;
}

//
// Write HTTP header
//
INT_32 FastCGIResponseWriter::WriteHeader(ASResponse & oResponse)
{
	ASLogger & oErrorLog = *oWorkerContext.error_log;
	DEBUG_HELPER(&oErrorLog, "FastCGIResponseWriter::WriteHeader");

	// Set WRITE_RESPONSE state
	Scoreboard::ScoreboardRec oRec;
	oWorkerContext.scoreboard.ReadScoreboard(oWorkerContext.scoreboard_pos, oRec);
	oRec.state = Scoreboard::ScoreboardRec::WRITE_RESPONSE;
	oWorkerContext.scoreboard.WriteScoreboard(oWorkerContext.scoreboard_pos, oRec);

	CHAR_8 szTMPBuf[C_HTTP_LINE_BUFFER_SIZE + 1];

	const STLW::string sStatusLine = oResponse.GetHTTPStatusLine();
	// Set status line
	if (sStatusLine.empty())
	{
		const INT_32 iLen = snprintf(szTMPBuf, C_HTTP_LINE_BUFFER_SIZE, "Status: %s\r\n", GetResponseCode(oResponse.GetHTTPResponseCode()));
		oOutputCollector.Collect(szTMPBuf, iLen);

		oErrorLog.Info("HTTP: %u(%s)", oResponse.GetHTTPResponseCode(), GetResponseCode(oResponse.GetHTTPResponseCode()));
	}
	else
	{
		const INT_32 iLen = snprintf(szTMPBuf, C_HTTP_LINE_BUFFER_SIZE, "Status: %.*s\r\n", INT_32(sStatusLine.size()), sStatusLine.data());
		oOutputCollector.Collect(szTMPBuf, iLen);

		oErrorLog.Info("HTTP: %s", sStatusLine.data());
	}

	const STLW::string & sContentType = oResponse.GetContentType();
	if (!sContentType.empty())
	{
		INT_32 iLen = snprintf(szTMPBuf, C_HTTP_LINE_BUFFER_SIZE, "Content-Type: %.*s\r\n", INT_32(sContentType.size()), sContentType.data());
		oOutputCollector.Collect(szTMPBuf, iLen);

		oErrorLog.Info("Content-Type: %.*s", INT_32(sContentType.size()), sContentType.data());
	}

	INT_32 iLen = snprintf(szTMPBuf, C_HTTP_LINE_BUFFER_SIZE, "%s\r\n", "X-Powered-By: C++ Application Server v" AS_VERSION "(" AS_IDENT ")");
	oOutputCollector.Collect(szTMPBuf, iLen);

	// Write headers
	STLW::vector<STLW::pair<STLW::string, STLW::string> > & vHeaders = oResponse.GetHeaders();
	STLW::vector<STLW::pair<STLW::string, STLW::string> >::const_iterator itvHeaders = vHeaders.begin();
	while (itvHeaders != vHeaders.end())
	{
		if (itvHeaders -> first.size() != 0 && itvHeaders -> second.size() != 0)
		{
			INT_32 iLen = snprintf(szTMPBuf, C_HTTP_LINE_BUFFER_SIZE, "%.*s: %.*s\r\n",
			                                                          INT_32(itvHeaders -> first.size()),  itvHeaders -> first.data(),
			                                                          INT_32(itvHeaders -> second.size()), itvHeaders -> second.data());
			oOutputCollector.Collect(szTMPBuf, iLen);

			oErrorLog.Info("%s: %s", itvHeaders -> first.data(), itvHeaders -> second.data());
		}
		++itvHeaders;
	}

	const UINT_64 iContentLength = oResponse.GetContentLength();
	if (iContentLength != 0)
	{
		iLen = snprintf(szTMPBuf, C_HTTP_LINE_BUFFER_SIZE, "Content-Length: %llu\r\n", (long long unsigned)iContentLength);
		oOutputCollector.Collect(szTMPBuf, iLen);

		oErrorLog.Info("Content-Length: %llu", (long long unsigned)iContentLength);
	}

	// Write Cookies
	STLW::vector<ASCookie> & vCookies = oResponse.GetCookies();
	STLW::vector<ASCookie>::iterator itvCookies = vCookies.begin();
	while(itvCookies != vCookies.end())
	{
		const STLW::string & sCookie = itvCookies -> GetCookie();
		INT_32 iLen = snprintf(szTMPBuf, C_HTTP_LINE_BUFFER_SIZE, "Set-Cookie: %.*s\r\n",
		                                                          INT_32(sCookie.size()), sCookie.data());
		oOutputCollector.Collect(szTMPBuf, iLen);

		oErrorLog.Info("Set-Cookie: %s", itvCookies -> GetCookie());
		++itvCookies;
	}

	// Send end-of-HTTP headers to client
	oOutputCollector.Collect("\r\n", 2);

return 0;
}

//
// Flush buffer
//
INT_32 FastCGIResponseWriter::Flush() { return oOutputCollector.Flush(); }

//
// Get output data collector
//
CTPP::OutputCollector * FastCGIResponseWriter::GetBodyOutputCollector() { return &oOutputCollector; }

//
// A destructor
//
FastCGIResponseWriter::~FastCGIResponseWriter() throw() { ;; }

} // namespace CAS
// End.
