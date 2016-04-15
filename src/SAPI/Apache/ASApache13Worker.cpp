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
#include "ASApache13Worker.h"
#include "ASApache13SAPI.hpp"

#include "ASDebugHelper.hpp"
#include "ASJSONRequestParser.hpp"
#include "ASLoggerFile.hpp"

#include "ASMultipartParser.hpp"
#include "ASUrlencodedParser.hpp"
#include "ASSAPIUtil.hpp"
#include "ASServer.hpp"
#include "ASServerManager.hpp"
#include "ASRequest.hpp"
#include "ASUtil.hpp"
#include "ASXMLRequestParser.hpp"
#include "ASRawDataParser.hpp"
#include "ASXMLRPCHandler.hpp"
#include "ASXMLParser.hpp"

#include <CTPP2Util.hpp>

#ifdef CR
#undef CR
#endif

#include "httpd.h"
#include "http_log.h"
#include "http_protocol.h"
#include "http_main.h"

using namespace CAS; // C++ Application Server

#define C_ESCAPE_BUFFER_LEN 8192

//
// Opaque server manager object
//
struct CASOpaqueServerManager
{
	// Server manager
	ASServerManager   * server_manager;
};

//
// Opaque server object
//
struct CASOpaqueServer
{
	// Server
	ASServer          * server;
	// Request object
	ASRequest         * request;
	// Location ID
	INT_32              location;
	// Uploaded files
	STLW::vector<STLW::string>   filelist;
};

#define DEFAULT_ENCTYPE    "application/x-www-form-urlencoded"
#define MULTIPART_ENCTYPE  "multipart/form-data"
#define TEXT_XML_ENCTYPE   "text/xml"
#define APPLICATION_JSON_ENCTYPE  "application/json"

#define C_POST_NO_ERROR            0
#define C_MAX_POST_SIZE_REACHED   -1
#define C_PRE_REQUEST_FAILED      -2
#define C_REQUEST_HANDLER_FAILED  -3

//
// Parse POST request
//
static int ParsePOST(request_rec                 * rsRequest,
                     CAS::ASRequestParser        * pParser,
                     CASOpaqueServer             * pServer,
                     ASServer::ASRequestContext  & oContext,
                     ASLogger                    & oLogger,
                     const INT_64                & iMaxPostSize)
{
	// Allocate memory block
	char   * pData      = (char *)ap_palloc(rsRequest -> pool, C_ESCAPE_BUFFER_LEN);
	INT_32   iDataSize  = 0;
	INT_32   iReadBytes = 0;
	INT_32   iCanRead   = 0;
	INT_32   iRC        = 0;

	// Set timeout for request
	ap_hard_timeout((char *)"ReadRequest", rsRequest);

	// Read data
	while ((iDataSize = ap_get_client_block(rsRequest, pData, C_ESCAPE_BUFFER_LEN)) > 0)
	{
		// Check max. post size
		if (iMaxPostSize != -1 && iReadBytes >= iMaxPostSize) { iCanRead = C_MAX_POST_SIZE_REACHED; }

		// Process data
		if (iCanRead == C_POST_NO_ERROR)
		{
			iRC = pServer -> server -> HandlePreRequest(oContext, pData, iDataSize, *(pServer -> request), &oLogger);
			if (iRC != 0) { iCanRead = C_PRE_REQUEST_FAILED; }

			pParser -> ParseChunk(pData, pData + iDataSize);
			iRC = pParser -> GetState();
			if (iRC != 0 && iRC != 1) { iCanRead = C_REQUEST_HANDLER_FAILED; }
		}

		// Read bytes
		iReadBytes += iDataSize;

		// Reset timeout
		ap_reset_timeout(rsRequest);
	}
	// Remove timeout
	ap_kill_timeout(rsRequest);

	// Commit process
	pParser -> ParseDone();
	iRC = pParser -> GetState();
	if (iRC != 0 && iRC != 1) { iCanRead = C_REQUEST_HANDLER_FAILED; }

	// All done
	switch(iCanRead)
	{
		case C_MAX_POST_SIZE_REACHED:
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, rsRequest, "CAS: POST Content-Length of %lld bytes exceeds the limit of %lld bytes", (long long)iReadBytes, (long long)iMaxPostSize);
			rsRequest -> status = HTTP_REQUEST_ENTITY_TOO_LARGE;
			ap_send_http_header(rsRequest);
			return HTTP_REQUEST_ENTITY_TOO_LARGE;

		case C_PRE_REQUEST_FAILED:
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, rsRequest, "CAS: POST Pre-request failed");
			rsRequest -> status = HTTP_INTERNAL_SERVER_ERROR;
			ap_send_http_header(rsRequest);
			return HTTP_INTERNAL_SERVER_ERROR;

		case C_REQUEST_HANDLER_FAILED:
			const STLW::string & sError = pParser -> GetError();
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, rsRequest, "CAS: POST Request handler failed: %s", sError.c_str());
			rsRequest -> status = HTTP_BAD_REQUEST;
			ap_send_http_header(rsRequest);
			ap_rwrite(sError.c_str(), sError.size(), rsRequest);
			return HTTP_BAD_REQUEST;
	}

