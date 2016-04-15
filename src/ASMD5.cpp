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
 *      ASUtil.cpp
 *
 * $CAS$
 */

#include "ASMD5.hpp"

namespace CAS // C++ Application Server
{

#ifdef AS_MD5_SUPPORT
//
// Constructor
//
MD5Hash::MD5Hash()
{
	MD5_Init(&oMD5Context);
}

//
// Update digest
//
MD5Hash & MD5Hash::Update(const STLW::string  & sData) { return Update(sData.data(), sData.size()); }

//
// Update digest
//
MD5Hash & MD5Hash::Update(CCHAR_P sData, const UINT_32 iLength)
{
	MD5_Update(&oMD5Context, (const unsigned char *)sData, iLength);

return *this;
}

//
// Update digest
//
MD5Hash & MD5Hash::Update(UCCHAR_P sData, const UINT_32 iLength)
{
	MD5_Update(&oMD5Context, sData, iLength);

return *this;
}

//
// Get Hex-encoded string
//
STLW::string MD5Hash::AsBin()
{
	unsigned char sDigest[16];
	MD5_Final(sDigest, &oMD5Context);

	MD5_Init(&oMD5Context);

return STLW::string((char *)sDigest, 16);
}

//
// Get MD5 digest as Hex-encoded string
//
STLW::string MD5Hash::AsHex(const bool  bUc)
{
	static CHAR_8  aHexL[] = "0123456789abcdef";
	static CHAR_8  aHexU[] = "0123456789ABCDEF";

	CHAR_8         szMD5[32];
	unsigned char  sDigest[16];

	MD5_Final(sDigest, &oMD5Context);

	if (bUc)
	{
		UINT_32 iJ = 0;
		for (UINT_32 iI = 0; iI < 16; ++iI)
		{
			szMD5[ iJ++] = aHexL[ (sDigest[iI] >> 4 )& 0x0F ];
			szMD5[ iJ++] = aHexL[  sDigest[iI]       & 0x0F ];
		}
	}
	else
	{
		UINT_32 iJ = 0;
		for (UINT_32 iI = 0; iI < 16; ++iI)
		{
			szMD5[ iJ++] = aHexU[ (sDigest[iI] >> 4 )& 0x0F ];
			szMD5[ iJ++] = aHexU[  sDigest[iI]       & 0x0F ];
		}
	}

	MD5_Init(&oMD5Context);

return STLW::string(szMD5, 32);
}

//
// Destructor
//
MD5Hash::~MD5Hash() throw()
{
	;;
}
#endif // AS_MD5_SUPPORT

} // namespace CAS
// End.
