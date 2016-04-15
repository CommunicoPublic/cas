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
 *      ASHostConfig.hpp
 *
 * $CAS$
 */
#ifndef _AS_HOST_CONFIG_HPP__
#define _AS_HOST_CONFIG_HPP__ 1

/**
  @file ASHostConfig.hpp
  @brief Application server global configuration object
*/
#include "ASLocation.hpp"

namespace CAS // C++ Application Server
{

/**
  @struct ASModuleConfig ASHostConfig.hpp <ASHostConfig.hpp>
  @brief Application server module configuration object
*/
struct ASModuleConfig
{
	/** Module name                  */
	STLW::string    module_name;
	/** Module configuration         */
	CTPP::CDT       module_config;
};

/**
  @struct ASHostConfig ASHostConfig.hpp <ASHostConfig.hpp>
  @brief Application server host configuration object
*/
struct ASHostConfig
{
	/** Host name                         */
	STLW::string                    host_name;
	/** Server root directory             */
	STLW::string                    server_root;
	/** Directory for temporary files     */
	STLW::string                    tempfile_dir;
	/** List of template directories      */
	STLW::vector<STLW::string>      template_dirs;
	/** List of locations                 */
	STLW::vector<ASLocation>        locations;
	/** Per-server modules configuration  */
	STLW::vector<ASModuleConfig>    modules_config;
};

} // namespace CAS
#endif // _AS_HOST_CONFIG_HPP__
// End.
