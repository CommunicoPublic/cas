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
 *      FastCGIWorker.cpp
 *
 * $CAS$
 */
#include "ASBSONRequestParser.hpp"
#include "ASDebugHelper.hpp"
#include "ASHostConfig.hpp"
#include "ASJSONRequestParser.hpp"
#include "ASLogger.hpp"
#include "ASMultipartParser.hpp"
#include "ASRawDataParser.hpp"
#include "ASRequest.hpp"
#include "ASSAPIUtil.hpp"
#include "ASScopedPtr.hpp"
#include "ASServer.hpp"
#include "ASServerManager.hpp"
#include "ASUrlencodedParser.hpp"
#include "ASUtil.hpp"
#include "ASXMLRequestParser.hpp"
#include "ASXMLRPCHandler.hpp"
#include "ASXMLRPCRequestParser.hpp"
#include "ASXMLParser.hpp"

#include "FastCGI.hpp"
#include "FastCGIIO.hpp"
#include "FastCGISAPI.hpp"
#include "FastCGIWorker.hpp"
#include "FastCGIServerContext.hpp"
#include "FastCGIWorkerContext.hpp"
#include "FastCGISetProcTitle.h"

#include <CTPP2Util.hpp>

#include <sysexits.h>
#include <unistd.h>

#define DEFAULT_ENCTYPE           "application/x-www-form-urlencoded"
#define MULTIPART_ENCTYPE         "multipart/form-data"
#define TEXT_XML_ENCTYPE          "text/xml"
#define APPLICATION_JSON_ENCTYPE  "application/json"
#define APPLICATION_BSON_ENCTYPE  "application/bson"

#define C_POST_NO_ERROR            0
#define C_MAX_POST_SIZE_REACHED   -1
#define C_PRE_REQUEST_FAILED      -2
#define C_REQUEST_HANDLER_FAILED  -3

