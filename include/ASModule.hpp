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
 *      ASModule.hpp
 *
 * $CAS$
 */
#ifndef _AS_MODULE_HPP__
#define _AS_MODULE_HPP__ 1

/**
  @file ASModule.hpp
  @brief Application server loadable module
*/
#include "ASObject.hpp"

#define MODULE_OK         0  // Success
#define MODULE_ERROR     -1  // Error occured

namespace CTPP
{
// FWD
class CDT;
}

namespace CAS // C++ Application Server
{
// FWD
class ASPool;
class ASLogger;

/**
  @class ASModule ASModule.hpp <ASModule.hpp>
  @brief Application server loadable module
*/
class ASModule:
  public ASObject
{
public:
	/**
	  @brief Initialize module
	  @param oConfiguration - module configuration
	  @param oModulesPool - pool of modules
	  @param oGlobalPool - global pool of objects
	  @param oLogger - logger object
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 InitModule(CTPP::CDT  & oConfiguration,
	                          ASPool     & oModulesPool,
	                          ASPool     & oGlobalPool,
	                          ASLogger   & oLogger);

	/**
	  @brief Initialize server hook
	  @param oConfiguration - module configuration
	  @param oGlobalPool - pool of global objects
	  @param oServerPool - pool of per-server objects
	  @param oLogger - logger object
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 InitServer(CTPP::CDT  & oConfiguration,
	                          ASPool     & oGlobalPool,
	                          ASPool     & oServerPool,
	                          ASLogger   & oLogger);

	/**
	  @brief Shutdown server hook
	  @param oConfiguration - module configuration
	  @param oGlobalPool - pool of global objects
	  @param oServerPool - pool of per-server objects
	  @param oLogger - logger object
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 ShutdownServer(CTPP::CDT  & oConfiguration,
	                              ASPool     & oGlobalPool,
	                              ASPool     & oServerPool,
	                              ASLogger   & oLogger);

	/**
	  @brief Shutdown module
	  @param oConfiguration - module configuration
	  @param oModulesPool - pool of modules
	  @param oGlobalPool - global pool of objects
	  @param oLogger - logger object
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 ShutdownModule(CTPP::CDT  & oConfiguration,
	                              ASPool     & oModulesPool,
	                              ASPool     & oGlobalPool,
	                              ASLogger   & oLogger);

	/**
	  @brief Get object type
	  @return human-readable type of module
	*/
	virtual CCHAR_P GetModuleType() const = 0;

	/**
	  @brief A virtual destructor
	*/
	virtual ~ASModule() throw();

private:
	/**
	  @brief Get object type
	  @return human-readable type of object ("ASModule")
	*/
	virtual CCHAR_P GetObjectType() const;
};

} // namespace CAS
#endif // _AS_MODULE_HPP__
// End.
