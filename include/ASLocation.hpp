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
 *      ASLocation.hpp
 *
 * $CAS$
 */
#ifndef _AS_LOCATION_HPP__
#define _AS_LOCATION_HPP__ 1

/**
  @file ASLocation.hpp
  @brief Application server location configuration
*/
#include "ASHandlerConfig.hpp"
#include "ASPool.hpp"
#include "ASPreRequestHandlerConfig.hpp"
#include "ASViewConfig.hpp"

namespace CAS // C++ Application Server
{
// FWD
class ASLocationURI;

typedef STLW::map<STLW::string, ASViewConfig> ASViewMap;

/**
  @class ASHandler ASHandler.hpp <ASHandler.hpp>
  @brief Application server handler
*/
struct ASLocation
{
	/** Location name                         */
	STLW::string                            name;
	/** List of allowed HTTP methods          */
	UINT_32                                 allowed_methods;
	/** Max. size of POST request             */
	INT_64                                  max_post_size;
	/** Max. file size                        */
	INT_64                                  max_file_size;
	/** Default HTTP response code            */
	UINT_32                                 default_response_code;
	/** Default HTTP content type             */
	STLW::string                            default_content_type;
	/** URL list                              */
	STLW::vector<ASLocationURI *>           uri_list;
	/** List of request hooks                 */
	STLW::vector<ASPreRequestHandlerConfig> pre_request_handlers;
	/** Controller                            */
	ASHandlerConfig                         controller;
	/** List of handlers                      */
	STLW::vector<ASHandlerConfig>           handlers;
	/** List of finalizers                    */
	STLW::vector<ASHandlerConfig>           finalizers;
	/** View                                  */
	ASViewMap                               views;

	/**
	  @brief Clear  ASLocation object
	  @param oLocation - object to clear
	*/
	static void ClearLocation(ASLocation & oLocation);
};

} // namespace CAS
#endif // _AS_LOCATION_HPP__
// End.