return OK;
}

//
// Read request
//
static int ReadRequest(request_rec                    * rsRequest,
                       CASOpaqueServer                * pServer,
                       ASServer::ASRequestContext     & oContext,
                       ASLogger                       & oLogger)
{
	INT_32 iRC = OK;

	static CCHAR_P szBoundaryPrefix = "\r\n--";
	// GET, HEAD and POST

	// Get server configuration and check limits
	ASHostConfig * pConfig = pServer -> server -> GetConfig();

	// GET, HEAD and POST
	// URI components
	uri_components    oURI;

	// Parse URI Components
	ap_parse_uri_components(rsRequest -> pool, rsRequest -> unparsed_uri, &oURI);

	// Content-Length header
	pServer -> request -> content_length = -1;
	CCHAR_P szContentLength = ap_table_get(rsRequest -> headers_in, "Content-Length");
	if (szContentLength != NULL) { pServer -> request -> content_length = atoll(szContentLength); }

	// Get Referer field
	CCHAR_P szReferer = ap_table_get(rsRequest -> headers_in, "Referer");
	if (szReferer != NULL) { pServer -> request -> referer = szReferer; }

	// Get User-Agent field
	CCHAR_P szUserAgent = ap_table_get(rsRequest -> headers_in, "User-Agent");
	if (szUserAgent != NULL) { pServer -> request -> user_agent = szUserAgent; }

	// Parse request
	if (oURI.query != NULL && *oURI.query != '\0')
	{
		// Parse request
		UrlencodedParser oParser(pServer -> request -> arguments);
		oParser.ParseChunk(oURI.query, oURI.query + strlen(oURI.query));
		oParser.ParseDone();
	}

	pServer -> request -> request_type = ASRequest::HTTP_REQUEST;

	// POST
	if (rsRequest -> method_number == M_POST)
	{
		// Got Error?
		if (ap_setup_client_block(rsRequest, REQUEST_CHUNKED_ERROR) != OK) { return -1; }

		// Check max. file size
		const INT_64 iMaxPostSize = pConfig -> locations[pServer -> location].max_post_size;

		// Get content type
		CCHAR_P szContentType = ap_table_get(rsRequest -> headers_in, "Content-Type");
		// If Content type not set, revert it to default
		if (szContentType == NULL) { szContentType = DEFAULT_ENCTYPE; }

		// foo=bar&baz=boo
		CCHAR_P szFoundContentType = NULL;
		CHAR_P  szBoundary         = NULL;

		// URL-encoded data
		if      ((szFoundContentType = StrCaseStr(szContentType, DEFAULT_ENCTYPE))   != NULL)
		{
			UrlencodedParser oParser(pServer -> request -> arguments);
			iRC = ParsePOST(rsRequest, &oParser, pServer, oContext, oLogger, iMaxPostSize);
		}
		// Multipart message
		else if ((szFoundContentType = StrCaseStr(szContentType, MULTIPART_ENCTYPE)) != NULL)
		{
			pServer -> request -> request_type = ASRequest::HTTP_RFC1867_REQUEST;

			// Get boundary
			CCHAR_P szTMPBoundary = StrCaseStr(szFoundContentType, "; boundary=");
			if (szTMPBoundary == NULL)
			{
				ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, rsRequest, "CAS: Read POST(" MULTIPART_ENCTYPE "), invalid boundary");
				return HTTP_INTERNAL_SERVER_ERROR;
			}
			// New boundary
			szBoundary = (CHAR_P)ap_pcalloc(rsRequest -> pool, strlen(szTMPBoundary) + 5);
			strcpy(szBoundary, szBoundaryPrefix);
			strcpy(szBoundary + 4, szTMPBoundary);

			MultipartParser oParser(pServer -> request -> arguments,
			                        pServer -> request -> files,
			                        pServer -> filelist,
			                        pConfig -> tempfile_dir,
			                        szBoundary,
			                        pConfig -> locations[pServer -> location].max_file_size,
			                        *(pServer -> server),
			                        oContext,
			                        *(pServer -> request),
			                        oLogger);

			iRC = ParsePOST(rsRequest, &oParser, pServer, oContext, oLogger, iMaxPostSize);
		}
		// XML POST data
		else if ((szFoundContentType = StrCaseStr(szContentType, TEXT_XML_ENCTYPE)) != NULL)
		{
			pServer -> request -> request_type = ASRequest::XMLRPC_REQUEST;

			// TBD: Write XML Parser factory class
			ASXMLRPCHandler oHandler(pServer -> request -> arguments);
			ASXMLParser oXMLParser(&oHandler);
			ASXMLRequestParser oRequestParser(&oXMLParser);

			//  Read & parse XML request
			iRC = ParsePOST(rsRequest, &oRequestParser, pServer, oContext, oLogger, iMaxPostSize);
		}
		// JSON POST data
		else if ((szFoundContentType = StrCaseStr(szContentType, APPLICATION_JSON_ENCTYPE)) != NULL)
		{
			pServer -> request -> request_type = ASRequest::JSONRPC_REQUEST;

			ASJSONRequestParser oRequestParser(pServer -> request -> arguments);

			//  Read & parse XML request
			iRC = ParsePOST(rsRequest, &oRequestParser, pServer, oContext, oLogger, iMaxPostSize);
		}
		// Read raw data
		else
		{
			ASRawDataParser oRequestParser(pServer -> request -> raw_post_data);
			iRC = ParsePOST(rsRequest, &oRequestParser, pServer, oContext, oLogger, iMaxPostSize);
		}
	}

