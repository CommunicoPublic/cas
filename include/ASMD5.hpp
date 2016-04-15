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
 *      ASMD5.hpp
 *
 * $CAS$
 */
#ifndef _AS_MD5_HPP__
#define _AS_MD5_HPP__ 1

/**
  @file ASUtil.hpp
  @brief Various utilities & helpers
*/

#include "ASTypes.hpp"
#include "STLString.hpp"
#include "STLVector.hpp"

#if defined(AS_MD5_SUPPORT) && !defined(WIN32)
    #ifdef AS_MD5_WITHOUT_OPENSSL
        #include <md5.h>
        #define MD5_Init    MD5Init
        #define MD5_Update  MD5Update
        #define MD5_Final   MD5Final
    #else
        #include <openssl/md5.h>
    #endif
#endif // defined(AS_MD5_SUPPORT) && !defined(WIN32)

namespace CAS // C++ Application Server
{

#ifdef AS_MD5_SUPPORT
/**
  @class MD5Hash ASMD5.hpp <ASMD5.hpp>
  @brief MD5 hash support
*/
class MD5Hash
{
public:
	MD5Hash();

	/**
	  @brief Update digest
	  @param sData - source data, STL string
	  @return Reference to self
	*/
	MD5Hash & Update(const STLW::string  & sData);

	/**
	  @brief Update digest
	  @param sData - source data, pointer
	  @param iLength - source data length
	  @return Reference to self
	*/
	MD5Hash & Update(CCHAR_P sData, const UINT_32 iLength);

	/**
	  @brief Update digest
	  @param sData - source data, pointer
	  @param iLength - source data length
	  @return Reference to self
	*/
	MD5Hash & Update(UCCHAR_P sData, const UINT_32 iLength);

	/**
	  @brief Get Hex-encoded string
	  @param bUc - if set to true, use uppercase letters
	  @return Hex-encoded string
	*/
	STLW::string AsHex(const bool  bUc = false);

	/**
	  @brief Get BINARY string
	  @return BINARY string
	*/
	STLW::string AsBin();

	/**
	  @brief Destructor
	*/
	~MD5Hash() throw();
private:
	// Does not exist
	MD5Hash(const MD5Hash  & oRhs);
	MD5Hash & operator=(const MD5Hash  & oRhs);

	/** MD5Hash Context */
	MD5_CTX oMD5Context;
};
#endif // AS_MD5_SUPPORT

} // namespace CAS
#endif // _AS_MD5_HPP__
// End.
