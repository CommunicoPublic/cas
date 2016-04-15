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
 *      ASServerManager.hpp
 *
 * $CAS$
 */
#ifndef _AS_SERVER_MANAGER_HPP__
#define _AS_SERVER_MANAGER_HPP__ 1

/**
  @file ASServerManager.hpp
  @brief Application server manager
*/

#include "ASGlobalConfig.hpp"
#include "ASObjectLoader.hpp"
#include "ASServerContext.hpp"

namespace CTPP
{
// FWD
class SyscallFactory;
}

namespace CAS // C++ Application Server
{
// FWD
class ASLogger;
class ASModule;
class ASServer;

/**
  @class ASServerManager ASServerManager.hpp <ASServerManager.hpp>
  @brief Application server manager
*/
class ASServerManager
{
public:
	/**
	  @brief Consrtructor
	  @param pLogger - logger
	*/
	ASServerManager(ASLogger * pLogger);

	/**
	  @brief Initialize application server instance
	  @param szGlobalConfig - global configuration file
	  @param pLogger - logger
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 InitManager(CCHAR_P szGlobalConfig, ASLogger * pLogger);

	/**
	  @brief Initialize application server instance for host
	  @param szHostName - name of virtual host
	  @param szHostConfig - host configuration
	  @param pLogger - logger
	  @return pointer to resource, or NULL if any error occured
	*/
	INT_32 InitServer(CCHAR_P szHostName, CCHAR_P szHostConfig, ASLogger * pLogger);

	/**
	  @brief Get server context by host name
	  @param szHostName - Virtual host name
	  @param pLogger - logger
	  @return pointer to resource, or NULL if any error occured
	*/
	ASServer * GetServer(CCHAR_P szHostName, ASLogger * pLogger);

	/**
	  @brief Shut down server manager
	  @param pLogger - logger
	*/
	void ShutdownManager(ASLogger * pLogger);

	/**
	  @brief A destructor
	*/
	~ASServerManager() throw();

public:
	// Does not exist
	ASServerManager(const ASServerManager  & oRhs);
	ASServerManager& operator=(const ASServerManager  & oRhs);

	/** Global CAS configuration          */
	ASGlobalConfig                             oGlobalConfig;
	/** Global pool of objects            */
	ASPool                                     oGlobalPool;
	/** List of server contextes          */
	STLW::map<STLW::string, ASServerContext>   mVirtualServers;
	/** Object loader                     */
	ASObjectLoader                             oObjectLoader;
	/** Modules pool                      */
	ASPool                                     oModulesPool;
	/** CTPP syscall factory              */
	CTPP::SyscallFactory                     * pSyscallFactory;
	/** Main host name                    */
	STLW::string                               sSystemHostName;

	/**
	  @brief  Get module with specified name
	  @param szModuleName - module name
	  @param pLogger - logger
	  @return pointer to module
	*/
	ASModule * GetModule(CCHAR_P     szModuleName,
	                     ASLogger  * pLogger);

	/**
	  @brief Setup handler with specified name
	  @param szHandlerName - handler name from location config
	  @param pLogger - logger
	  @return pointer to handler
	*/
	ASHandler * SetupHandler(CCHAR_P     szHandlerName,
	                         ASLogger  * pLogger);

	/**
	  @brief Setup pre-request handler with specified name
	  @param szHandlerName - pre-request handler name from location config
	  @param pLogger - logger
	  @return pointer to handler
	*/
	ASPreRequestHandler * SetupPreRequestHandler(CCHAR_P     szHandlerName,
	                                             ASLogger  * pLogger);

	/**
	  @brief Setup view with specified name
	  @param szViewName - view name from location config
	  @param pLogger - logger
	  @return pointer to view object
	*/
	ASView * SetupView(CCHAR_P     szViewName,
	                   ASLogger  * pLogger);

	/**
	  @brief Clear garbage in host configuration object
	  @param pHostConfig - object to clear
	*/
	void ClearHostConfig(ASHostConfig * pHostConfig);
};

} // namespace CAS
#endif // _AS_SERVER_MANAGER_HPP__
// End.