return iRC;
}

//
// Iterate through headers
//
static int HeaderIterator(void        * req,
                          const char  * key,
                          const char  * value)
{
	// Nothing to do?
	if (key == NULL || value == NULL || value[0] == '\0') { return 1; }

	// Store element
	((CTPP::CDT *)req) -> operator[](key) = value;

return 1;
}

//
// Initialize CAS server manager
//
void * ASInitServerManager(server_rec  * sServerRec,
                           char        * szGlobalConfig,
                           int         * iRC)
{
	CASOpaqueServerManager * pOpaque = NULL;

	// Logger
	ASLoggerApacheST oLogger(sServerRec);

	DEBUG_HELPER(&oLogger, STLW::string("ASInitServerManager: \"") + szGlobalConfig + "\"");

	*iRC = -1;
	try
	{
		pOpaque = new CASOpaqueServerManager;
		pOpaque -> server_manager = NULL;

		DEBUG_HELPER_MESSAGE("Create server manager");
		// Create server manager
		pOpaque -> server_manager = new ASServerManager(&oLogger);

		DEBUG_HELPER_MESSAGE("Initialize manager");
		// Initialize manager
		if (pOpaque -> server_manager -> InitManager(szGlobalConfig, &oLogger) == -1)
		{
			DEBUG_HELPER_MESSAGE("Error in InitManager");
		}
		else
		{
			DEBUG_HELPER_MESSAGE("All done");
			*iRC = 0;
		}
	}
	catch(STLW::exception & e)
	{
		DEBUG_HELPER_MESSAGE(e.what());
		oLogger.Emerg(e.what());
	}
	catch(...)
	{
		DEBUG_HELPER_MESSAGE("Unknown error in ASInitServerManager");
		oLogger.Emerg("Unknown error in ASInitServerManager");
	}

	if (*iRC != 0)
	{
		// Clear garbage
		if (pOpaque -> server_manager != NULL)
		{
			DEBUG_HELPER_MESSAGE("ShutdownManager");
			pOpaque -> server_manager -> ShutdownManager(&oLogger);
			delete pOpaque -> server_manager;
		}
		delete pOpaque; pOpaque = NULL;
	}

return (void *)pOpaque;
}

