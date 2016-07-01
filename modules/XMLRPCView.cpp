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
 *      XMLRPCView.hpp
 *
 * $CAS$
 */

#include "ASDebugHelper.hpp"
#include "ASLoadableObject.hpp"
#include "ASLoader.hpp"
#include "ASLogger.hpp"
#include "ASUtil.hpp"
#include "ASView.hpp"

#include <CTPP2Util.hpp>
#include <CTPP2OutputCollector.hpp>

#include <sys/time.h>

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

using namespace CAS;

namespace CAS_MOD
{

/**
  @class XMLRPCView
  @brief Application server object
*/
class XMLRPCView:
  public ASView
{
public:
	/**
	  @brief A virtual destructor
	*/
	~XMLRPCView() throw();

	/**
	  @brief Constructor
	*/
	XMLRPCView();

private:
	/**
	  @brief Get handler name
	*/
	CCHAR_P GetObjectName() const;

	/**
	  @brief Write HTTP response
	  @param oData - data model
	  @param oResponse - response object
	  @param pResponseWriter - response writer object
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oRequestPool - requeset data pool
	  @param pContextData - context data
	  @param oIMC - Inter-Module communication object
	  @param oLogger - application server logger
	  @return VIEW_OK - if success, VIEW_ERROR - if any error occured
	*/
	INT_32 WriteResponse(CTPP::CDT         & oData,
	                     ASResponse        & oResponse,
	                     ASResponseWriter  & pResponseWriter,
	                     ASPool            & oGlobalPool,
	                     ASPool            & oVhostPool,
	                     ASPool            & oRequestPool,
	                     CTPP::CDT         & oLocationConfig,
	                     ASObject          * pContextData,
	                     CTPP::CDT         & oIMC,
	                     ASLogger          & oLogger);
};

EXPORT_HANDLER(XMLRPCView)

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Static functions
//

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
			// Need Base64
			if (*szString < ' ' && *szString != '\r' && *szString != '\n') { return 2; }

			// Need to escape XML sequence
			if (*szString == '<' || *szString == '>' || *szString == '&' || *szString == '"') { return 1; }

			// Check special case for ]]>
			if (*szString == ']')
			{
				CCHAR_P szTMP = szString;
				++szTMP;
				if (szTMP != szStringEnd && *szTMP == ']')
				{
					++szTMP;
					// Need Base64
					if (szTMP != szStringEnd && *szTMP == '>') { return 2; }
				}
			}

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
				CTPP::CDTConstIterator itHash = oData.Begin();
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

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class XMLRPCView
//

//
// A constructor
//
XMLRPCView::XMLRPCView()
{
	;;
}

//
// Get handler name
//
CCHAR_P XMLRPCView::GetObjectName() const { return "XMLRPCView"; }

//
// Write HTTP response
//
INT_32 XMLRPCView::WriteResponse(CTPP::CDT         & oData,
                                 ASResponse        & oResponse,
                                 ASResponseWriter  & oResponseWriter,
                                 ASPool            & oGlobalPool,
                                 ASPool            & oVhostPool,
                                 ASPool            & oRequestPool,
                                 CTPP::CDT         & oLocationConfig,
                                 ASObject          * pContextData,
                                 CTPP::CDT         & oIMC,
                                 ASLogger          & oLogger)
{
	DEBUG_HELPER(&oLogger, "XMLRPCView::WriteResponse");

	using namespace CTPP;

	INT_32 iRC  = VIEW_ERROR;
	try
	{
		DEBUG_HELPER_MESSAGE("Write response headers");
		// Write response headers
		oResponseWriter.WriteHeader(oResponse);

		DEBUG_HELPER_MESSAGE("Write response body");


/*

Normal response:
Types: <i4>, <int>, <boolean>, <string>, <double>, <dateTime.iso8601>, <base64>
<?xml version="1.0"?>
<methodResponse>
<params>
	<param>
		<value>
			<struct>
				<member>
					<name>Foo</name>
					<value><int>4</int></value>
				</member>
			</struct>
		</value>
	</param>
	<param>
		<value>
			<array>
				<data>
					<value><i4>12</i4></value>
					<value><int>123456</int></value>
					<value><boolean>true</boolean></value>
					<value><string>Hello, World!</string></value>
					<value><double>123.456</double></value>
					<value><dateTime.iso8601>20101124T12:49:00</dateTime.iso8601></value>
					<value><base64></base64></value>
				</data>
			</array>
		</value>
	</param>
</params>
</methodResponse>

Fault example:
<?xml version="1.0"?>
<methodResponse>
	<fault>
		<value>
			<struct>
				<member>
					<name>faultCode</name>
					<value><int>4</int></value>
				</member>
				<member>
					<name>faultString</name>
					<value><string>Too many parameters.</string></value>
				</member>
			</struct>
		</value>
	</fault>
</methodResponse>
*/
		// Need to refactor!!!

		OutputCollector * pCollector = oResponseWriter.GetBodyOutputCollector();
		pCollector -> Collect("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<methodResponse>\n<params>\n<param>\n", sizeof("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<methodResponse>\n<params>\n<param>\n") - 1);
		DumpXMPRPCData(oData, pCollector);
		pCollector -> Collect("</param>\n</params>\n</methodResponse>", sizeof("</param>\n</params>\n</methodResponse>") - 1);

		iRC = VIEW_OK;
	}
	catch (std::exception & e)
	{
		STLW::string sError("Error in XMLRPCView: ");
		sError += e.what();
		oLogger.WriteLog(AS_LOG_EMERG, sError.c_str());
	}
	catch (...)
	{
		oLogger.WriteLog(AS_LOG_EMERG, "Unknown fatal error in XMLRPCView");
	}

return iRC;
}

//
// A destructor
//
XMLRPCView::~XMLRPCView() throw()
{
	;;
}

} // namespace CAS_MOD
// End.