namespace CAS // C++ Application Server
{
//
//  Constructor
//
WorkerProcess::WorkerProcess(WorkerContext & oIWorkerContext): oWorkerContext(oIWorkerContext)
{
	vReadBuffer = (CHAR_P)malloc(oWorkerContext.read_buffer_size);
}

//
// Handle FastCGI connection
//
INT_32 WorkerProcess::HandleConnection(const INT_32  iClientSocket)
{
	using namespace CTPP;

	ASLogger & oErrorLog    = *oWorkerContext.error_log;
	ASLogger & oTransferLog = *oWorkerContext.transfer_log;
	DEBUG_HELPER(&oErrorLog, "WorkerProcess::HandleConnection");

	ServerContext  & oServerContext = *oWorkerContext.server_context;

	ASRequest oASRequest;
	oASRequest.files          = ASFilePool();
	oASRequest.headers        = CDT(CDT::HASH_VAL);
	oASRequest.cookies        = CDT(CDT::HASH_VAL);
	oASRequest.arguments      = CDT(CDT::HASH_VAL);
	oASRequest.uri_components = CDT(CDT::HASH_VAL);

	// FastCGI I/O object
	FastCGIIO oFastCGIIO(iClientSocket, oWorkerContext);

	// Uploaded files
	STLW::vector<STLW::string>             vFileList;
	// POST request parser
	ScopedPtr<ASRequestParser>             pPOSTParser(NULL);
	// Location Id
	INT_32                                 iLocationId = -1;
	// Location request context
	ScopedPtr<ASServer::ASRequestContext>  pRequestContext(NULL);

	CHAR_8 szProcTitle[1024];

	UINT_64 iBytesRead    = oWorkerContext.bytes_read;
	UINT_64 iBytesWritten = oWorkerContext.bytes_written;
	INT_64  iPOSTRead     = 0;
	INT_64  iMaxPOSTSize  = 0;

	UINT_32 iReqState     = 0;
	INT_32  iReqId        = 0;
	Scoreboard::ScoreboardRec oRec;
	for(;;)
	{
		oWorkerContext.scoreboard.ReadScoreboard(oWorkerContext.scoreboard_pos, oRec);
		oRec.uri[0] = '\0';
		oRec.state  = Scoreboard::ScoreboardRec::READ_REQUEST;
		oWorkerContext.scoreboard.WriteScoreboard(oWorkerContext.scoreboard_pos, oRec);

		INT_32       iPacketType = 0;
		INT_32       iRequestId  = 0;
		const INT_32 iPacketLen  = oFastCGIIO.ReadRequest(iPacketType, iRequestId, vReadBuffer, oWorkerContext.read_buffer_size);
		if      (iPacketType == FastCGIHeader::FCGI_BEGIN_REQUEST)
		{
			DEBUG_HELPER_MESSAGE("FCGI_BEGIN_REQUEST");

			if (iPacketLen != sizeof(FastCGIBeginRequest))
			{
				oErrorLog.Error("Invalid FCGI_BEGIN_REQUEST packet");
				return -1;
			}

			iReqId = iRequestId;

			FastCGIBeginRequest & oBeginRequest = *((FastCGIBeginRequest *)vReadBuffer);
			const UINT_32 iRole = UINT_32(oBeginRequest.role_b1 << 8) + oBeginRequest.role_b0;
			if (iRole == FastCGIBeginRequest::FCGI_RESPONDER)
			{
				oErrorLog.Debug("FCGI_RESPONDER is one supported role");
			}
			else if (iRole == FastCGIBeginRequest::FCGI_AUTHORIZER)
			{
				oErrorLog.Error("FCGI_AUTHORIZER role not supported");
				return UnknownRole(oFastCGIIO, iReqId, iRole);

			}
			else if (iRole == FastCGIBeginRequest::FCGI_FILTER)
			{
				oErrorLog.Error("FCGI_FILTER role not supported");
				return UnknownRole(oFastCGIIO, iReqId, iRole);
			}
			else
			{
				oErrorLog.Error("Invalid role: 0x%04X", iRole);
				return -1;
			}
		}
		else if (iPacketType == FastCGIHeader::FCGI_PARAMS)
		{
			DEBUG_HELPER_MESSAGE("FCGI_PARAMS");

			if (iPacketLen != 0)
			{
				ParseParams(vReadBuffer, iPacketLen, oASRequest.headers);
			}
			else
			{
				// Parse request method
				static const UINT_32 aMethods[] = {
				                                    ASRequest::GET,   ASRequest::POST,    ASRequest::OPTIONS,  ASRequest::HEAD,
				                                    ASRequest::TRACE, ASRequest::CONNECT, ASRequest::PROPFIND, ASRequest::PROPPATCH,
				                                    ASRequest::MKCOL, ASRequest::DELETE,  ASRequest::PUT,      ASRequest::COPY,
				                                    ASRequest::MOVE,  ASRequest::LOCK,    ASRequest::UNLOCK,   ASRequest::GET
				                                  };

				static CCHAR_P       sMethods[] = {
				                                    "GET",   "POST",    "OPTIONS",  "HEAD",
				                                    "TRACE", "CONNECT", "PROPFIND", "PROPPATCH",
				                                    "MKCOL", "DELETE",  "PUT",      "COPY",
				                                    "MOVE",  "LOCK",    "UNLOCK",    NULL
				                                  };

				const STLW::string sReqMethod = oASRequest.headers.GetCDT("Request-Method").GetString();

				const UINT_32 * aMethod = aMethods;
				CCHAR_P       * sMethod = sMethods;
				while (*sMethod != NULL)
				{
					if (strcasecmp(sReqMethod.c_str(), *sMethod) == 0) { break; }
					++aMethod; ++sMethod;
				}
				oASRequest.request_method = *aMethod;

				// Parse query string
				const STLW::string sQueryString = oASRequest.headers.GetCDT("Query-String").GetString();
				if (!sQueryString.empty())
				{
					UrlencodedParser oParser(oASRequest.arguments);
					oParser.ParseChunk(sQueryString.data(), sQueryString.data() + sQueryString.size());
					oParser.ParseDone();
				}

				// Parse cookies
				const STLW::string sCookie = oASRequest.headers.GetCDT("Cookie").GetString();
				ParseCookies(sCookie.c_str(), oASRequest.cookies);

				// Store data
				oASRequest.uri            = oASRequest.headers.GetCDT("Document-Uri").GetString();
				oASRequest.unparsed_uri   = oASRequest.headers.GetCDT("Request-Uri").GetString();
				oASRequest.remote_ip      = oASRequest.headers.GetCDT(oWorkerContext.config.realip_header).GetString();
				if (oASRequest.remote_ip.empty()) { oASRequest.remote_ip = oASRequest.headers.GetCDT("Remote-Addr").GetString(); }

				oASRequest.referer        = oASRequest.headers.GetCDT("Referer").GetString();
				oASRequest.user_agent     = oASRequest.headers.GetCDT("User-Agent").GetString();
				oASRequest.content_length = atoll(oASRequest.headers.GetCDT("Content-Length").GetString().c_str());
				oASRequest.request_type   = ASRequest::HTTP_REQUEST;

				snprintf(szProcTitle, 1024, "cas-fcgid: %s %s", sReqMethod.c_str(), oASRequest.uri.c_str());
				_setproctitle(szProcTitle);

				oASRequest.headers.Erase("Request-Uri");
				oASRequest.headers.Erase("Document-Uri");
				oASRequest.headers.Erase("Remote-Addr");
				oASRequest.headers.Erase("Request-Method");
				oASRequest.headers.Erase("Query-String");

				const STLW::string sHTTPHost = oASRequest.headers.GetCDT("Host").GetString();
				if (!sHTTPHost.empty())
				{
					const size_t iPos = sHTTPHost.find(':');
					if (iPos == STLW::string::npos)
					{
						oASRequest.host.assign(sHTTPHost);
						oASRequest.port = 80;
					}
					else
					{
						oASRequest.host.assign(sHTTPHost, 0, iPos);
						oASRequest.port = atoi(sHTTPHost.c_str() + iPos + 1);
					}
				}

				// Get Auth status
				const STLW::string sAuthorization = oASRequest.headers.GetCDT("Authorization").GetString();
				if (!sAuthorization.empty())
				{
					//       123456
					// Find "Basic "
					CCHAR_P szTMP = StrCaseStr(sAuthorization.c_str(), "Basic ");
					if (szTMP != NULL)
					{
						STLW::string sDecoded = Base64Decode(szTMP);
						STLW::string::size_type iPos = sDecoded.find(':');
						if (iPos != STLW::string::npos)
						{
							oASRequest.user.assign(sDecoded, 0, iPos);
							oASRequest.password.assign(sDecoded, iPos + 1, sDecoded.size() - iPos);
						}
					}
				}

				if (oASRequest.uri != oWorkerContext.config.status_uri)
				{
					// Check location
					iLocationId = oServerContext.server -> CheckLocationURI(oASRequest.uri.c_str(), oASRequest, &oErrorLog);
					if (iLocationId != -1)
					{
						// Host configuration
						ASHostConfig  * pASHostConfig = oServerContext.server -> GetConfig();
						// Location configuration
						ASLocation    & oASLocation   = pASHostConfig -> locations[iLocationId];

						// Request context
						pRequestContext = new ASServer::ASRequestContext(iLocationId, oASLocation);
						// Parse arguments
						oServerContext.server -> NotifyObjects(C_START_OF_PRE_REQUEST);

						// Create POST request parser
						pPOSTParser  = CreateParser(oASRequest, vFileList, iLocationId, pRequestContext);
						iPOSTRead    = 0;
						iMaxPOSTSize = oASLocation.max_post_size;
					}
				}

				// Headers completed
				iReqState |= 0x01;
			}
		}
		else if (iPacketType == FastCGIHeader::FCGI_STDIN)
		{
			DEBUG_HELPER_MESSAGE("FCGI_STDIN");

			iPOSTRead += iPacketLen;
			if (iLocationId != -1)
			{
				if (iMaxPOSTSize == -1 || iPOSTRead < iMaxPOSTSize)
 				{
					oServerContext.server -> HandlePreRequest(*pRequestContext, vReadBuffer, iPacketLen, oASRequest, &oErrorLog);
					if (pPOSTParser)
					{
						pPOSTParser -> ParseChunk(vReadBuffer, vReadBuffer + iPacketLen);
					}
				}
			}

			if (iPacketLen == 0)
			{
				if (iLocationId != -1)
				{
					if (pPOSTParser) { pPOSTParser -> ParseDone(); }
					oServerContext.server -> NotifyObjects(C_END_OF_PRE_REQUEST);
				}

				// Request body completed
				iReqState |= 0x02;
			}
		}
		else if (iPacketType == FastCGIHeader::FCGI_GET_VALUES)
		{
			DEBUG_HELPER_MESSAGE("FCGI_GET_VALUES");

			UINT_32 iMsgLen  = 0;
			CHAR_P vBuf      = vReadBuffer;
			UINT_32 iBufSize = oWorkerContext.read_buffer_size;
			iMsgLen += BuildGetValueResponse("FCGI_MAX_CONNS",  oWorkerContext.config.max_clients, vBuf, iBufSize);
			iMsgLen += BuildGetValueResponse("FCGI_MAX_REQS",   oWorkerContext.config.max_clients, vBuf, iBufSize);
			iMsgLen += BuildGetValueResponse("FCGI_MPXS_CONNS", 0,                                 vBuf, iBufSize);

			if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_GET_VALUES_RESULT, iReqId, vReadBuffer, iMsgLen) == -1)
			{
				oErrorLog.Error("Can't write FCGI_GET_VALUES_RESULT");
				return -1;
			}
		}
		else
		{
			DEBUG_HELPER_MESSAGE("FCGI_INVALID_PACKET");

			oErrorLog.Error("Invalid packet type: 0x%04X", iPacketType);
			return -1;
		}

