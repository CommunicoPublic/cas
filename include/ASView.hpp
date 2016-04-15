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
 *      ASView.hpp
 *
 * $CAS$
 */
#ifndef _AS_VIEW_HPP__
#define _AS_VIEW_HPP__ 1

/**
  @file ASView.hpp
  @brief Application server view class
*/
#include "ASLogger.hpp"
#include "ASModule.hpp"
#include "ASPool.hpp"
#include "ASRequest.hpp"
#include "ASResponse.hpp"
#include "ASResponseWriter.hpp"

#include "STLVector.hpp"

namespace CAS // C++ Application Server
{

#define VIEW_OK         0             // Success
#define VIEW_ERROR     -1             // Error occured

/**
  @class ASView ASView.hpp <ASView.hpp>
  @brief Application server view class
*/
class ASView:
  public ASModule
{
public:
	/**
	  @brief Initialize handler
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oLocationConfig - handler configuration
	  @param pContextData - context data
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured
	*/
	virtual INT_32 InitLocation(ASPool       & oGlobalPool,
	                            ASPool       & oVhostPool,
	                            CTPP::CDT    & oLocationConfig,
	                            ASObject   * & pContextData,
	                            ASLogger     & oLogger);

	/**
	  @brief Write HTTP response
	  @param oData - data model
	  @param oResponse - response object
	  @param pResponseWriter - response writer object
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oRequestPool - requeset data pool
	  @param oLocationConfig - handler configuration
	  @param pContextData - context data
	  @param oIMC - Inter-Module communication object
	  @param oLogger - application server logger
	  @return VIEW_OK - if success, VIEW_ERROR - if any error occured
	*/
	virtual INT_32 WriteResponse(CTPP::CDT                   & oData,
	                             ASResponse                  & oResponse,
	                             ASResponseWriter            & pResponseWriter,
	                             ASPool                      & oGlobalPool,
	                             ASPool                      & oVhostPool,
	                             ASPool                      & oRequestPool,
	                             CTPP::CDT                   & oLocationConfig,
	                             ASObject                    * pContextData,
	                             CTPP::CDT                   & oIMC,
	                             ASLogger                    & oLogger) = 0;

	/**
	  @brief Shutdown location hook
	  @param oGlobalPool - global pool of objects
	  @param oVhostPool - virtual host pool
	  @param oLocationConfig - handler configuration
	  @param pContextData - context data
	  @param oLogger - application server logger
	  @return HANDLER_OK - if success, HANDLER_ERROR - if any error occured
	*/
	virtual INT_32 ShutdownLocation(ASPool       & oGlobalPool,
	                                ASPool       & oVhostPool,
	                                CTPP::CDT    & oLocationConfig,
	                                ASObject   * & pContextData,
	                                ASLogger     & oLogger);

	/**
	  @brief A virtual destructor
	*/
	virtual ~ASView() throw();

private:
	/**
	  @brief Get object type
	  @return human-readable type of object
	*/
	CCHAR_P GetModuleType() const;
};

} // namespace CAS
#endif // _AS_VIEW_HPP__
// End.
