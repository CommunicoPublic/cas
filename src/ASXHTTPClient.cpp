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
 *      ASCookie.cpp
 *
 * $CAS$
 */
#include "ASHTTPClient.hpp"
#include "ASXMLParser.hpp"
#include "ASXMLRPCHandler.hpp"

#include <curl/curl.h>

#include <CTPP2Util.hpp>
#include <CTPP2StringOutputCollector.hpp>

namespace CAS // C++ Application Server
{

#define C_UNDEF_VAL           "<value><string></string></value>\n"

#define C_INT_VAL_S           "<value><int>"
#define C_INT_VAL_E           "</int></value>\n"

#define C_REAL_VAL_S          "<value><double>"
#define C_REAL_VAL_E          "</double></value>\n"

#define C_STRING_VAL_S        "<value><string>"
#define C_STRING_VAL_E        "</string></value>\n"

#define C_STRING_CDATA_VAL_S  "<value><string><![CDATA["
#define C_STRING_CDATA_VAL_E  "]]></string></value>\n"

#define C_STRING_BASE64_VAL_S "<value><base64>"
#define C_STRING_BASE64_VAL_E "</base64></value>\n"

#define C_INVALID_STRING      "<value><string>Invalid string!</string></value>\n"

#define C_ARRAY_VAL_S         "<value><array><data>\n"
#define C_ARRAY_VAL_E         "</data></array></value>\n"

#define C_HASH_VAL_S          "<value><struct>\n"
#define C_HASH_VAL_E          "</struct></value>\n"

#define C_MEMBER_S            "<member>\n<name>"
#define C_MEMBER_NAME_E       "</name>\n"

#define C_MEMBER_E            "</member>\n"

#define C_INVALID_TYPE        "<value><string><![CDATA[Invalid type]]></string></value>\n"

static INT_32 CheckStringType(const STLW::string & sSource)
{
	CCHAR_P szString = sSource.data();
	CCHAR_P szStringEnd = sSource.data() + sSource.size();
	for (;;)
	{
		INT_32 iLen = CTPP::utf_charlen(szString, szStringEnd);
		if (iLen <= 0)
		{
			// UTF-8 string, all done
			if (iLen == -3) { return 0; }

			// Need Base64
			return 2;
		}
		// Need CDATA?
		else if (iLen == 1)
		{
			if (*szString == '<' || *szString == '>' || *szString == '&' || *szString == '"') { return 1; }
		}

		szString += iLen;
	}

return 2;
}

//
// Dump to string
//
static void DumpXMPRPCData(const CTPP::CDT & oData, CTPP::OutputCollector * pCollector)
{
	switch (oData.GetType())
	{
		case CTPP::CDT::UNDEF:
			{
				pCollector -> Collect(C_UNDEF_VAL, sizeof(C_UNDEF_VAL) - 1);
			}
			return;

		case CTPP::CDT::INT_VAL:
		case CTPP::CDT::POINTER_VAL:
			{
				const STLW::string & oTMP = oData.GetString();

				pCollector -> Collect(C_INT_VAL_S, sizeof(C_INT_VAL_S) - 1);
				pCollector -> Collect(oTMP.data(), oTMP.size());
				pCollector -> Collect(C_INT_VAL_E, sizeof(C_INT_VAL_E) - 1);
			}
			return;

		case CTPP::CDT::REAL_VAL:
			{
				const STLW::string & oTMP = oData.GetString();

				pCollector -> Collect(C_REAL_VAL_S, sizeof(C_REAL_VAL_S) - 1);
				pCollector -> Collect(oTMP.data(), oTMP.size());
				pCollector -> Collect(C_REAL_VAL_E, sizeof(C_REAL_VAL_E) - 1);
			}
			return;

		case CTPP::CDT::STRING_INT_VAL:
		case CTPP::CDT::STRING_REAL_VAL:
		case CTPP::CDT::STRING_VAL:
			{
				const STLW::string & oTMP = oData.GetString();
				// Check UTF-8 string
				switch(CheckStringType(oTMP))
				{
					// Do nothing
					case 0:
						{
							pCollector -> Collect(C_STRING_VAL_S, sizeof(C_STRING_VAL_S) - 1);
							pCollector -> Collect(oTMP.data(), oTMP.size());
							pCollector -> Collect(C_STRING_VAL_E, sizeof(C_STRING_VAL_E) - 1);
						}
						break;
					// <![CDATA[ data ]]>
					case 1:
						{
							pCollector -> Collect(C_STRING_CDATA_VAL_S, sizeof(C_STRING_CDATA_VAL_S) - 1);
							pCollector -> Collect(oTMP.data(), oTMP.size());
							pCollector -> Collect(C_STRING_CDATA_VAL_E, sizeof(C_STRING_CDATA_VAL_E) - 1);
						}
						break;
					// Convert to Base64
					case 2:
						{
							pCollector -> Collect(C_STRING_BASE64_VAL_S, sizeof(C_STRING_BASE64_VAL_S) - 1);
							const STLW::string & oTMP1 = CTPP::Base64Encode(oTMP);
							pCollector -> Collect(oTMP1.data(), oTMP1.size());
							pCollector -> Collect(C_STRING_BASE64_VAL_E, sizeof(C_STRING_BASE64_VAL_E) - 1);
						}
						break;
					default:
						pCollector -> Collect(C_INVALID_TYPE, sizeof(C_INVALID_TYPE) - 1);
				}
			}
			return;

		case CTPP::CDT::ARRAY_VAL:
			{
				pCollector -> Collect(C_ARRAY_VAL_S, sizeof(C_ARRAY_VAL_S) - 1);
				for (UINT_32 iI = 0; iI < oData.Size(); ++iI)
				{
					DumpXMPRPCData(oData.GetCDT(iI), pCollector);
				}
				pCollector -> Collect(C_ARRAY_VAL_E, sizeof(C_ARRAY_VAL_E) - 1);
			}
			return;

		case CTPP::CDT::HASH_VAL:
			{
				pCollector -> Collect(C_HASH_VAL_S, sizeof(C_HASH_VAL_S) - 1);
				CTPP::CDT::ConstIterator itHash = oData.Begin();
				while (itHash != oData.End())
				{
					pCollector -> Collect(C_MEMBER_S, sizeof(C_MEMBER_S) - 1);
					pCollector -> Collect(itHash -> first.data(), itHash -> first.size());
					pCollector -> Collect(C_MEMBER_NAME_E, sizeof(C_MEMBER_NAME_E) - 1);

					DumpXMPRPCData(itHash -> second, pCollector);

					pCollector -> Collect(C_MEMBER_E, sizeof(C_MEMBER_E) - 1);
					++itHash;
				}
				pCollector -> Collect(C_HASH_VAL_E, sizeof(C_HASH_VAL_E) - 1);
			}
			return;

		default:
			pCollector -> Collect(C_INVALID_TYPE, sizeof(C_INVALID_TYPE) - 1);
	}
}


//
// Input callback
//
static size_t InCallback(void * vData, size_t iSize, size_t iObjects, void * pThis)
{
	const size_t iMaxBytes = iSize * iObjects;
	((ASXMLParser *)pThis) -> ParseStream((CHAR_P)vData, iMaxBytes);

return iMaxBytes;
}

struct Chunk
{
	CCHAR_P   data;
	INT_32    processed;
	INT_32    size;
};

//
// Output callback
//
static size_t OutCallback(void * vData, size_t iSize, size_t iObjects, void * pThis)
{
	Chunk * pChunk = (Chunk *)pThis;

	const UINT_32 iBytesLeft = pChunk -> size - pChunk -> processed;
	UINT_32 iMaxBytes = iSize * iObjects;
	if (iMaxBytes > iBytesLeft) { iMaxBytes = iBytesLeft; }

	for(UINT_32 iPos = 0; iPos < iMaxBytes; ++iPos)
	{
		((CHAR_8 *)vData)[iPos] = pChunk -> data[pChunk -> processed + iPos];
	}
	pChunk -> processed += iMaxBytes;

return iMaxBytes;
}

//
// Number of instances
//
INT_64 ASHTTPClient::iInstances = 0;

//
// Constructor
//
ASHTTPClient::ASHTTPClient(const STLW::string  & sHost,
                           const UINT_32         iPort,
                           const UINT_32         iConnTimeout,
                           const UINT_32         iIOTimeout,
                           const STLW::string  & sServer)
{
	using namespace CTPP;

	ASHTTPClient::Init();

	vCURL = curl_easy_init();

	curl_easy_setopt(vCURL, CURLOPT_POST, 1);
	curl_easy_setopt(vCURL, CURLOPT_URL, sURL.c_str());
	curl_easy_setopt(vCURL, CURLOPT_PORT, iPort);
	curl_easy_setopt(vCURL, CURLOPT_USERAGENT, "CAS v" AS_VERSION "(" AS_IDENT ") XMLRPC Client");
	curl_easy_setopt(vCURL, CURLOPT_CONNECTTIMEOUT_MS, iConnTimeout);
	curl_easy_setopt(vCURL, CURLOPT_TIMEOUT_MS, iIOTimeout);

	struct curl_slist * pHeaders = NULL;
	pHeaders = curl_slist_append(pHeaders, "Content-type: text/xml");

	if (oHeaders.GetType() == CDT::HASH_VAL)
	{
		CDT::ConstIterator itoHeaders = oHeaders.Begin();
		while (itoHeaders != oHeaders.End())
		{
			STLW::string sHeader(itoHeaders -> first);
			sHeader.append(": ", 2);
			sHeader.append(itoHeaders -> second.GetString());
			pHeaders = curl_slist_append(pHeaders, sHeader.c_str());
			++itoHeaders;
		}
	}

	if (!sServer.empty())
	{
		STLW::string sHostHeader("Host: ");
		sHostHeader.append(sServer);
		pHeaders = curl_slist_append(pHeaders, sHostHeader.c_str());
	}

	curl_easy_setopt(vCURL, CURLOPT_HTTPHEADER, pHeaders);
}

//
// Execute XMLRPC call
//
INT_32 ASHTTPClient::Call(const STLW::string  & sMethod,
                          const STLW::map<STLW::string, STLW::string>  & oParams,
                          const STLW::map<STLW::string, STLW::string>  & oHeaders,
                          CTPP::CDT           & oResponse)
{


//
// Execute XMLRPC call
//
INT_32 ASHTTPClient::Call(const STLW::string  & sMethod,
                         const CTPP::CDT     & oRequest,
                         CTPP::CDT           & oResponse)
{
	using namespace CTPP;

	STLW::string sRequest("<?xml version=\"1.0\"?>\n<methodCall>\n<methodName>");
	sRequest.append(sMethod);
	sRequest.append("</methodName>\n<params>\n");

	// Create XMLRPC request data
	if (oRequest.GetType() != CDT::ARRAY_VAL)
	{
		sRequest.append("<param>\n");
		StringOutputCollector oCollector(sRequest);
		DumpXMPRPCData(oRequest, &oCollector);
		sRequest.append("</param>\n");
	}
	else
	{
		for (UINT_32 iPos = 0; iPos < oRequest.Size(); ++iPos)
		{
			sRequest.append("<param>\n");
			StringOutputCollector oCollector(sRequest);
			DumpXMPRPCData(oRequest.GetCDT(iPos), &oCollector);
			sRequest.append("</param>\n");
		}
	}
	sRequest.append("</params>\n</methodCall>");

	Chunk oRequestChunk = {sRequest.c_str(), 0, sRequest.size()};
	curl_easy_setopt(vCURL, CURLOPT_READDATA, (void *)&oRequestChunk);
	curl_easy_setopt(vCURL, CURLOPT_READFUNCTION,  OutCallback);

	//  Read & parse XML request
	CDT oTMP;
	ASXMLRPCHandler oHandler(oTMP);
	ASXMLParser oXMLParser(&oHandler);
	oXMLParser.InitStream();
	curl_easy_setopt(vCURL, CURLOPT_WRITEDATA, (void *)&oXMLParser);
	curl_easy_setopt(vCURL, CURLOPT_WRITEFUNCTION, InCallback);

	curl_easy_setopt(vCURL, CURLOPT_POSTFIELDSIZE, sRequest.size());

	curl_easy_perform(vCURL);
	curl_easy_cleanup(vCURL);

	oXMLParser.CloseStream();

	const CDT & oEntry = oTMP.GetCDT("params");
	if (oEntry.GetType() == CDT::ARRAY_VAL && oEntry.Size() == 1)
	{
		oResponse = oEntry.GetCDT(0);
	}
	else
	{
		oResponse = oTMP;
	}

return 0;
}

//
// A drestructor
//
ASHTTPClient::~ASHTTPClient() throw()
{
	ASHTTPClient::Destroy();
}

void ASHTTPClient::Init()
{
	if (iInstances == 0)
	{
		curl_global_init(CURL_GLOBAL_ALL);
	}
	++iInstances;
}

void ASHTTPClient::Destroy()
{
	--iInstances;

	if (iInstances == 0)
	{
		curl_global_cleanup();
	}
}

} // namespace CAS
// End.