		// Execute request
		if (iReqState == 0x03)
		{
			DEBUG_HELPER_MESSAGE("Setup request");

			if (oWorkerContext.config.foreground && oWorkerContext.config.debug)
			{
				fprintf(stderr, "Args: %s\n",    oASRequest.arguments.Dump().c_str());
				fprintf(stderr, "Headers: %s\n", oASRequest.headers.Dump().c_str());
				fprintf(stderr, "Cookies: %s\n", oASRequest.cookies.Dump().c_str());
			}

			iReqState = 0;

			// Format query string
			oWorkerContext.scoreboard.ReadScoreboard(oWorkerContext.scoreboard_pos, oRec);
			FormatQueryString(oASRequest.request_type, oASRequest.request_method, oASRequest.unparsed_uri.c_str(), oRec.uri);
			oRec.state = Scoreboard::ScoreboardRec::RUN;
			oWorkerContext.scoreboard.WriteScoreboard(oWorkerContext.scoreboard_pos, oRec);

			if (oASRequest.uri == oWorkerContext.config.status_uri)
			{
				DEBUG_HELPER_MESSAGE("Server-status location");

				oErrorLog.Info("Generating server-status");

				CCHAR_P szOK = "Status: 200 OK\r\nContent-Type: text/xml; charset=utf-8\r\n\r\n<?xml version=\"1.0\" ?>\r\n<status>\r\n";
				if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, szOK, strlen(szOK)) == -1)
				{
					oErrorLog.Error("Can't write FCGI_STDOUT");
					return -1;
				}

