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
 *      ASSAPIUtil.cpp
 *
 * $CAS$
 */
#ifndef _AS_SAPI_UTIL_HPP__
#define _AS_SAPI_UTIL_HPP__ 1

// C++ Includes
#include "ASSAPIUtil.hpp"
#include "STLString.hpp"
#include "ASTypes.hpp"

/**
  @fn void ParseCookies(CCHAR_P  szString, CTPP::CDT  & oRetVal);
  @brief Parse cookies "foo=bar; baz=bar+baz/boo"
  @param szString - string to parse
  @param oRetVal - return value
*/
void ParseCookies(CCHAR_P      szString,
                  CTPP::CDT  & oRetVal);


/**
  @fn void StorePair(CTPP::CDT & oRetVal, STLW::string  & sKey, STLW::string  & sValue);
  @brief Store pair of key => value into CDT object
  @param oRetVal - return value
  @param sKey - key to store
  @param sValue - value to store
*/
void StorePair(CTPP::CDT     & oRetVal,
               STLW::string  & sKey,
               STLW::string  & sValue);

#endif // _AS_SAPI_UTIL_HPP__
// End.
