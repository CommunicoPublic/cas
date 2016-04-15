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
 *      ASServerManager.cpp
 *
 * $CAS$
 */

#include "ASServerManager.hpp"

#include "ASDebugHelper.hpp"
#include "ASException.hpp"
#include "ASGlobalConfigHandler.hpp"
#include "ASHandler.hpp"
#include "ASHostConfigHandler.hpp"
#include "ASLoadableObject.hpp"
#include "ASModule.hpp"
#include "ASPreRequestHandler.hpp"
#include "ASServer.hpp"
#include "ASUtil.hpp"
#include "ASView.hpp"
#include "ASXMLParser.hpp"

#include <CDT.hpp>

#include <errno.h>
#include <unistd.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif // HOST_NAME_MAX

namespace CAS // C++ Application Server
{
// FWD
class ASLogger;

//
// Consrtructor
//
ASServerManager::ASServerManager(ASLogger * pLogger): oObjectLoader(C_CAS_INIT_SYM_PREFIX),
                                                      pSyscallFactory(NULL)
{
	DEBUG_HELPER(pLogger, "ASServerManager");

	CHAR_8 szHostname[HOST_NAME_MAX + 1];
	INT_32 iRC = gethostname(szHostname, HOST_NAME_MAX);

	if (iRC == 0) { sSystemHostName.assign(szHostname); }
	else          { sSystemHostName.assign("unknown");  }
}

//
// Initialize application server instance
//
INT_32 ASServerManager::InitManager(CCHAR_P szGlobalConfig, ASLogger * pLogger)
{
	DEBUG_HELPER(pLogger, STLW::string("ASServerManager::InitManager \"") + szGlobalConfig + "\"");

	// Try to open global configuration file
	FILE * F = fopen(szGlobalConfig, "rb");
	if (F == NULL)
	{
		INT_32 iErrNo = errno;

		CHAR_8 szTMP[1024];
		snprintf(szTMP, 1024, "Cannot open main configuration file \"%s\" for reading: %s", szGlobalConfig, strerror(iErrNo));

		DEBUG_HELPER_MESSAGE("%s", szTMP);
		pLogger -> Crit("%s", szTMP);

		throw UnixException(szTMP, iErrNo);
	}

	try
	{
		DEBUG_HELPER_MESSAGE("Parse global config");

		// Store path to file as include directory
		CCHAR_P szTMP = szGlobalConfig + strlen(szGlobalConfig);
		while (szTMP != szGlobalConfig && *szTMP != '/' && *szTMP != '\\') { --szTMP; }

		STLW::vector<STLW::string> vIncludeDirs;
		if (szTMP != szGlobalConfig) { vIncludeDirs.push_back(STLW::string(szGlobalConfig, (szTMP - szGlobalConfig))); }

		// Create global config parser object
		ASGlobalConfigHandler oHandler(oGlobalConfig, vIncludeDirs);

		// Create generic XML parser
		ASXMLParser oParser(&oHandler);

		// Parse configuration
		if (oParser.ParseFile(F) == -1) { throw LogicError(oHandler.GetError().c_str()); }
		fclose(F);

		DEBUG_HELPER_MESSAGE("Parse OK");
	}
	catch(...)
	{
		fclose(F);
		throw;
	}

	DEBUG_HELPER_MESSAGE("Load modules");

	// Set system host name
	oGlobalConfig.system_hostname.assign(sSystemHostName);

	// Okay, try to load modules
	STLW::vector<ASObjectConfig>::iterator itmModulesList = oGlobalConfig.modules_list.begin();
	while (itmModulesList != oGlobalConfig.modules_list.end())
	{
		STLW::string sFullFileName = CheckFile(oGlobalConfig.libexec_dirs, itmModulesList -> library, pLogger);
		if (sFullFileName.empty())
		{
			CHAR_8 szTMP[1024];
			snprintf(szTMP, 1024, "Cannot find file \"%s\"", itmModulesList -> library.c_str());

			DEBUG_HELPER_MESSAGE("%s", szTMP);
			pLogger -> Crit("%s", szTMP);

			throw LogicError(szTMP);
		}
		DEBUG_HELPER_MESSAGE("Found library \"%s\"", sFullFileName.c_str());

		// Okay, search it in pool
		// Object already present, throw exception
		if (oModulesPool.GetResourceByName<ASObject>(itmModulesList -> name) != NULL)
		{
			CHAR_8 szTMP[1024];
			snprintf(szTMP, 1024, "Module \"%s\" already present", itmModulesList -> name.c_str());

			DEBUG_HELPER_MESSAGE("%s", szTMP);
			pLogger -> Crit("%s", szTMP);

			throw LogicError(szTMP);
		}

		// Object not found, try to load it from specified library
		itmModulesList -> object = oObjectLoader.GetObject(sFullFileName.c_str(), itmModulesList -> name.c_str());

		// Register it in pool
		oModulesPool.RegisterResource<ASObject>(itmModulesList -> object, itmModulesList -> name);

		++itmModulesList;
	}

	DEBUG_HELPER_MESSAGE("Initialize modules");

	// Initialize modules
	itmModulesList = oGlobalConfig.modules_list.begin();
	while (itmModulesList != oGlobalConfig.modules_list.end())
	{
		// Only if object is derived from ASModule
		if (strcasecmp("ASModule", itmModulesList -> object -> GetObjectType()) == 0)
		{
			// Paranoid check
			ASModule * pModule = dynamic_cast<ASModule *>(itmModulesList -> object);
			if (pModule != NULL)
			{
				try
				{
					// Initialize module and check error
					if (pModule -> InitModule(itmModulesList -> configuration,
					                          oModulesPool,
					                          oGlobalPool,
					                          *pLogger) == -1)
					{
						return -1;
					}
				}
				// STL exception
				catch(STLW::exception & e) { STLW::string sTMP("Exception in module \""); sTMP.append(itmModulesList -> name); sTMP.append("\": "); sTMP.append(e.what()); DEBUG_HELPER_MESSAGE(sTMP.c_str()); throw LogicError(sTMP.c_str()); }
				// Unknown error
				catch(...)                 { STLW::string sTMP("Exception in module \""); sTMP.append(itmModulesList -> name); sTMP.append("\"."); DEBUG_HELPER_MESSAGE(sTMP.c_str()); throw LogicError(sTMP.c_str()); }
			}
		}

		++itmModulesList;
	}

return 0;
}

//
// Initialize virtual host
//
INT_32 ASServerManager::InitServer(CCHAR_P     szHostName,
                                   CCHAR_P     szHostConfig,
                                   ASLogger  * pLogger)
{
	DEBUG_HELPER(pLogger, STLW::string("ASServerManager::InitServer \"") + szHostName + "\", \"" + szHostConfig + "\"");

	// Check virtual host existence
	if (GetServer(szHostName, pLogger) != NULL)
	{
		DEBUG_HELPER_MESSAGE("Server \"%s\" already exist", szHostName);
		pLogger -> Crit("Server \"%s\" already exist", szHostName);
		return 0;
	}

	// Try to open global configureation file
	FILE * F = fopen(szHostConfig, "rb");
	if (F == NULL)
	{
		CHAR_8 szTMP[1024];
		snprintf(szTMP, 1024, "Cannot open host configuration file \"%s\" for reading; error: %s", szHostConfig, strerror(errno));

		DEBUG_HELPER_MESSAGE("%s", szTMP);
		pLogger -> Crit("%s", szTMP);

		throw UnixException(szTMP, errno);
	}

	ASHostConfig * pHostConfig = NULL;
	try
	{
		DEBUG_HELPER_MESSAGE("Parse host config");

		// Host configuration
		pHostConfig = new ASHostConfig;
		// Create global config parser object
		ASHostConfigHandler oHandler(*pHostConfig);

		// Create generic XML parser
		ASXMLParser oParser(&oHandler);

		// Parse configuration
		if (oParser.ParseFile(F) == -1) { throw LogicError(oHandler.GetError().c_str()); }
		fclose(F);
	}
	catch(...)
	{
		fclose(F);
		// Clear garbage
		ClearHostConfig(pHostConfig);
		throw;
	}

	DEBUG_HELPER_MESSAGE("Create new instance of application server");

	// Create new instance of application server
	ASServerContext oServerContext;
	oServerContext.host_config        = pHostConfig;
	oServerContext.host_pool          = new ASPool;
	oServerContext.application_server = new ASServer(&oGlobalConfig,
	                                                 oServerContext.host_config,
	                                                 &oGlobalPool,
	                                                 oServerContext.host_pool,
	                                                 pLogger);

	DEBUG_HELPER_MESSAGE("Setup application server");
	try
	{
		DEBUG_HELPER_MESSAGE("Setup modules");

		// Invoke per-server initializers
		STLW::vector<ASModuleConfig>::iterator itmModules = pHostConfig -> modules_config.begin();
		while (itmModules != pHostConfig -> modules_config.end())
		{
			// Get module by name from pool
			ASModule * pModule = GetModule(itmModules -> module_name.c_str(), pLogger);

			// Invoke method
			if (pModule -> InitServer(itmModules -> module_config,
			                          oGlobalPool,
			                          *(oServerContext.host_pool),
			                          *pLogger) != HANDLER_OK)
			{
				CHAR_8 szTMP[1024];
				snprintf(szTMP, 1024, "Host \"%s\": cannot initialize module \"%s\"", szHostName, itmModules -> module_name.c_str());

				DEBUG_HELPER_MESSAGE("%s", szTMP);
				pLogger -> Crit("%s", szTMP);

				throw LogicError(szTMP);
			}

			++itmModules;
		}

		// Setup handlers by locations
		STLW::vector<ASLocation>::iterator itvLocations = pHostConfig -> locations.begin();
		while (itvLocations != pHostConfig -> locations.end())
		{
			DEBUG_HELPER_MESSAGE("Setup pre-request handlers");

			// Setup pre-request handlers
			STLW::vector<ASPreRequestHandlerConfig>::iterator itvPreRequestHandlers = itvLocations -> pre_request_handlers.begin();
			while (itvPreRequestHandlers != itvLocations -> pre_request_handlers.end())
			{
				// Setup handler
				itvPreRequestHandlers -> object = SetupPreRequestHandler(itvPreRequestHandlers -> name.c_str(), pLogger);
				// Check error
				if (itvPreRequestHandlers -> object -> InitLocation(oGlobalPool,
				                                                    *oServerContext.host_pool,
				                                                    itvPreRequestHandlers -> handler_config,
				                                                    itvPreRequestHandlers -> context_data,
				                                                    *pLogger) != HOOK_OK)
				{
					CHAR_8 szTMP[1024];
					snprintf(szTMP, 1024, "Host \"%s\", location \"%s\": cannot initialize pre-request handler \"%s\"",
					                      szHostName, itvLocations -> name.c_str(), itvPreRequestHandlers -> name.c_str());

					DEBUG_HELPER_MESSAGE("%s", szTMP);
					pLogger -> Crit("%s", szTMP);

					// Throw exception
					throw LogicError(szTMP);
				}
				++itvPreRequestHandlers;
			}

			DEBUG_HELPER_MESSAGE("Setup controller");

			// Skip if controller object not present
			if (itvLocations -> controller.name.empty()) { itvLocations -> controller.object = NULL; }
			else
			{
				// Setup controller
				itvLocations -> controller.object = SetupHandler(itvLocations -> controller.name.c_str(), pLogger);
				// Check error
				if (itvLocations -> controller.object -> InitLocation(oGlobalPool,
				                                                      *oServerContext.host_pool,
				                                                      itvLocations -> controller.handler_config,
				                                                      itvLocations -> controller.context_data,
				                                                      *pLogger) != HANDLER_OK)
				{
					CHAR_8 szTMP[1024];
					snprintf(szTMP, 1024, "Host \"%s\", location \"%s\": cannot initialize controller \"%s\"",
					                      szHostName, itvLocations -> name.c_str(), itvLocations -> controller.name.c_str());

					DEBUG_HELPER_MESSAGE("%s", szTMP);
					pLogger -> Crit("%s", szTMP);

					throw LogicError(szTMP);
				}
			}

			DEBUG_HELPER_MESSAGE("Setup handlers");

			// Setup handlers
			STLW::vector<ASHandlerConfig>::iterator itvHandlers = itvLocations -> handlers.begin();
			while (itvHandlers != itvLocations -> handlers.end())
			{
				// Setup handler
				itvHandlers -> object = SetupHandler(itvHandlers -> name.c_str(), pLogger);
				// Check error
				if (itvHandlers -> object -> InitLocation(oGlobalPool,
				                                          *oServerContext.host_pool,
				                                          itvHandlers -> handler_config,
				                                          itvHandlers -> context_data,
				                                          *pLogger) != HANDLER_OK)
				{
					CHAR_8 szTMP[1024];
					snprintf(szTMP, 1024, "Host \"%s\", location \"%s\": cannot initialize handler \"%s\"",
					                      szHostName, itvLocations -> name.c_str(), itvHandlers -> name.c_str());

					DEBUG_HELPER_MESSAGE("%s", szTMP);
					pLogger -> Crit("%s", szTMP);

					throw LogicError(szTMP);
				}
				++itvHandlers;
			}

			DEBUG_HELPER_MESSAGE("Setup finalizers");

			// Setup finalizers
			itvHandlers = itvLocations -> finalizers.begin();
			while (itvHandlers != itvLocations -> finalizers.end())
			{
				// Setup handler
				itvHandlers -> object = SetupHandler(itvHandlers -> name.c_str(), pLogger);
				// Check error
				if (itvHandlers -> object -> InitLocation(oGlobalPool,
				                                          *oServerContext.host_pool,
				                                          itvHandlers -> handler_config,
				                                          itvHandlers -> context_data,
				                                          *pLogger) != HANDLER_OK)
				{
					CHAR_8 szTMP[1024];
					snprintf(szTMP, 1024, "Host \"%s\", location \"%s\": cannot initialize finalizer \"%s\"",
					                      szHostName, itvLocations -> name.c_str(), itvHandlers -> name.c_str());

					DEBUG_HELPER_MESSAGE("%s", szTMP);
					pLogger -> Crit("%s", szTMP);

					throw LogicError(szTMP);
				}
				++itvHandlers;
			}

			DEBUG_HELPER_MESSAGE("Setup view");

			ASViewMap::iterator itmViews = itvLocations -> views.begin();
			while (itmViews != itvLocations -> views.end())
			{
				// Setup view
				itmViews -> second.object = SetupView(itmViews -> first.c_str(), pLogger);
				// Check error
				if (itmViews -> second.object -> InitLocation(oGlobalPool,
				                                              *oServerContext.host_pool,
				                                              itmViews -> second.view_config,
				                                              itmViews -> second.context_data,
				                                              *pLogger) != HANDLER_OK)
				{
					CHAR_8 szTMP[1024];
					snprintf(szTMP, 1024, "Host \"%s\", location \"%s\": cannot initialize finalizer \"%s\"",
					                      szHostName, itvLocations -> name.c_str(), itmViews -> first.c_str());

					DEBUG_HELPER_MESSAGE("%s", szTMP);
					pLogger -> Crit("%s", szTMP);

					throw LogicError(szTMP);
				}
				++itmViews;
			}
			++itvLocations;
		}
	}
	catch(...)
	{
		// Clear garbage in host configuration
		ClearHostConfig(pHostConfig);
		// Remove host pool
		delete oServerContext.host_pool;
		// Remove application server instance
		delete oServerContext.application_server;
		// Escalate exception to upper level
		throw;
	}

	DEBUG_HELPER_MESSAGE("Setup OK");

	// Store server context
	mVirtualServers[szHostName] = oServerContext;

return 0;
}

//
// Get server context by server name
//
ASServer * ASServerManager::GetServer(CCHAR_P szHostName, ASLogger * pLogger)
{
	DEBUG_HELPER(pLogger, "ASServerManager::GetServer");

	STLW::map<STLW::string, ASServerContext>::iterator itmVirtualServers = mVirtualServers.find(szHostName);
	if (itmVirtualServers == mVirtualServers.end())
	{
		DEBUG_HELPER_MESSAGE("Server \"%s\"", szHostName);
		return NULL;
	}

return itmVirtualServers -> second.application_server;
}

//
// Get module with specified name
//
ASModule * ASServerManager::GetModule(CCHAR_P szModuleName, ASLogger * pLogger)
{
	DEBUG_HELPER(pLogger, STLW::string("ASServerManager::GetModule \"") + szModuleName + "\"");
	// Check resource
	if (oModulesPool.CheckResourceByName<ASObject>(szModuleName) == -1)
	{
		STLW::string sTMP("No such module \"");
		sTMP.append(szModuleName);
		sTMP.append("\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

	// Get resource
	ASModule * pModule = oModulesPool.GetResourceByName<ASModule>(szModuleName);
	// Can't cast to type
	if (pModule == NULL)
	{
		STLW::string sTMP("Can't cast object \"");
		sTMP.append(szModuleName);
		sTMP.append("\" to type \"ASModule\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

return pModule;
}

//
// Setup pre-request handler with specified name
//
ASPreRequestHandler * ASServerManager::SetupPreRequestHandler(CCHAR_P szHandlerName, ASLogger * pLogger)
{
	DEBUG_HELPER(pLogger, STLW::string("ASServerManager::SetupPreRequestHandler \"") + szHandlerName + "\"");
	// Check resource
	if (oModulesPool.CheckResourceByName<ASObject>(szHandlerName) == -1)
	{
		STLW::string sTMP("No such module \"");
		sTMP.append(szHandlerName);
		sTMP.append("\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

	// Get resource
	ASPreRequestHandler * pHandler = oModulesPool.GetResourceByName<ASPreRequestHandler>(szHandlerName);
	// Can't cast to type
	if (pHandler == NULL)
	{
		STLW::string sTMP("Can't cast object \"");
		sTMP.append(szHandlerName);
		sTMP.append("\" to type \"ASPreRequestHandler\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

	CCHAR_P szModuleType = static_cast<ASModule *>(pHandler) -> GetModuleType();
	// Check object type
	if (strcmp("ASPreRequestHandler", szModuleType) != 0)
	{
		STLW::string sTMP("Module \"");
		sTMP.append(szHandlerName);
		sTMP.append("\" has type \"");
		sTMP.append(szModuleType);
		sTMP.append("\" but need \"ASPreRequestHandler\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

return pHandler;
}

//
// Setup handler with specified name
//
ASHandler * ASServerManager::SetupHandler(CCHAR_P szHandlerName, ASLogger * pLogger)
{
	DEBUG_HELPER(pLogger, STLW::string("ASServerManager::SetupHandler \"") + szHandlerName + "\"");
	// Check resource
	if (oModulesPool.CheckResourceByName<ASObject>(szHandlerName) == -1)
	{
		STLW::string sTMP("No such module \"");
		sTMP.append(szHandlerName);
		sTMP.append("\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

	// Get resource
	ASHandler * pHandler = oModulesPool.GetResourceByName<ASHandler>(szHandlerName);
	// Can't cast to type
	if (pHandler == NULL)
	{
		STLW::string sTMP("Can't cast object \"");
		sTMP.append(szHandlerName);
		sTMP.append("\" to type \"ASHandler\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

	CCHAR_P szModuleType = static_cast<ASModule *>(pHandler) -> GetModuleType();
	// Check object type
	if (strcmp("ASHandler", szModuleType) != 0)
	{
		STLW::string sTMP("Module \"");
		sTMP.append(szHandlerName);
		sTMP.append("\" has type \"");
		sTMP.append(szModuleType);
		sTMP.append("\" but need \"ASHandler\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

return pHandler;
}

//
// Setup view with specified name
//
ASView * ASServerManager::SetupView(CCHAR_P szViewName, ASLogger * pLogger)
{
	DEBUG_HELPER(pLogger, STLW::string("ASServerManager::SetupView \"") + szViewName + "\"");
	// Check resource
	if (oModulesPool.CheckResourceByName<ASObject>(szViewName) == -1)
	{
		STLW::string sTMP("No such module \"");
		sTMP.append(szViewName);
		sTMP.append("\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

	// Get resource
	ASView * pView = oModulesPool.GetResourceByName<ASView>(szViewName);
	// Can't cast to type
	if (pView == NULL)
	{
		STLW::string sTMP("Can't cast object \"");
		sTMP.append(szViewName);
		sTMP.append("\" to type \"ASView\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

	CCHAR_P szModuleType = static_cast<ASModule *>(pView) -> GetModuleType();
	// Check object type
	if (strcmp("ASView", szModuleType) != 0)
	{
		STLW::string sTMP("Module \"");
		sTMP.append(szViewName);
		sTMP.append("\" has type \"");
		sTMP.append(szModuleType);
		sTMP.append("\" but need \"ASView\"");

		DEBUG_HELPER_MESSAGE(sTMP.c_str());
		throw LogicError(sTMP.c_str());
	}

return pView;
}

//
// Clear garbage in host configuration object
//
void ASServerManager::ClearHostConfig(ASHostConfig * pHostConfig)
{
	// Clear garbage
	if (pHostConfig != NULL)
	{
		STLW::vector<ASLocation>::iterator itvLocations = pHostConfig -> locations.begin();
		while (itvLocations != pHostConfig -> locations.end())
		{
			ASLocation::ClearLocation(*itvLocations);
			++itvLocations;
		}
		delete pHostConfig;
	}
}

//
// Shut down server manager
//
void ASServerManager::ShutdownManager(ASLogger * pLogger)
{
	DEBUG_HELPER(pLogger, "ASServerManager::ShutdownManager");

	DEBUG_HELPER_MESSAGE("Clear virtual hosts");
	// Clear virtual hosts
	STLW::map<STLW::string, ASServerContext>::iterator itmVirtualServers = mVirtualServers.begin();
	while(itmVirtualServers != mVirtualServers.end())
	{
		ASHostConfig * pHostConfig = itmVirtualServers -> second.host_config;
		STLW::vector<ASLocation>::iterator itvLocations = pHostConfig -> locations.begin();
		while (itvLocations != pHostConfig -> locations.end())
		{
			DEBUG_HELPER_MESSAGE("Terminate controller");
			// Skip if controller object not present
			if (itvLocations -> controller.object != NULL)
			{
				// Check error
				if (itvLocations -> controller.object -> ShutdownLocation(oGlobalPool,
				                                                          *(itmVirtualServers -> second.host_pool),
				                                                          itvLocations -> controller.handler_config,
				                                                          itvLocations -> controller.context_data,
				                                                          *pLogger) != HANDLER_OK)
				{
					STLW::string sTMP = STLW::string("Host \"") + itmVirtualServers -> first +
					                    "\", location \"" + itvLocations -> name +
					                    "\": cannot terminate controller \"" + itvLocations -> controller.name + "\"";
					DEBUG_HELPER_MESSAGE(sTMP.c_str());
					pLogger -> Crit(sTMP.c_str());
				}
			}

			DEBUG_HELPER_MESSAGE("Terminate handlers");
			// Setup handlers
			STLW::vector<ASHandlerConfig>::iterator itvHandlers = itvLocations -> handlers.begin();
			while (itvHandlers != itvLocations -> handlers.end())
			{
				// Shutdown location
				if (itvHandlers -> object -> ShutdownLocation(oGlobalPool,
				                                              *(itmVirtualServers -> second.host_pool),
				                                              itvHandlers -> handler_config,
				                                              itvHandlers -> context_data,
				                                              *pLogger) != HANDLER_OK)
				{
					STLW::string sTMP = STLW::string("Host \"") + itmVirtualServers -> first +
					                    "\", location \"" + itvLocations -> name +
					                    "\": cannot terminate handler \"" + itvHandlers -> name + "\"";
					DEBUG_HELPER_MESSAGE(sTMP.c_str());
					pLogger -> Crit(sTMP.c_str());
				}
				++itvHandlers;
			}

			DEBUG_HELPER_MESSAGE("Terminate view(s)");
			ASViewMap::iterator itmViews = itvLocations -> views.begin();
			while (itmViews != itvLocations -> views.end())
			{
				// Check error
				if (itmViews -> second.object -> ShutdownLocation(oGlobalPool,
				                                                  *(itmVirtualServers -> second.host_pool),
				                                                  itmViews -> second.view_config,
				                                                  itmViews -> second.context_data,
				                                                  *pLogger) != HANDLER_OK)
				{
					STLW::string sTMP = STLW::string("Host \"") + itmVirtualServers -> first +
					                    "\", location \"" + itvLocations -> name +
					                    "\": cannot terminate view \"" + itmViews -> first + "\"";
					DEBUG_HELPER_MESSAGE(sTMP.c_str());
					pLogger -> Crit(sTMP.c_str());
				}
				++itmViews;
			}

			ASLocation::ClearLocation(*itvLocations);
			++itvLocations;
		}

		// Invoke per-server shutdown hooks
		STLW::vector<ASModuleConfig>::reverse_iterator itmModules = pHostConfig -> modules_config.rbegin();
		while (itmModules != pHostConfig -> modules_config.rend())
		{
			// Get module by name from pool
			ASModule * pModule = GetModule(itmModules -> module_name.c_str(), pLogger);
			// Invoke method
			if (pModule -> ShutdownServer(itmModules -> module_config,
			                              oGlobalPool,
			                              *(itmVirtualServers -> second.host_pool),
			                              *pLogger) != HANDLER_OK)
			{
				STLW::string sTMP = STLW::string("Host \"") + itmVirtualServers -> first +
				                    "\": cannot shutdown module \"" + itmModules -> module_name + "\", continue anyway";
				DEBUG_HELPER_MESSAGE(sTMP.c_str());
				pLogger -> Crit(sTMP.c_str());
				// Throw exception
			}

			++itmModules;
		}
		delete pHostConfig;

		// Destroy host data pool
		itmVirtualServers -> second.host_pool -> ClearPool<ASObject>();

		delete itmVirtualServers -> second.host_pool;
		delete itmVirtualServers -> second.application_server;

		++itmVirtualServers;
	}
	{ STLW::map<STLW::string, ASServerContext> mTMP; mVirtualServers.swap(mTMP); }

	DEBUG_HELPER_MESSAGE("Destroy global data pool");
	oGlobalPool.ClearPool<ASObject>();

	DEBUG_HELPER_MESSAGE("Clear global objects");
	// Clear global objects
	STLW::vector<ASObjectConfig>::reverse_iterator itvModulesList = oGlobalConfig.modules_list.rbegin();
	while (itvModulesList != oGlobalConfig.modules_list.rend())
	{
		// Uninitialized module?
		if (itvModulesList -> object != NULL)
		{
			ASModule * pModule = dynamic_cast<ASModule *>(itvModulesList -> object);
			try
			{
				// Initialize module and check error
				if (pModule -> ShutdownModule(itvModulesList -> configuration,
				                              oModulesPool,
				                              oGlobalPool,
				                              *pLogger) == -1)
				{
					STLW::string sTMP("Cannot shutdown module \"");
					sTMP += itvModulesList -> name + "\", continue anyway";
					DEBUG_HELPER_MESSAGE(sTMP.c_str());
				}
			}
			// STL exception
			catch(STLW::exception & e) { STLW::string sTMP("Exception in module \""); sTMP.append(itvModulesList -> name); sTMP.append("\":"); sTMP.append(e.what()); DEBUG_HELPER_MESSAGE(sTMP.c_str()); }
			// Unknown error
			catch(...)                 { STLW::string sTMP("Exception in module \""); sTMP.append(itvModulesList -> name); sTMP.append("\"."); DEBUG_HELPER_MESSAGE(sTMP.c_str()); }

			if (pModule != NULL) { delete pModule; }
		}

		++itvModulesList;
	}

	{ STLW::vector<ASObjectConfig> vTMP; oGlobalConfig.modules_list.swap(vTMP); }
}

//
// A destructor
//
ASServerManager::~ASServerManager() throw()
{
	;;
}

} // namespace CAS
// End.