				for (INT_32 iScoreboardPos = 0; iScoreboardPos < oWorkerContext.config.max_clients; ++iScoreboardPos)
				{
					Scoreboard::ScoreboardRec oRec;
					oWorkerContext.scoreboard.ReadScoreboard(iScoreboardPos, oRec);

					if (oRec.pid != 0)
					{
						CHAR_8 szStatInfo[1024];
						INT_32 iStatInfoLen = snprintf(szStatInfo, 1023, "\t<pid>%u</pid>"
						                                                 "<s>%s</s>"
						                                                 "<reqs>%llu</reqs>"
						                                                 "<in>%llu</in>"
						                                                 "<out>%llu</out>"
						                                                 "<q>%s</q>\r\n",
						                                                 oRec.pid, oRec.GetState(), (long long unsigned)oRec.requests, (long long unsigned)oRec.bytes_read, (long long unsigned)oRec.bytes_written, oRec.uri);

						if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, szStatInfo, iStatInfoLen) == -1)
						{
							oErrorLog.Error("Can't write FCGI_STDOUT");
							return -1;
						}
					}
				}

				CCHAR_P szEnd = "</status>\r\n";
				if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, szEnd, strlen(szEnd)) == -1)
				{
					oErrorLog.Info("Can't write FCGI_STDOUT");
					return -1;
				}
			}
			else
			{
				// Check location Id
				if (iLocationId == -1)
				{
					DEBUG_HELPER_MESSAGE("Declining location");

					oErrorLog.Error("Location `%s` declined", oASRequest.uri.c_str());

					CCHAR_P szLocationDecliend = "Status: 404 Not Found\r\n\r\nLocation declined";
					if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, szLocationDecliend, strlen(szLocationDecliend)) == -1)
					{
						oErrorLog.Error("Can't write FCGI_STDOUT");
						return -1;
					}
				}
				else
				{
					DEBUG_HELPER_MESSAGE("Accepting location");

					oErrorLog.Info("Location `%s` accepted", oASRequest.uri.c_str());

					// Response writer
					FastCGIResponseWriter oResponseWriter(oWorkerContext, oFastCGIIO, iReqId);

					// Server time
					gettimeofday(&(oASRequest.server_time.timestamp), NULL);
					// Time
					const time_t iTime = oASRequest.server_time.timestamp.tv_sec;
					// Server local time
					localtime_r(&iTime, &(oASRequest.server_time.localtime));
					// Server GMT time
					gmtime_r(&iTime, &(oASRequest.server_time.gmtime));

					// Host configuration
					ASHostConfig  * pASHostConfig = oServerContext.server -> GetConfig();
					// Location configuration
					ASLocation    & oASLocation   = pASHostConfig -> locations[iLocationId];
					// Fill location name
					oASRequest.location_name      = oASLocation.name;

					if (oASRequest.request_type == ASRequest::XMLRPC_REQUEST  ||
					    oASRequest.request_type == ASRequest::JSONRPC_REQUEST ||
					    oASRequest.request_type == ASRequest::BSONRPC_REQUEST)
					{
						snprintf(szProcTitle, 1024, "cas-fcgid: %s %s %s %s",
						                            oASRequest.remote_ip.c_str(),
						                            ASRequest::MethodToStr(oASRequest.request_method),
						                            oASRequest.uri.c_str(),
						                            oASRequest.arguments.GetCDT("method").GetString().c_str());
					}
					else
					{
						snprintf(szProcTitle, 1024, "cas-fcgid: %s %s %s",
						                            oASRequest.remote_ip.c_str(),
						                            ASRequest::MethodToStr(oASRequest.request_method),
						                            oASRequest.uri.c_str());
					}

					_setproctitle(szProcTitle);

					// Check request method
					if ((oASRequest.request_method & oASLocation.allowed_methods) == 0)
					{
						oErrorLog.Error("Method not allowed for location `%s`", oASRequest.uri.c_str());

						CCHAR_P szMethodNotAllowed = "Status: 405 Method Not Allowed\r\n\r\nMethod Not Allowed";
						if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, szMethodNotAllowed, strlen(szMethodNotAllowed)) == -1)
						{
							oErrorLog.Error("Can't write FCGI_STDOUT");
							return -1;
						}
					}
					else
					{
						if (iMaxPOSTSize != -1 && iPOSTRead > iMaxPOSTSize)
						{
							oErrorLog.Error("Content-Length of %lld bytes exceeds the limit of %lld bytes", (long long)iPOSTRead, (long long)oASLocation.max_post_size);

							CCHAR_P szEntityTooLarge = "Status: 413 Request Entity Too Large\r\n\r\nRequest Entity Too Large";
							if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, szEntityTooLarge, strlen(szEntityTooLarge)) == -1)
							{
								oErrorLog.Error("Can't write FCGI_STDOUT");
								return -1;
							}
						}
						else
						{
							DEBUG_HELPER_MESSAGE("Handle request");

							// Handle request
							if (oServerContext.server -> HandleRequest(*pRequestContext, oASRequest, &oResponseWriter, &oErrorLog) == -1)
							{
								oErrorLog.Error("Fatal error in HandleRequest (location \"%s\")", oASRequest.uri.c_str());
							}

							// Flush data
							oResponseWriter.Flush();
						}
					}

					iBytesRead    = oWorkerContext.bytes_read - iBytesRead;
					iBytesWritten = oWorkerContext.bytes_written - iBytesWritten;

					struct timeval  oEndTS;
					gettimeofday(&oEndTS, NULL);
					const W_FLOAT dExecTime = (1.0 * (oEndTS.tv_sec - oASRequest.server_time.timestamp.tv_sec) +
					                           1.0 * (oEndTS.tv_usec - oASRequest.server_time.timestamp.tv_usec) / 1000000);

					if (oASRequest.request_type == ASRequest::XMLRPC_REQUEST  ||
					    oASRequest.request_type == ASRequest::JSONRPC_REQUEST ||
					    oASRequest.request_type == ASRequest::BSONRPC_REQUEST)
					{
						oTransferLog.Emerg("%s %s: %s/%s %s %s %f %llu %llu",
						                   oASRequest.remote_ip.c_str(),
						                   oASRequest.location_name.c_str(),
						                   ASRequest::RequestToStr(oASRequest.request_type),
						                   ASRequest::MethodToStr(oASRequest.request_method),
						                   oASRequest.uri.c_str(),
						                   oASRequest.arguments.GetCDT("method").GetString().c_str(),
						                   dExecTime,
						                   (long long unsigned)iBytesRead,
						                   (long long unsigned)iBytesWritten);
					}
					else
					{
						oTransferLog.Emerg("%s %s: %s/%s %s %f %llu %llu",
						                   oASRequest.remote_ip.c_str(),
						                   oASRequest.location_name.c_str(),
						                   ASRequest::RequestToStr(oASRequest.request_type),
						                   ASRequest::MethodToStr(oASRequest.request_method),
						                   oASRequest.uri.c_str(),
						                   dExecTime,
						                   (long long unsigned)iBytesRead,
						                   (long long unsigned)iBytesWritten);
					}
				}
			}

			DEBUG_HELPER_MESSAGE("Clear garbage after request");

			STLW::vector<STLW::string>::iterator itvFileList = vFileList.begin();
			while (itvFileList != vFileList.end())
			{
				// Try to unlink file; ignore errors
				unlink(itvFileList -> c_str());
				++itvFileList;
			}

			if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDOUT, iReqId, NULL, 0) == -1)
			{
				oErrorLog.Error("Can't write FCGI_STDOUT");
				return -1;
			}

