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
 *      XMLView.hpp
 *
 * $CAS$
 */

#include "ASDebugHelper.hpp"
#include "ASLoadableObject.hpp"
#include "ASLoader.hpp"
#include "ASLogger.hpp"
#include "ASUtil.hpp"
#include "ASView.hpp"

#include <CDT.hpp>
#include <CTPP2OutputCollector.hpp>
#include <CTPP2Util.hpp>

#include <sys/time.h>
#include <stdio.h>

using namespace CAS;

#define C_ARRAY_OPEN  "<array>\n"
#define C_ARRAY_CLOSE "</array>\n"

#define C_HASH_OPEN   "<hash>\n"
#define C_HASH_CLOSE  "</hash>\n"

#define C_ITEM_OPEN0  "<item key=\"%d\">"
#define C_ITEM_OPEN1  "<item key=\""
#define C_ITEM_OPEN2  "\">"
#define C_ITEM_CLOSE  "</item>\n"

namespace CAS_MOD
{

/**
  @class XMLView
  @brief Application server object
*/
class XMLView:
  public ASView
{
public:
	/**
	  @brief A virtual destructor
	*/
	~XMLView() throw();

	/**
	  @brief Constructor
	*/
	XMLView();

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

	void DumpData(const CTPP::CDT & oData, CTPP::OutputCollector * pCollector);
};

EXPORT_HANDLER(XMLView)

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class XMLView
//

//
// A constructor
//
XMLView::XMLView()
{
	;;
}

//
// Get handler name
//
CCHAR_P XMLView::GetObjectName() const { return "XMLView"; }

//
// Write HTTP response
//
INT_32 XMLView::WriteResponse(CTPP::CDT         & oData,
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
	using namespace CTPP;

	DEBUG_HELPER(&oLogger, "XMLView::WriteResponse");

	INT_32 iRC  = VIEW_ERROR;
	try
	{
		DEBUG_HELPER_MESSAGE("Write response headers");
		// Write response headers
		oResponseWriter.WriteHeader(oResponse);

		DEBUG_HELPER_MESSAGE("Write response body");
		OutputCollector * pCollector = oResponseWriter.GetBodyOutputCollector();
		pCollector -> Collect("<?xml version=\"1.0\" ?>\n", sizeof("<?xml version=\"1.0\" ?>\n") - 1);

		if (oData.GetType() == CDT::UNDEF) { pCollector -> Collect("<item/>\n", sizeof("<item/>\n") - 1); }
		else
		{
			DumpData(oData, oResponseWriter.GetBodyOutputCollector());
		}
		iRC = VIEW_OK;
	}
	catch (std::exception & e)
	{
		STLW::string sError("Error in XMLView: ");
		sError += e.what();
		oLogger.WriteLog(AS_LOG_EMERG, sError.c_str());
	}
	catch (...)
	{
		oLogger.WriteLog(AS_LOG_EMERG, "Unknown fatal error in XMLView");
	}

return iRC;
}

//
// Dump data to XML
//
void XMLView::DumpData(const CTPP::CDT & oData, CTPP::OutputCollector * pCollector)
{
	using namespace CTPP;

	CHAR_8 szData[1024];

	const CDT::eValType oEtype = oData.GetType();
	if (oEtype <= CDT::STRING_VAL)
	{
		// Store plain data
		const STLW::string sData = XMLEscape(oData.GetString());
		pCollector -> Collect(sData.data(), sData.size());
	}
	else if (oEtype == CDT::ARRAY_VAL)
	{
		pCollector-> Collect(C_ARRAY_OPEN, sizeof(C_ARRAY_OPEN) - 1);
		const UINT_32 iArraySize = oData.Size();
		for (UINT_32 iPos = 0; iPos < iArraySize; ++iPos)
		{
			// Store array index
			UINT_32 iChars = snprintf(szData, 1024, C_ITEM_OPEN0, iPos);
			pCollector-> Collect(szData, iChars);

			// Store data
			DumpData(oData.GetCDT(iPos), pCollector);

			// Close tag
			pCollector-> Collect(C_ITEM_CLOSE, sizeof(C_ITEM_CLOSE) - 1);
		}
		pCollector-> Collect(C_ARRAY_CLOSE, sizeof(C_ARRAY_CLOSE) - 1);
	}
	else if (oEtype == CDT::HASH_VAL)
	{
		pCollector-> Collect(C_HASH_OPEN, sizeof(C_HASH_OPEN) - 1);
		CDTConstIterator itoData = oData.Begin();
		while(itoData != oData.End())
		{
			// Store hash key
			pCollector-> Collect(C_ITEM_OPEN1, sizeof(C_ITEM_OPEN1) - 1);
			const STLW::string sData = XMLEscape(itoData -> first);
			pCollector-> Collect(sData.data(), sData.size());
			pCollector-> Collect(C_ITEM_OPEN2, sizeof(C_ITEM_OPEN2) - 1);

			// Store data
			DumpData(itoData -> second, pCollector);

			// Close tag
			pCollector-> Collect(C_ITEM_CLOSE, sizeof(C_ITEM_CLOSE) - 1);
			++itoData;
		}
		pCollector-> Collect(C_HASH_CLOSE, sizeof(C_HASH_CLOSE) - 1);
	}
}

//
// A destructor
//
XMLView::~XMLView() throw()
{
	;;
}

} // namespace CAS_MOD
// End.
