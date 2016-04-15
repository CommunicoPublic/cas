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
 *      ASExampleModule.cpp
 *
 * $CAS$
 */

#include <ASDebugHelper.hpp>
#include <ASLoadableObject.hpp>
#include <ASModule.hpp>
#include <ASPool.hpp>

#include "ASExampleObject.hpp"

using namespace CAS;

namespace TEST_NS
{

/**
  @class ASExampleModule ASExampleModule.hpp <ASExampleModule.cpp>
  @brief Application server object
*/
class ASExampleModule:
  public ASModule
{
public:
	/**
	  @brief A constructor
	*/
	ASExampleModule();

	/**
	  @brief A destructor
	*/
	~ASExampleModule() throw();

private:
	/** Pointer to example class */
	STLW::vector<ASObject *>  vExampleObjects;

	/**
	  @brief Initialize module
	  @param oConfiguration - module configuration
	  @param oModulesPool - pool of modules
	  @param oGlobalPool - pool of objects
	  @param oLogger - logger object
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 InitModule(CTPP::CDT  & oConfiguration,
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
	INT_32 InitServer(CTPP::CDT  & oConfiguration,
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
	INT_32 ShutdownServer(CTPP::CDT  & oConfiguration,
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
	INT_32 ShutdownModule(CTPP::CDT  & oConfiguration,
	                      ASPool     & oModulesPool,
	                      ASPool     & oGlobalPool,
	                      ASLogger   & oLogger);

	/**
	  @brief Get object type
	  @return human-readable type of module
	*/
	CCHAR_P GetModuleType() const;

	/**
	  @brief Get handler name
	*/
	CCHAR_P GetObjectName() const;
};

EXPORT_HANDLER(ASExampleModule)

//
// A constructor
//
ASExampleModule::ASExampleModule()
{
	;;
}

//
// Initialize module
//
INT_32 ASExampleModule::InitModule(CTPP::CDT  & oConfiguration,
                                   ASPool     & oModulesPool,
                                   ASPool     & oGlobalPool,
                                   ASLogger   & oLogger)
{
	DEBUG_HELPER(&oLogger, "ASExampleModule::InitModule");

return MODULE_OK;
}

//
// Initialize server hook
//
INT_32 ASExampleModule::InitServer(CTPP::CDT  & oConfiguration,
                                   ASPool     & oGlobalPool,
                                   ASPool     & oServerPool,
                                   ASLogger   & oLogger)
{
	DEBUG_HELPER(&oLogger, "ASExampleModule::InitServer");

	// Create per-server persistent resource
	ASObject * pExampleObject = new ASExampleObject(0);

	DEBUG_HELPER_MESSAGE("Register resource in server pool");

	// Register resource
	oServerPool.RegisterResource<ASObject>(pExampleObject, pExampleObject -> GetObjectName());

	vExampleObjects.push_back(pExampleObject);

return MODULE_OK;
}

//
// Shutdown server hook
//
INT_32 ASExampleModule::ShutdownServer(CTPP::CDT  & oConfiguration,
                                       ASPool     & oGlobalPool,
                                       ASPool     & oServerPool,
                                       ASLogger   & oLogger)
{
	DEBUG_HELPER(&oLogger, "ASExampleModule::ShutdownServer");

	STLW::vector<ASObject *>::iterator itvExampleObjects = vExampleObjects.begin();
	while (itvExampleObjects != vExampleObjects.end())
	{
		delete *itvExampleObjects;
		++itvExampleObjects;
	}

return MODULE_OK;
}

//
// Initialize module
//
INT_32 ASExampleModule::ShutdownModule(CTPP::CDT  & oConfiguration,
                                       ASPool     & oModulesPool,
                                       ASPool     & oGlobalPool,
                                       ASLogger   & oLogger)
{
	DEBUG_HELPER(&oLogger, "ASExampleModule::ShutdownModule");

return MODULE_OK;
}

//
// Get object type
//
CCHAR_P ASExampleModule::GetModuleType() const { return "ASExampleClassHolder"; }

//
// Get module name
//
CCHAR_P ASExampleModule::GetObjectName() const { return "ASExampleModule"; }

//
// A destructor
//
ASExampleModule::~ASExampleModule() throw()
{
	;;
}

} // namespace TEST_NS
// End.