//			if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_STDERR, iReqId, NULL, 0) == -1)
//			{
//				oErrorLog.Error("Can't write FCGI_STDERR");
//				return -1;
//			}

			++oRec.requests;
			oRec.bytes_read    = oWorkerContext.bytes_read;
			oRec.bytes_written = oWorkerContext.bytes_written;
			oWorkerContext.scoreboard.WriteScoreboard(oWorkerContext.scoreboard_pos, oRec);

			FastCGIEndRequest oEndRequest;
			oEndRequest.app_status_b3 = 0;
			oEndRequest.app_status_b2 = 0;
			oEndRequest.app_status_b1 = 0;
			oEndRequest.app_status_b0 = 0;
			oEndRequest.protocol_status = FastCGIEndRequest::FCGI_REQUEST_COMPLETE;
			if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_END_REQUEST, iReqId, &oEndRequest, sizeof(FastCGIEndRequest)) == -1)
			{
				oErrorLog.Error("Can't write FCGI_END_REQUEST");
				return -1;
			}

			DEBUG_HELPER_MESSAGE("Request completed");
			return 0;
		}
	}
return 0;
}

//
// Create POST request
//
ASRequestParser * WorkerProcess::CreateParser(ASRequest                   & oASRequest,
                                              STLW::vector<STLW::string>  & vFileList,
                                              const INT_32                  iLocationId,
                                              ASServer::ASRequestContext  * pRequestContext)
{
	ServerContext  & oServerContext = *oWorkerContext.server_context;
	if (iLocationId == -1) { return NULL; }

	ASLogger  & oErrorLog = *oWorkerContext.error_log;

	// POST
	if (oASRequest.request_method == ASRequest::POST)
	{
		static CCHAR_P szBoundaryPrefix = "\r\n--";

		// Host configuration
		ASHostConfig   * pASHostConfig  = oServerContext.server -> GetConfig();
		// Location configuration
		ASLocation     & oASLocation    = pASHostConfig -> locations[iLocationId];

		// Get content type
		CCHAR_P szContentType = oASRequest.headers.GetCDT("Content-Type").GetString().c_str();
		// If Content type not set, revert it to default
		if (szContentType == NULL || *szContentType == '\0') { szContentType = DEFAULT_ENCTYPE; }

		// foo=bar&baz=boo
		CCHAR_P szFoundContentType = NULL;

		// URL-encoded data
		if      ((szFoundContentType = StrCaseStr(szContentType, DEFAULT_ENCTYPE))   != NULL)
		{
			return new UrlencodedParser(oASRequest.arguments);
		}

		// Multipart message
		if ((szFoundContentType = StrCaseStr(szContentType, MULTIPART_ENCTYPE)) != NULL)
		{
			oASRequest.request_type = ASRequest::HTTP_RFC1867_REQUEST;

			// Get boundary
			CCHAR_P szTMPBoundary = StrCaseStr(szFoundContentType, "; boundary=");
			if (szTMPBoundary == NULL)
			{
				oErrorLog.Error("Read POST(" MULTIPART_ENCTYPE "): invalid boundary");
				return NULL;
			}

			// Boundary
			STLW::string sBoundary(szBoundaryPrefix);
			sBoundary.append(szTMPBoundary);

			return new MultipartParser(oASRequest.arguments,
			                               oASRequest.files,
			                               vFileList,
			                               pASHostConfig -> tempfile_dir,
			                               sBoundary,
			                               oASLocation.max_file_size,
			                               *(oServerContext.server),
			                               *pRequestContext,
			                               oASRequest,
			                               oErrorLog);
		}

		// XML RPC POST data
		if ((szFoundContentType = StrCaseStr(szContentType, TEXT_XML_ENCTYPE)) != NULL)
		{
			oASRequest.request_type = ASRequest::XMLRPC_REQUEST;
			// TBD: Write XML Parser factory class
			return new ASXMLRPCRequestParser(oASRequest.arguments);
		}

		// JSON POST data
		if ((szFoundContentType = StrCaseStr(szContentType, APPLICATION_JSON_ENCTYPE)) != NULL)
		{
			oASRequest.request_type = ASRequest::JSONRPC_REQUEST;
			return new ASJSONRequestParser(oASRequest.arguments);
		}

		// BSON POST Data
		if ((szFoundContentType = StrCaseStr(szContentType, APPLICATION_BSON_ENCTYPE)) != NULL)
		{
			oASRequest.request_type = ASRequest::BSONRPC_REQUEST;
			return new ASBSONRequestParser(oASRequest.arguments);
		}

		// Read raw data
		return new ASRawDataParser(oASRequest.raw_post_data);
	}

return NULL;
}