//
// Initialize CAS server manager
//
void * ASInitHost(void         * vOpaqueServerManager,
                  request_rec  * vRequest,
                  char         * szHostConfig)
{
	// Logger
	ASLoggerApache oLogger(vRequest);

	DEBUG_HELPER(&oLogger, STLW::string("ASInitHost: \"") + vRequest -> server -> server_hostname + "\"");

	// Return code
	int iRC = -1;
	// Server manager
	CASOpaqueServerManager  * pOpaqueServerManager = (CASOpaqueServerManager *)vOpaqueServerManager;
	// Server instance
	CASOpaqueServer         * pServer = NULL;

	try
	{
		pServer = new CASOpaqueServer;
		pServer -> server  = NULL;
		pServer -> request = NULL;

		// Initialize virtual server
		DEBUG_HELPER_MESSAGE("Initialize virtual server");
		// Return NULL if any error occured
		pOpaqueServerManager -> server_manager -> InitServer(vRequest -> server -> server_hostname, szHostConfig, &oLogger);

		DEBUG_HELPER_MESSAGE("GetServer");
		pServer -> server = pOpaqueServerManager -> server_manager -> GetServer(vRequest -> server -> server_hostname, &oLogger);

		DEBUG_HELPER_MESSAGE("New ASRequest");
		pServer -> request = new ASRequest;

		// All done
		iRC = 0;
	}
	catch(STLW::exception & e)
	{
		DEBUG_HELPER_MESSAGE(e.what());
		oLogger.Emerg(e.what());
	}
	catch(...)
	{
		DEBUG_HELPER_MESSAGE("Unknown error in ASInitHost");
		oLogger.Emerg("Unknown error in ASInitHost");
	}

	// Clear garbage
	if (iRC != 0)
	{
		if (pServer -> request != NULL) { delete pServer -> request; }
		if (pServer -> server  != NULL) { delete pServer -> server;  }
		delete pServer; pServer = NULL;
	}

return (void *)pServer;
}

