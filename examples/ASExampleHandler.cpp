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
 *      ASExampleHandler.hpp
 *
 * $CAS$
 */

#include <ASDebugHelper.hpp>
#include <ASLoadableObject.hpp>
#include <ASHandler.hpp>

#include "ASExampleClass.hpp"
#include "ASExampleObject.hpp"

using namespace CAS;

namespace TEST_NS
{

/**
  @class ASExampleHandler ASExampleHandler.hpp <ASExampleHandler.cpp>
  @brief Application server object
*/
class ASExampleHandler:
  public ASHandler
{
public:
	/**
	  @brief A constructor
	*/
	ASExampleHandler();

	/**
	  @brief Initialize module
	  @param oConfiguration - module configuration
	  @param oModulesPool - pool of modules
	  @param oObjectPool - pool of objects
	  @param oLogger - logger object
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 InitModule(CTPP::CDT  & oConfiguration,
	                  ASPool     & oModulesPool,
	                  ASPool     & oObjectPool,
	                  ASLogger   & oLogger);

	/**
	  @brief Initialize server hook
	  @param oConfiguration - module configuration
	  @param oGlobalPool - pool of global objects
	  @param oServerPool - pool of per-server objects
	  @param oLogger - logger object
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 InitServer(CTPP::CDT  & oConfiguration,
	                  ASPool     & oGlobalPool,
	                  ASPool     & oServerPool,
	                  ASLogger   & oLogger);

	/**
	  @brief Initialize view
	  @param oLocationConfig - handler configuration
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured
	*/
	INT_32 InitLocation(CTPP::CDT  & oLocationConfig,
	                    ASObject   * pContextData,
	                    ASLogger   & oLogger);

	/**
	  @brief Initialize handler
	  @param oData - data model
	  @param oRequest - request object
	  @param oResponse - response object
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oLocationConfig - location configuration
	  @param oIMC - Inter-Module communication object
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured, HANDLER_DONE - if no futher processing need
	*/
	INT_32 Handler(CTPP::CDT   & oData,
	               ASRequest   & oRequest,
	               ASResponse  & oResponse,
	               ASPool      & oGlobalPool,
	               ASPool      & oVhostPool,
	               ASPool      & oRequestPool,
	               CTPP::CDT   & oLocationConfig,
	               ASObject    * pContextData,
	               CTPP::CDT   & oIMC,
	               ASLogger    & oLogger);

	/**
	  @brief Post-execution fixup
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oRequestPool - requeset data pool
	  @param oLocationConfig - location configuration
	  @param oIMC - Inter-Module communication object
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured, HANDLER_DONE - if no futher processing need
	*/
	INT_32 Fixup(ASPool      & oGlobalPool,
	             ASPool      & oVhostPool,
	             ASPool      & oRequestPool,
	             CTPP::CDT   & oLocationConfig,
	             ASObject    * pContextData,
	             CTPP::CDT   & oIMC,
	             ASLogger    & oLogger);

	/**
	  @brief Get handler name
	*/
	CCHAR_P GetObjectName() const;

	/**
	  @brief A destructor
	*/
	~ASExampleHandler() throw();
};

EXPORT_HANDLER(ASExampleHandler)

//
// A constructor
//
ASExampleHandler::ASExampleHandler()
{
	;;
}

//
// Initialize module
//
INT_32 ASExampleHandler::InitModule(CTPP::CDT  & oConfiguration,
                                    ASPool     & oModulesPool,
                                    ASPool     & oObjectPool,
                                    ASLogger   & oLogger)
{
	DEBUG_HELPER(&oLogger, "ASExampleHandler::InitModule");

return 0;
}

//
// Initialize server hook
//
INT_32 ASExampleHandler::InitServer(CTPP::CDT  & oConfiguration,
                                    ASPool     & oGlobalPool,
                                    ASPool     & oServerPool,
                                    ASLogger   & oLogger)
{
	DEBUG_HELPER(&oLogger, "ASExampleHandler::InitServer");

return 0;
}

//
// Initialize handler by location
//
INT_32 ASExampleHandler::InitLocation(CTPP::CDT  & oLocationConfig,
                                      ASObject   * pContextData,
                                      ASLogger   & oLogger)
{
	DEBUG_HELPER(&oLogger, "ASExampleHandler::InitLocation");

return 0;
}

//
// Initialize handler
//
INT_32 ASExampleHandler::Handler(CTPP::CDT   & oData,
                                 ASRequest   & oRequest,
                                 ASResponse  & oResponse,
                                 ASPool      & oGlobalPool,
                                 ASPool      & oVhostPool,
                                 ASPool      & oRequestPool,
                                 CTPP::CDT   & oLocationConfig,
                                 ASObject    * pContextData,
                                 CTPP::CDT   & oIMC,
                                 ASLogger    & oLogger)
{
	using namespace CTPP;

	DEBUG_HELPER(&oLogger, "ASExampleHandler::Handler");

	// Authentication example
	if (oRequest.arguments["check_auth"] == "yes")
	{
		if (oRequest.user == "user" && oRequest.password == "password")
		{
			DEBUG_HELPER_MESSAGE("User authenticated.");
		}
		else
		{
			DEBUG_HELPER_MESSAGE("Invalid login/password");
			oLogger.WriteLog(AS_LOG_EMERG, "ASExampleHandler: Invalid login/password");
			oResponse.AuthenticateBasic("Secured area");
			return HANDLER_DONE;
		}
	}

	ASExampleObject * pResource = oVhostPool.GetResourceByName<ASExampleObject>("ASExampleClassPlaceholder");
	if (pResource == NULL)
	{
		DEBUG_HELPER_MESSAGE("Object NOT found");
		oLogger.WriteLog(AS_LOG_EMERG, "ASExampleHandler: Object NOT found");
	}
	else
	{
		DEBUG_HELPER_MESSAGE("Object found");
		pResource -> GetClass() -> IncreaseCounter();
		oData["Counter"]  = pResource -> GetClass() -> GetCounter();
	}

	CDT oFiles(CDT::ARRAY_VAL);
	ASFilePool::Iterator itmFiles = oRequest.files.Begin();
	while (itmFiles != oRequest.files.End())
	{
		CDT oTMP(CDT::HASH_VAL);
		oTMP["name"] = itmFiles -> name;
		oTMP["filename"] = itmFiles -> filename;
		oTMP["tmp_name"] = itmFiles -> tmp_name;
		oTMP["content_type"] = itmFiles -> content_type;
		oTMP["filesize"] = itmFiles -> filesize;
		oFiles.PushBack(oTMP);
		++itmFiles;
	}

	oData["Configuration"]        = oLocationConfig;
	oData["Request_Uri"]          = oRequest.uri;
	oData["Request_Unparsed_Uri"] = oRequest.unparsed_uri;
	oData["Headers"]              = oRequest.headers;
	oData["Arguments"]            = oRequest.arguments;

	oData["Files"]                = oFiles;
	oData["Cookies"]              = oRequest.cookies;
	oData["URI_Components"]       = oRequest.uri_components;
	oData["IMC"]                  = oIMC;
	oData["Version"]              = AS_VERSION "(" AS_IDENT ")";

	oIMC["template"]              = "tmpl/example.tmpl";

	oLogger.Emerg("This message logged with AS_LOG_EMERG priority");

	oLogger.Alert("This message logged with AS_LOG_ALERT priority");

	oLogger.Crit("This message logged with AS_LOG_CRIT priority");

	oLogger.Err("This message logged with AS_LOG_ERR priority");

	oLogger.Warn("This message logged with AS_LOG_WARNING priority");

	oLogger.Notice("This message logged with AS_LOG_NOTICE priority");

	oLogger.Info("This message logged with AS_LOG_INFO priority");

	oLogger.Debug("This message logged with AS_LOG_DEBUG priority");

//	Redirect ?
//	oResponse.Redirect("http://cas.havoc.ru/");

	// Set cookie
	oResponse.SetCookie("foo", "bar baz/boo", 1707834065, oRequest.host, "/");

	// Set HTTP header
	oResponse.SetHeader("X-Module", "ASExampleHandler");

return 0;
}

//
// Post-execution fixup
//
INT_32 ASExampleHandler::Fixup(ASPool     & oGlobalPool,
                               ASPool     & oVhostPool,
                               ASPool     & oRequestPool,
                               CTPP::CDT  & oLocationConfig,
                               ASObject   * pContextData,
                               CTPP::CDT  & oIMC,
                               ASLogger   & oLogger)
{
	DEBUG_HELPER(&oLogger, "ASExampleHandler::Fixup");

return 0;
}

//
// Get handler name
//
CCHAR_P ASExampleHandler::GetObjectName() const { return "ASExampleHandler"; }

//
// A destructor
//
ASExampleHandler::~ASExampleHandler() throw()
{
	;;
}

} // namespace TEST_NS
// End.