//
// Get parameter length
//
INT_32 WorkerProcess::GetParamLen(UCCHAR_P  & sFrame,
                                  UCCHAR_P    sEndBuffer)
{
	UINT_32 iVal = *sFrame++;
	if (iVal < 0x80) { return iVal; }

	if (sFrame + 3 >= sEndBuffer) { return -1; }

	iVal = ((iVal & 0x7F) << 24) + (sFrame[0] << 16) + (sFrame[1] << 8) + sFrame[2];
	sFrame += 3;

return iVal;
}

//
// Parse params
//
INT_32 WorkerProcess::ParseParams(const void     * vReadBuffer,
                                  const UINT_32    iFullLen,
                                  CTPP::CDT      & mParams)
{
	UCCHAR_P sFrame     = UCCHAR_P(vReadBuffer);
	UCCHAR_P sEndBuffer = sFrame + iFullLen;

	for(;;)
	{
		if (sFrame >= sEndBuffer) { break; }
		const INT_32 iKeyLen = GetParamLen(sFrame, sEndBuffer);
		if (iKeyLen < 0) { return -1; }

		if (sFrame >= sEndBuffer) { break; }
		const INT_32 iValLen = GetParamLen(sFrame, sEndBuffer);
		if (iValLen < 0) { return -1; }

		CHAR_P sKey = CHAR_P(sFrame);
		sFrame += iKeyLen;
		if (sFrame > sEndBuffer) { break; }

		CHAR_P sVal = CHAR_P(sFrame);
		sFrame += iValLen;
		if (sFrame > sEndBuffer) { break; }

		if (iKeyLen > 0)
		{
			CHAR_P  sIter    = sKey;
			CHAR_P  sIterEnd = sKey + iKeyLen;
			UCHAR_8 chBase   = 0x00;
			while (sIter != sIterEnd)
			{
				if (*sIter == '_') { *sIter = '-'; chBase = 0x00; }
				else
				{
					*sIter ^= chBase;
					chBase  = 0x20;
				}
				++sIter;
			}
			if (strncmp(sKey, "Http-", 5) == 0)
			{
				mParams[STLW::string(sKey + 5, iKeyLen - 5)] = STLW::string(sVal, iValLen);
			}
			else
			{
				mParams[STLW::string(sKey, iKeyLen)] = STLW::string(sVal, iValLen);
			}
		}
	}

return 0;
}

