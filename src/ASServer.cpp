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
 *      ASServer.cpp
 *
 * $CAS$
 */

#include "ASServer.hpp"

#include "ASDebugHelper.hpp"
#include "ASGlobalConfig.hpp"
#include "ASHandler.hpp"
#include "ASHostConfig.hpp"
#include "ASLocationURI.hpp"
#include "ASPreRequestHandler.hpp"
#include "ASResponseWriter.hpp"
#include "ASView.hpp"

#include <sys/time.h>

namespace CAS // C++ Application Server
{

//
// Run post-execution fixups
//
static INT_32 RunFixups(ASPool                                         & oGlobalPool,
                        ASPool                                         & oHostPool,
                        ASPool                                         & oRequestPool,
                        CTPP::CDT                                      & oIMC,
                        STLW::vector<ASHandlerConfig>::iterator        & itvHandlers,
                        const STLW::vector<ASHandlerConfig>::iterator  & itEndPos,
                        ASLogger                                       * pLogger)
{
	DEBUG_HELPER(pLogger, "RunFixups");

	INT_32 iRC = 0;
	// Run fixups
	for(;;)
	{
		if (itvHandlers -> object -> Fixup(oGlobalPool,
		                                   oHostPool,
		                                   oRequestPool,
		                                   itvHandlers -> handler_config,
		                                   itvHandlers -> context_data,
		                                   oIMC,
		                                   *pLogger)== -1)
		{
			pLogger -> Crit("Fatal error in fixup \"%s\", exiting", itvHandlers -> name.c_str());
			DEBUG_HELPER_MESSAGE("Fatal error in fixup \"%s\", exiting", itvHandlers -> name.c_str());

			iRC = -1;
		}
		// End of list
		if (itvHandlers == itEndPos) { break; }

		--itvHandlers;
	}

return iRC;
}

//
// Constructor
//
ASServer::ASRequestContext::ASRequestContext(const UINT_32    iLocationId,
                                             ASLocation     & oLocation): location_id(iLocationId),
                                                                          location(oLocation),
                                                                          data(CTPP::CDT::HASH_VAL),
                                                                          imc(CTPP::CDT::HASH_VAL)
{
	;;
}

//
// Constructor
//
ASServer::ASServer(const ASGlobalConfig  * pIGlobalConfig,
                   ASHostConfig          * pIHostConfig,
                   ASPool                * pIGlobalPool,
                   ASPool                * pIHostPool,
                   ASLogger              * pLogger): pGlobalConfig(pIGlobalConfig),
                                                     pHostConfig(pIHostConfig),
                                                     pGlobalPool(pIGlobalPool),
                                                     pHostPool(pIHostPool)
{
	// Nothing to do, but let it be
	DEBUG_HELPER(pLogger, "ASServer");
}

//
// Check URI
//
INT_32 ASServer::CheckLocationURI(CCHAR_P      szLocationURI,
                                  ASRequest  & oRequest,
                                  ASLogger   * pLogger) const
{
	DEBUG_HELPER(pLogger, "CheckLocationURI");

	// Set real host name
	oRequest.system_hostname = pGlobalConfig -> system_hostname;

	// Iterate through locations
	for(UINT_32 iLocation = 0; iLocation < pHostConfig -> locations.size(); ++iLocation)
	{
		UINT_32 iURI = 0;
		for(iURI = 0; iURI < pHostConfig -> locations[iLocation].uri_list.size(); ++iURI)
		{
			if (pHostConfig -> locations[iLocation].uri_list[iURI] -> CheckURI(szLocationURI, oRequest.uri_components) == 0)
			{
				DEBUG_HELPER_MESSAGE("Found location %u", iLocation);
				return iLocation;
			}
		}
	}
	// Location not found
	DEBUG_HELPER_MESSAGE("Location not found");

// Nothing found
return -1;
}

//
// Get list of accepted parameters
//
INT_32 ASServer::GetLocationParameters(const UINT_32  & iLocation,
                                       CTPP::CDT      & oHeaders,
                                       CTPP::CDT      & oArguments,
                                       CTPP::CDT      & oCookies,
                                       ASLogger       * pLogger) const
{
	DEBUG_HELPER(pLogger, "GetLocationParameters");

	// Invalid location ?
	if (iLocation >= pHostConfig -> locations.size())
	{
		DEBUG_HELPER_MESSAGE("Invalid location Id %u", iLocation);
		pLogger -> Crit("Invalid location Id %u", iLocation);

		return -1;
	}

	DEBUG_HELPER_MESSAGE("Not developed yet");
	pLogger -> Crit("Not developed yet");

return -1;
}

//
// Handle request
//
INT_32 ASServer::HandleRequest(ASRequestContext  & oContext,
                               ASRequest         & oRequest,
                               ASResponseWriter  * pResponseWriter,
                               ASLogger          * pLogger)
{
	DEBUG_HELPER(pLogger, "HandleRequest");

	// Notify objects from global pool
	pGlobalPool -> NotifyObjects<ASObject>(C_START_OF_REQUEST);
	// Notify objects from server pool
	pHostPool -> NotifyObjects<ASObject>(C_START_OF_REQUEST);

	// Really hanlde request
	INT_32 iRC = HANDLER_OK;
	try
	{
		iRC = RealHandleRequest(oContext, oRequest, pResponseWriter, pLogger);
	}
	catch(...)
	{

		DEBUG_HELPER_MESSAGE("Fatal error in RealHandleRequest");
		pLogger -> Crit("Fatal error in RealHandleRequest");
		iRC = HANDLER_ERROR;
	}

	// Notify objects from server pool
	pHostPool -> NotifyObjects<ASObject>(C_END_OF_REQUEST);
	// Notify objects from global pool
	pGlobalPool -> NotifyObjects<ASObject>(C_END_OF_REQUEST);

return iRC;
}

//
// Handle pre-request
//
INT_32 ASServer::HandlePreRequest(ASRequestContext  & oContext,
                                  CCHAR_P             vChunk,
                                  const UINT_32       iChunkSize,
                                  ASRequest         & oRequest,
                                  ASLogger          * pLogger)
{
	// Invoke pre-request handlers
	DEBUG_HELPER(pLogger, "Invoke pre-request handlers");
	STLW::vector<ASPreRequestHandlerConfig>::iterator itvPreRequestHandlers = oContext.location.pre_request_handlers.begin();

	INT_32 iRC = 0;

	if (itvPreRequestHandlers == oContext.location.pre_request_handlers.end()) { DEBUG_HELPER_MESSAGE("No pre-request hooks in list"); }
	else
	{
		while (itvPreRequestHandlers != oContext.location.pre_request_handlers.end())
		{
			DEBUG_HELPER_MESSAGE("%s", itvPreRequestHandlers -> name.c_str());

			try
			{
				iRC = itvPreRequestHandlers -> object -> DataChunk(vChunk,
				                                                   iChunkSize,
				                                                   oRequest,
				                                                   *pGlobalPool,
				                                                   *pHostPool,
				                                                   oContext.request_pool,
				                                                   itvPreRequestHandlers -> handler_config,
				                                                   itvPreRequestHandlers -> context_data,
				                                                   oContext.imc,
				                                                   *pLogger);
			}
			catch(STLW::exception & e)
			{
				pLogger -> Crit("Fatal exception in pre-request handler `%s`: %s", itvPreRequestHandlers -> name.c_str(), e.what());
				iRC = -1;
			}
		}
	}

return iRC;
}

//
// Handle ful upload
//
INT_32 ASServer::HandleRFC1867File(ASRequestContext    & oContext,
                                   const STLW::string  & sName,
                                   const STLW::string  & sFullFileName,
                                   const STLW::string  & sFileName,
                                   const STLW::string  & sTempName,
                                   ASRequest           & oRequest,
                                   ASLogger            * pLogger)
{
	// Invoke pre-request handlers
	DEBUG_HELPER(pLogger, "Invoke RFC 1867 file handlers");

	STLW::vector<ASPreRequestHandlerConfig>::iterator itvPreRequestHandlers = oContext.location.pre_request_handlers.begin();
	INT_32 iRC = 0;
	if (itvPreRequestHandlers == oContext.location.pre_request_handlers.end()) { DEBUG_HELPER_MESSAGE("No RFC 1867 file handlers in list"); }
	else
	{
		while (itvPreRequestHandlers != oContext.location.pre_request_handlers.end())
		{
			DEBUG_HELPER_MESSAGE("%s", itvPreRequestHandlers -> name.c_str());

			try
			{
				iRC = itvPreRequestHandlers -> object -> RFC1867File(sName,
				                                                     sFullFileName,
				                                                     sFileName,
				                                                     sTempName,
				                                                     oRequest,
				                                                     *pGlobalPool,
				                                                     *pHostPool,
				                                                     oContext.request_pool,
				                                                     itvPreRequestHandlers -> handler_config,
				                                                     itvPreRequestHandlers -> context_data,
				                                                     oContext.imc,
				                                                     *pLogger);
			}
			catch(STLW::exception & e)
			{
				pLogger -> Crit("Fatal exception in pre-request handler `%s`: %s", itvPreRequestHandlers -> name.c_str(), e.what());
				iRC = -1;
			}
		}
	}
return iRC;
}

//
// Notify objects in global and host pools
//
INT_32 ASServer::NotifyObjects(const UINT_32 iEvent)
{
	// Notify objects from server pool
	pHostPool -> NotifyObjects<ASObject>(iEvent);
	// Notify objects from global pool
	pGlobalPool -> NotifyObjects<ASObject>(iEvent);

return 0;
}

//
// Handle request
//
INT_32 ASServer::RealHandleRequest(ASRequestContext  & oContext,
                                   ASRequest         & oRequest,
                                   ASResponseWriter  * pResponseWriter,
                                   ASLogger          * pLogger)
{
	DEBUG_HELPER(pLogger, "RealHandleRequest");

	// Response object;
	ASResponse  oResponse;
	// Set default response code
	oResponse.SetHTTPCode(oContext.location.default_response_code);
	// Set default content-type
	oResponse.SetContentType(oContext.location.default_content_type);

	// Invoke controller, if present
	if (oContext.location.controller.object != NULL)
	{
		DEBUG_HELPER_MESSAGE("Invoke controller");
		INT_32 iRC;
		try
		{
			iRC = oContext.location.controller.object -> Handler(oContext.data,
			                                                     oRequest,
			                                                     oResponse,
			                                                     *pGlobalPool,
			                                                     *pHostPool,
			                                                     oContext.request_pool,
			                                                     oContext.location.controller.handler_config,
			                                                     oContext.location.controller.context_data,
			                                                     oContext.imc,
			                                                     *pLogger);
		}
		catch(STLW::exception & e)
		{
			pLogger -> Crit("Fatal exception in controller `%s`: %s", oContext.location.controller.name.c_str(), e.what());
			return HANDLER_ERROR;
		}

		if (iRC == HANDLER_ERROR) { pLogger -> Crit("Fatal error in controller, exiting."); return -1; }
		// All done
		if (iRC == HANDLER_DONE) { DEBUG_HELPER_MESSAGE("All done, no further processing need"); return 0; }
	}

	// Invoke handlers
	DEBUG_HELPER_MESSAGE("Invoke handlers");
	STLW::vector<ASHandlerConfig>::iterator itvHandlers = oContext.location.handlers.begin();

	if (itvHandlers == oContext.location.handlers.end()) { DEBUG_HELPER_MESSAGE("No handlers in list"); }
	else
	{
		while (itvHandlers != oContext.location.handlers.end())
		{
			DEBUG_HELPER_MESSAGE(itvHandlers -> name.c_str());

			INT_32 iRC;
			try
			{
				iRC = itvHandlers -> object -> Handler(oContext.data,
				                                       oRequest,
				                                       oResponse,
				                                       *pGlobalPool,
				                                       *pHostPool,
				                                       oContext.request_pool,
				                                       itvHandlers -> handler_config,
				                                       itvHandlers -> context_data,
				                                       oContext.imc,
				                                       *pLogger);
			}
			catch(STLW::exception & e)
			{
				pLogger -> Crit("Fatal exception in handler `%s`: %s", itvHandlers -> name.c_str(), e.what());
				iRC = -1;
			}

			// Invoke finalizers if any error occured
			if (iRC == -1)
			{
				STLW::string sTMP("Fatal error in handler \"");
				sTMP.append(itvHandlers -> name);
				sTMP.append("\", run finalizers");
				pLogger -> Crit(sTMP.c_str());

				DEBUG_HELPER_MESSAGE(sTMP.c_str());

				// Run finalizers
				STLW::vector<ASHandlerConfig>::iterator itvFinalizers = oContext.location.finalizers.begin();
				while (itvFinalizers != oContext.location.finalizers.end())
				{
					try
					{
						iRC = itvFinalizers -> object -> Handler(oContext.data,
						                                         oRequest,
						                                         oResponse,
						                                         *pGlobalPool,
						                                         *pHostPool,
						                                         oContext.request_pool,
						                                         itvFinalizers -> handler_config,
						                                         itvFinalizers -> context_data,
						                                         oContext.imc,
						                                         *pLogger);
					}
					catch(STLW::exception & e)
					{
						pLogger -> Crit("Fatal exception in finalizer `%s`: %s", itvFinalizers -> name.c_str(), e.what());
						iRC = -1;
					}

					// Fatal error
					if (iRC == -1)
					{
						pLogger -> Crit("Fatal error in finalizer \"%s\", exiting", itvFinalizers -> name.c_str());
						DEBUG_HELPER_MESSAGE("Fatal error in finalizer \"%s\", exiting", itvFinalizers -> name.c_str());

						// Run FINALIZER fixups
						DEBUG_HELPER_MESSAGE("Run FINALIZER fixups");
						RunFixups(*pGlobalPool, *pHostPool, oContext.request_pool, oContext.imc, itvFinalizers, oContext.location.finalizers.begin(), pLogger);

						// Run HANDLER fixups
						DEBUG_HELPER_MESSAGE("Run HANDLER fixups");
						RunFixups(*pGlobalPool, *pHostPool, oContext.request_pool, oContext.imc, itvHandlers, oContext.location.handlers.begin(), pLogger);

						// Fatal error
						return -1;
					}

					// All done
					if (iRC == 1)
					{
						DEBUG_HELPER_MESSAGE("All done, no further processing need");
						break;
					}

					++itvFinalizers;
				}

				// Invoke VIEW
				break;
			}
			// All done
			if (iRC == 1)
			{
				DEBUG_HELPER_MESSAGE("All done, no further processing need");
				break;
			}
			++itvHandlers;
		}

		if (itvHandlers == oContext.location.handlers.end()) { --itvHandlers; }
		// Run fixups BEFORE VIEW invocation
		DEBUG_HELPER_MESSAGE("Run fixups BEFORE VIEW invocation");
		if (RunFixups(*pGlobalPool, *pHostPool, oContext.request_pool, oContext.imc, itvHandlers, oContext.location.handlers.begin(), pLogger) == -1)
		{
			return -1;
		}
	}

	INT_32 iRC = 0;
	DEBUG_HELPER_MESSAGE("Write response");

	// Write response
	const UINT_32 iViews = oContext.location.views.size();
	if (iViews == 1)
	{
		try
		{
			ASViewConfig & oTMP = oContext.location.views.begin() -> second;
			iRC = oTMP.object -> WriteResponse(oContext.data,
			                                   oResponse,
			                                   *pResponseWriter,
			                                   *pGlobalPool,
			                                   *pHostPool,
			                                   oContext.request_pool,
			                                   oTMP.view_config,
			                                   oTMP.context_data,
			                                   oContext.imc,
			                                   *pLogger);
		}
		catch(STLW::exception & e)
		{
			pLogger -> Crit("Fatal exception in view `%s`: %s", oContext.location.views.begin() -> first.c_str(), e.what());
			iRC = -1;
		}
	}
	// Get view by name
	else if (iViews > 1)
	{
		const STLW::string sViewName = oContext.imc.GetCDT("view").GetString();
		if (sViewName.empty())
		{
			try
			{
				ASViewConfig & oTMP = oContext.location.views.begin() -> second;
				iRC = oTMP.object -> WriteResponse(oContext.data,
				                                   oResponse,
				                                   *pResponseWriter,
				                                   *pGlobalPool,
				                                   *pHostPool,
				                                   oContext.request_pool,
				                                   oTMP.view_config,
				                                   oTMP.context_data,
				                                   oContext.imc,
				                                   *pLogger);
			}
			catch(STLW::exception & e)
			{
				pLogger -> Crit("Fatal exception in view `%s` (name not given in IMC): %s", oContext.location.views.begin() -> first.c_str(), e.what());
				iRC = -1;
			}
		}
		else
		{
			STLW::map<STLW::string, ASViewConfig>::iterator itmViews = oContext.location.views.find(sViewName);
			if (itmViews == oContext.location.views.end())
			{
				pLogger -> Crit("No such view `%s` in location `%s`", sViewName.c_str(), oContext.location.name.c_str());
				iRC = -1;
			}
			else
			{
				try
				{
					ASViewConfig & oTMP = itmViews -> second;
					iRC = oTMP.object -> WriteResponse(oContext.data,
					                                   oResponse,
					                                   *pResponseWriter,
					                                   *pGlobalPool,
					                                   *pHostPool,
					                                   oContext.request_pool,
					                                   oTMP.view_config,
					                                   oTMP.context_data,
					                                   oContext.imc,
					                                   *pLogger);
				}
				catch(STLW::exception & e)
				{
					pLogger -> Crit("Fatal exception in view `%s` (called by name through IMC): %s", itmViews -> first.c_str(), e.what());
					iRC = -1;
				}
			}
		}
	}
	else
	{
		pLogger -> Crit("Need to define at leat one view for location `%s`", oContext.location.name.c_str());
		return -1;
	}

// All done
return iRC;
}

//
// Get server configuration
//
ASHostConfig * ASServer::GetConfig() { return pHostConfig; }

//
// A destructor
//
ASServer::~ASServer() throw()
{
	;;
}

} // namespace CAS
// End.
