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
 *      ASUtil.hpp
 *
 * $CAS$
 */
#ifndef _AS_UTIL_HPP__
#define _AS_UTIL_HPP__ 1

/**
  @file ASUtil.hpp
  @brief Various utilities & helpers
*/

#include "ASTypes.hpp"
#include "STLString.hpp"
#include "STLVector.hpp"

namespace CAS // C++ Application Server
{

// FWD
class ASLogger;

/**
  @fn STLW::string CheckFile(const STLW::vector<STLW::string> & vDirList, const STLW::string & sFileName, ASLogger * pLogger)
  @brief Find file in include directories
  @param vDirList - directories list
  @param sFileName - file to find
  @param pLogger - logger object
  @return File with full path
*/
STLW::string CheckFile(const STLW::vector<STLW::string>  & vDirList,
                       const STLW::string                & sFileName,
                       ASLogger                          * pLogger);

/**
  @fn STLW::string CheckFile(const STLW::vector<STLW::string> & vDirList, const STLW::string & sFileName)
  @brief Find file in include directories
  @param vDirList - directories list
  @param sFileName - file to find
  @return File with full path
*/
STLW::string CheckFile(const STLW::vector<STLW::string>  & vDirList,
                       const STLW::string                & sFileName);

/**
  @fn CCHAR_P StrCaseStr(CCHAR_P sX, CCHAR_P sY);
  @brief This function locates the first occurrence of the null-terminated string
  @param sX - string where need find substring
  @param sY - substring to find
  @return If sY is an empty string, sX is returned; if sY occurs nowhere
          in sX, NULL is returned; otherwise a pointer to the first character of
          the first occurrence of sY is returned.
*/
CCHAR_P StrCaseStr(CCHAR_P sX, CCHAR_P sY);

/**
  @fn STLW::string UnescapeURI(const STLW::string  & sSource);
  @brief This function unescapes URI-escaped string
  @param sSource - string to unescape
  @return Unescaped string
*/
STLW::string UnescapeURI(const STLW::string  & sSource);

#ifdef AS_MD5_SUPPORT
/**
  @fn STLW::string MD5Hex(const STLW::string & sSource)
  @brief Calculate MD5 sum for given string
  @brief sSource - source string
  @return MD5(sSource) digest in hexadecimal form
*/
STLW::string MD5Hex(const STLW::string & sSource);

/**
  @fn STLW::string MD5Bin(const STLW::string & sSource)
  @brief Calculate MD5 sum for given string
  @brief sSource - source string
  @return MD5(sSource) digest in hexadecimal form
*/
STLW::string MD5Bin(const STLW::string & sSource);

#endif // AS_MD5_SUPPORT

} // namespace CAS
#endif // _AS_UTIL_HPP__
// End.