//
// Format query string
//
void WorkerProcess::FormatQueryString(const ASRequest::RequestType  eRequestType,
                                      const UINT_32                 iRequestMethod,
                                      CCHAR_P                       szUnparsedURI,
                                      CHAR_P                        sURI)
{
	static CCHAR_P szEscape = "0123456789ABCDEF";
	// x -> %XX
	CHAR_8 sEscapedURI[C_SCOREBOARD_URI_LEN + 3];

	INT_32 iPos = 0;
	for (;;)
	{
		CHAR_8 chTMP = *szUnparsedURI;
		if (chTMP == '\0' || iPos >= C_SCOREBOARD_URI_LEN) { break; }

		if ((chTMP >= 'a' && chTMP <= 'z') ||
		    (chTMP >= 'A' && chTMP <= 'Z') ||
		    (chTMP >= '0' && chTMP <= '9') ||
		     chTMP == '/' || chTMP == '.' || chTMP == '-' || chTMP == '_')
		{
			sEscapedURI[iPos++] = chTMP;
		}
		else
		{
			sEscapedURI[iPos++] = '%';
			sEscapedURI[iPos++] = szEscape[((chTMP >> 4) & 0x0F)];
			sEscapedURI[iPos++] = szEscape[(chTMP & 0x0F)];
		}

		++szUnparsedURI;
	}

	snprintf(sURI, C_SCOREBOARD_URI_LEN - 1, "%s/%s %.*s",
	                                     ASRequest::RequestToStr(eRequestType),
	                                     ASRequest::MethodToStr(iRequestMethod),
	                                     iPos,
	                                     sEscapedURI);
	sURI[C_SCOREBOARD_URI_LEN - 1] = '\0';
}

