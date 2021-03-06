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
 *      ASViewConfig.hpp
 *
 * $CAS$
 */
#ifndef _AS_VIEW_CONFIG_HPP__
#define _AS_VIEW_CONFIG_HPP__ 1

/**
  @file ASViewConfig.hpp
  @brief Application server view configuration
*/
#include "ASTypes.hpp"
#include "STLString.hpp"

namespace CAS // C++ Application Server
{
class ASView;

/**
  @struct ASViewConfig ASViewConfig.hpp <ASViewConfig.hpp>
  @brief Application server view configuration
*/
struct ASViewConfig
{
	/** View name                   */
	STLW::string      name;
	/** Location handler            */
	ASView          * object;
	/** Handler configuration       */
	CTPP::CDT         view_config;
	/** Handler context data        */
	ASObject        * context_data;
};

} // namespace CAS
#endif // _AS_VIEW_CONFIG_HPP__
// End.