//
// Check location
//
int ASCheckLocation(void         * vOpaqueServer,
                    request_rec  * vRequest)
{
	// Logger
	ASLoggerApache oLogger(vRequest);

	DEBUG_HELPER(&oLogger, STLW::string("ASCheckLocation: \"") + vRequest -> uri + "\"");

	int iRC = -1;
	try
	{
		CASOpaqueServer * pOpaqueServer = (CASOpaqueServer *)vOpaqueServer;

		// Check location
		pOpaqueServer -> request -> uri            = vRequest -> uri;
		pOpaqueServer -> request -> unparsed_uri   = vRequest -> unparsed_uri;
		pOpaqueServer -> request -> host           = vRequest -> server -> server_hostname;
		pOpaqueServer -> request -> port           = vRequest -> server -> port;
		pOpaqueServer -> request -> remote_ip      = vRequest -> connection -> remote_ip;
		pOpaqueServer -> request -> headers        = CTPP::CDT(CTPP::CDT::HASH_VAL);
		pOpaqueServer -> request -> cookies        = CTPP::CDT(CTPP::CDT::HASH_VAL);
		pOpaqueServer -> request -> arguments      = CTPP::CDT(CTPP::CDT::HASH_VAL);
		pOpaqueServer -> request -> files          = ASFilePool();
		pOpaqueServer -> request -> uri_components = CTPP::CDT(CTPP::CDT::HASH_VAL);
		pOpaqueServer -> request -> user           = "";
		pOpaqueServer -> request -> password       = "";

		// Server time
		gettimeofday(&(pOpaqueServer -> request -> server_time.timestamp), NULL);
		// Time
		const time_t iTime = pOpaqueServer -> request -> server_time.timestamp.tv_sec;
		// Server local time
		localtime_r(&iTime, &(pOpaqueServer -> request -> server_time.localtime));
		// Server GMT time
		gmtime_r(&iTime, &(pOpaqueServer -> request -> server_time.gmtime));
		// Local IP
		pOpaqueServer -> request -> local_ip = vRequest -> connection -> local_ip;

		INT_32 iLocationId = pOpaqueServer -> server -> CheckLocationURI(vRequest -> uri, *(pOpaqueServer -> request), &oLogger);
		if (iLocationId == -1)
		{
			DEBUG_HELPER_MESSAGE("Location declined");
			return -1;
		}

		pOpaqueServer -> location = iLocationId;
		iRC = 0;
	}
	catch(std::exception & e)
	{
		DEBUG_HELPER_MESSAGE(e.what());
		oLogger.Emerg(e.what());
	}
	catch(...)
	{
		DEBUG_HELPER_MESSAGE("Unknown error in ASCheckLocation");
		oLogger.Emerg("Unknown error in ASCheckLocation");
	}

return iRC;
}

//
// Process request
//
int ASProcessRequest(void         * vOpaqueServer,
                     request_rec  * vRequest)
{
	// Logger
	ASLoggerApache oLogger(vRequest);

	DEBUG_HELPER(&oLogger, STLW::string("ASProcessRequest: \"") + vRequest -> uri + "\"");

	int iRC = -1;
	try
	{
		CASOpaqueServer * pOpaqueServer = (CASOpaqueServer *)vOpaqueServer;

		if (pOpaqueServer -> location == -1)
		{
			DEBUG_HELPER_MESSAGE("Invalid location ID");
			oLogger.Emerg("Invalid location ID, URI `%s`", vRequest -> uri);
			return DECLINED;
		}

		// Check allowed methods
		ASHostConfig * pConfig = pOpaqueServer -> server -> GetConfig();
		// Request context
		ASServer::ASRequestContext oContext(pOpaqueServer -> location, pConfig -> locations[pOpaqueServer -> location]);

		const UINT_32 iAllowedMethods = oContext.location.allowed_methods;
		switch(vRequest -> method_number)
		{
			case M_GET:
				if ((iAllowedMethods & ASRequest::GET)     == 0) { return HTTP_METHOD_NOT_ALLOWED; }
				pOpaqueServer -> request -> request_method = ASRequest::GET;
				break;
			case M_PUT:
				if ((iAllowedMethods & ASRequest::PUT)     == 0) { return HTTP_METHOD_NOT_ALLOWED; }
				pOpaqueServer -> request -> request_method = ASRequest::PUT;
				break;
			case M_POST:
				if ((iAllowedMethods & ASRequest::POST)    == 0) { return HTTP_METHOD_NOT_ALLOWED; }
				pOpaqueServer -> request -> request_method = ASRequest::POST;
				break;
			case M_DELETE:
				if ((iAllowedMethods & ASRequest::DELETE)  == 0) { return HTTP_METHOD_NOT_ALLOWED; }
				pOpaqueServer -> request -> request_method = ASRequest::DELETE;
				break;
			case M_CONNECT:
				if ((iAllowedMethods & ASRequest::CONNECT) == 0) { return HTTP_METHOD_NOT_ALLOWED; }
				pOpaqueServer -> request -> request_method = ASRequest::CONNECT;
				break;
			case M_OPTIONS:
				if ((iAllowedMethods & ASRequest::OPTIONS) == 0) { return HTTP_METHOD_NOT_ALLOWED; }
				pOpaqueServer -> request -> request_method = ASRequest::OPTIONS;
				break;
			case M_TRACE:
				if ((iAllowedMethods & ASRequest::TRACE)   == 0) { return HTTP_METHOD_NOT_ALLOWED; }
				pOpaqueServer -> request -> request_method = ASRequest::TRACE;
				break;
		}

		// Create output collector
		ASApacheResponseWriter oApacheResponseWriter(vRequest);

		// Get headers
		ap_table_do(HeaderIterator, &(pOpaqueServer -> request -> headers), vRequest -> headers_in, NULL);

		// Parse request arguments
		pOpaqueServer -> server -> NotifyObjects(C_START_OF_PRE_REQUEST);
		const INT_32 iReadRequestCode = ReadRequest(vRequest, pOpaqueServer, oContext, oLogger);
		pOpaqueServer -> server -> NotifyObjects(C_END_OF_PRE_REQUEST);

		if (iReadRequestCode != OK) { return iReadRequestCode; }

		// Parse cookies
		ParseCookies(ap_table_get(vRequest -> headers_in, "Cookie"), pOpaqueServer -> request -> cookies);

		// Get Auth status
		CCHAR_P szAuthorization = ap_table_get(vRequest -> headers_in, "Authorization");
		if (szAuthorization != NULL && *szAuthorization != '\0')
		{
			//       123456
			// Find "Basic "
			CCHAR_P szTMP = StrCaseStr(szAuthorization, "Basic ");
			if (szTMP != NULL)
			{
				STLW::string sDecoded = CTPP::Base64Decode(szTMP);
				STLW::string::size_type iPos = sDecoded.find(':');
				if (iPos != STLW::string::npos)
				{
					pOpaqueServer -> request -> user.assign(sDecoded, 0, iPos);
					pOpaqueServer -> request -> password.assign(sDecoded, iPos + 1, sDecoded.size() - iPos);
				}
			}
		}

		// Fill location name
		pOpaqueServer -> request -> location_name = oContext.location.name;

		if (pOpaqueServer -> server -> HandleRequest(oContext, *(pOpaqueServer -> request), &oApacheResponseWriter, &oLogger) == -1)
		{
			fprintf(stderr, "ERROR: Fatal error in HandleRequest(location %s)\n", vRequest -> uri);
			return -1;
		}
		iRC = 0;
	}
	catch(std::exception & e)
	{
		DEBUG_HELPER_MESSAGE(e.what());
		oLogger.Emerg(e.what());
	}
	catch(...)
	{
		DEBUG_HELPER_MESSAGE("Unknown error in ASProcessRequest");
		oLogger.Emerg("Unknown error in ASProcessRequest");
	}

return iRC;
}