//
// Build key-value header
//
INT_32 WorkerProcess::BuildKeyValueHeader(const UINT_32    iKeyLen,
                                          const UINT_32    iValLen,
                                          CHAR_P         & sBuffer,
                                          UINT_32        & iBufferLen)
{
	if(iKeyLen < 0x80)
	{
		if (iBufferLen < 1) { return -1; }

		*sBuffer++ = iKeyLen;
		--iBufferLen;
	}
	else
	{
		if (iBufferLen < 4) { return -1; }

		*sBuffer++ = (iKeyLen >> 24) | 0x80;
		*sBuffer++ = (iKeyLen >> 16);
		*sBuffer++ = (iKeyLen >> 8);
		*sBuffer++ = iKeyLen;

		iBufferLen -= 4;
	}

	if(iValLen < 0x80)
	{
		if (iBufferLen < 1) { return -1; }

		*sBuffer++ = iValLen;
		--iBufferLen;
	}
	else
	{
		if (iBufferLen < 4) { return -1; }

		*sBuffer++ = (iValLen >> 24) | 0x80;
		*sBuffer++ = (iValLen >> 16);
		*sBuffer++ = (iValLen >> 8);
		*sBuffer++ = iValLen;

		iBufferLen -= 4;
	}
return 0;
}

//
// Build FCGI_GET_VALUES_RESULT response
//
INT_32 WorkerProcess::BuildGetValueResponse(CCHAR_P          szKey,
                                            const UINT_32    iValue,
                                            CHAR_P         & sBuffer,
                                            UINT_32        & iBufferLen)
{
	UINT_32 iTMP = iBufferLen;
	CHAR_8 sTMP[16];
	const UINT_32 iKeyLen = strlen(szKey);
	const UINT_32 iValLen = snprintf(sTMP, 16, "%u", iValue);

	BuildKeyValueHeader(iKeyLen, iValLen, sBuffer, iBufferLen);
	memcpy(sBuffer, szKey, iKeyLen);
	memcpy(sBuffer + iKeyLen, sTMP, iValLen);

return iTMP - iBufferLen + iKeyLen + iValLen;
}

//
// Unknown role
//
INT_32 WorkerProcess::UnknownRole(FastCGIIO    & oFastCGIIO,
                                  const INT_32   iReqId,
                                  const INT_32   iRole)
{
	FastCGIEndRequest oEndRequest;
	oEndRequest.app_status_b3 = 0;
	oEndRequest.app_status_b2 = 0;
	oEndRequest.app_status_b1 = 0;
	oEndRequest.app_status_b0 = 0;
	oEndRequest.protocol_status = iRole;

	if (oFastCGIIO.WriteResponse(FastCGIHeader::FCGI_END_REQUEST, iReqId, &oEndRequest, sizeof(FastCGIEndRequest)) == -1)
	{
		return -1;
	}

return 0;
}

//
// A destructor
//
WorkerProcess::~WorkerProcess() throw()
{
	free(vReadBuffer);
}

} // namespace CAS
// End.