//
// Post-request cleanup
//
void PostRequestCleanup(void * vOpaqueServer)
{
	CASOpaqueServer * pOpaqueServer = (CASOpaqueServer *)vOpaqueServer;
	STLW::vector<STLW::string>::iterator itvFileList = pOpaqueServer -> filelist.begin();
	while (itvFileList != pOpaqueServer -> filelist.end())
	{
		// Try to unlink file. Don't care if file does not exist
		unlink(itvFileList -> c_str());
		++itvFileList;
	}
}

//
// Shut down CAS server manager
//
void ASShutdownServerManager(void * vOpaqueServerManager)
{
	ASLoggerFile oLogger(stderr);
	DEBUG_HELPER(&oLogger, "ASShutdownServerManager");

	CASOpaqueServerManager * pOpaqueServerManager = (CASOpaqueServerManager *)vOpaqueServerManager;
	if (pOpaqueServerManager != NULL)
	{
		if (pOpaqueServerManager -> server_manager != NULL)
		{
			pOpaqueServerManager -> server_manager -> ShutdownManager(&oLogger);
			delete pOpaqueServerManager -> server_manager;
		}
		delete pOpaqueServerManager;
	}
}

//
// Shut down CAS server manager
//
void ASShutdownServer(void * vOpaqueServer)
{
	CASOpaqueServer * pOpaqueServer = (CASOpaqueServer *)vOpaqueServer;
	if (pOpaqueServer != NULL) { delete pOpaqueServer -> request; }
}

// End.
