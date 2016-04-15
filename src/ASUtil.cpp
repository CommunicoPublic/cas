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

#include "ASUtil.hpp"

#include "ASDebugHelper.hpp"
#include "ASLogger.hpp"

#include <sys/param.h>

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

#ifdef _MSC_VER
        #define random()        rand()
        #define INT_64(x)       (INT_64)(x)
        #define srandomdev()    srand( (unsigned)time(NULL) );
#endif

#include <fcntl.h>
#include <unistd.h>

namespace CAS // C++ Application Server
{

//
// Find file in include directories
//
STLW::string CheckFile(const STLW::vector<STLW::string>  & vDirList,
                       const STLW::string                & sFileName,
                       ASLogger                          * pLogger)
{
	DEBUG_HELPER(pLogger, STLW::string("CheckFile \"") + sFileName + "\"");

	if (sFileName.size() == 0) { DEBUG_HELPER_MESSAGE("Missed file name"); return ""; }

	if (sFileName[0] == '/')
	{
		if (access(sFileName.c_str(), R_OK) == 0)
		{
			DEBUG_HELPER_MESSAGE("Found here: \"%s\"", sFileName.c_str());
			return sFileName;
		}
		DEBUG_HELPER_MESSAGE("File \"%s\" not found", sFileName.c_str());
		return "";
	}

	CHAR_P szCurrentDir = (CHAR_P)malloc(MAXPATHLEN + 1);
	if (getcwd(szCurrentDir, MAXPATHLEN) == NULL) { return ""; }

	STLW::vector<STLW::string>::const_iterator itvDirList = vDirList.begin();
	while (itvDirList != vDirList.end())
	{
		STLW::string sTMP = *itvDirList;
		if (sTMP.size() != 0)
		{
			if (sTMP[0] != '/') { sTMP = STLW::string(szCurrentDir) + "/" + sTMP; }
			if (sTMP[sTMP.size() - 1] != '/') { sTMP.append("/"); }

			sTMP.append(sFileName);

			if (access(sTMP.c_str(), R_OK) == 0)
			{
				free(szCurrentDir);
				DEBUG_HELPER_MESSAGE("Found here: \"%s\"", sTMP.c_str());
				return sTMP;
			}
		}
		++itvDirList;
	}
	free(szCurrentDir);

	if (vDirList.size() == 0)
	{
		STLW::string sSearchPath("File \"");
		sSearchPath += sFileName + "\" not found; empty search path.";
		DEBUG_HELPER_MESSAGE(sSearchPath.c_str());
		pLogger -> WriteLog(AS_LOG_CRIT, sSearchPath.c_str());
	}

	STLW::string sSearchPath("File \"");
	sSearchPath +=  sFileName + "\" not found; search path: \"";

	itvDirList = vDirList.begin();
	while (itvDirList != vDirList.end())
	{
		sSearchPath += *itvDirList;
		++itvDirList;
		if (itvDirList != vDirList.end()) { sSearchPath += "\", \""; }
	}
	sSearchPath += "\"";
	DEBUG_HELPER_MESSAGE(sSearchPath.c_str());
	pLogger -> WriteLog(AS_LOG_CRIT, sSearchPath.c_str());

return "";
}

//
// Find file in include directories
//
STLW::string CheckFile(const STLW::vector<STLW::string>  & vDirList,
                       const STLW::string                & sFileName)
{
	if (sFileName.size() == 0) { return ""; }

	if (sFileName[0] == '/')
	{
		if (access(sFileName.c_str(), R_OK) == 0) { return sFileName; }
		return "";
	}

	CHAR_P szCurrentDir = (CHAR_P)malloc(MAXPATHLEN + 1);
	if(getcwd(szCurrentDir, MAXPATHLEN) == NULL) { return ""; }

	STLW::vector<STLW::string>::const_iterator itvDirList = vDirList.begin();
	while (itvDirList != vDirList.end())
	{
		STLW::string sTMP = *itvDirList;
		if (sTMP.size() != 0)
		{
			if (sTMP[0] != '/') { sTMP = STLW::string(szCurrentDir) + "/" + sTMP; }
			if (sTMP[sTMP.size() - 1] != '/') { sTMP.append("/"); }

			sTMP.append(sFileName);

			if (access(sTMP.c_str(), R_OK) == 0)
			{
				free(szCurrentDir);
				return sTMP;
			}
		}
		++itvDirList;
	}
	free(szCurrentDir);

return "";
}

//
// Find first occurense in string
//
CCHAR_P StrCaseStr(CCHAR_P sX, CCHAR_P sY)
{
	while (tolower(*sX) == tolower(*sY))
	{
		++sY; ++sX;
		if (*sY == '\0') { return sX; }
	}
return NULL;
}

#define C_UNESCAPE_BUFFER_LEN 128

STLW::string UnescapeURI(const STLW::string  & sSource)
{
	STLW::string  sResult;

	CHAR_8 sBuffer[C_UNESCAPE_BUFFER_LEN + 3];
	UINT_32 iBufferPointer = 0;

	STLW::string::const_iterator itsSource          = sSource.begin();
	const STLW::string::const_iterator itsSourceEnd = sSource.end();

	// Iterate through buffer
	while (itsSource != itsSourceEnd)
	{
		// Buffer overflow
		if (iBufferPointer == C_UNESCAPE_BUFFER_LEN)
		{
			sResult.append(sBuffer, iBufferPointer);
			iBufferPointer = 0;
		}

		UCHAR_8 ucSymbol = *itsSource;
		// Change '+' to space
		if      (ucSymbol == '+') { sBuffer[iBufferPointer++] = ' '; }
		// Store all unescaped symbols
		else if (ucSymbol != '%') { sBuffer[iBufferPointer++] = ucSymbol; }
		else
		{
			UCHAR_8 ucTMP = 0;

			++itsSource;
			if (itsSource == itsSourceEnd) { break; }

			ucSymbol = *itsSource;
			// Unescape correct sequence
			if      (ucSymbol >= 'A' && ucSymbol <= 'F') { ucTMP = ((ucSymbol - 'A' + 10) << 4); }
			else if (ucSymbol >= 'a' && ucSymbol <= 'f') { ucTMP = ((ucSymbol - 'a' + 10) << 4); }
			else if (ucSymbol >= '0' && ucSymbol <= '9') { ucTMP =  (ucSymbol - '0')      << 4;  }
			// Store '%' symbol to the buffer
			else
			{
				sBuffer[iBufferPointer++] = '%';
				continue;
			}

			++itsSource;
			if (itsSource == itsSourceEnd) { break; }

			ucSymbol = *itsSource;
			// Unescape correct sequence
			if      (ucSymbol >= 'A' && ucSymbol <= 'F') { ucTMP |= (ucSymbol - 'A' + 10); }
			else if (ucSymbol >= 'a' && ucSymbol <= 'f') { ucTMP |= (ucSymbol - 'a' + 10); }
			else if (ucSymbol >= '0' && ucSymbol <= '9') { ucTMP |= (ucSymbol - '0');      }
			// Store '%' symbol to the buffer
			else
			{
				sBuffer[iBufferPointer++] = '%';
				sBuffer[iBufferPointer++] = ucSymbol;
				continue;
			}

			// Okay, symbol successfully unescaped
			sBuffer[iBufferPointer++] = ucTMP;
		}

		++itsSource;
	}

	// Append buffer to result
	sResult.append(sBuffer, iBufferPointer);

return sResult;
}
#ifdef AS_MD5_SUPPORT
//
// Calculate MD5 sum for given string
//
STLW::string MD5Hex(const STLW::string & sSource)
{
	static CHAR_8 aHex[] = "0123456789abcdef";
	CHAR_8 szMD5[32];

	MD5_CTX oMD5Context;
	MD5_Init(&oMD5Context);

	MD5_Update(&oMD5Context, (const unsigned char *)sSource.c_str(), sSource.size());

	unsigned char sDigest[16];
	MD5_Final(sDigest, &oMD5Context);

	UINT_32 iJ = 0;
	for (UINT_32 iI = 0; iI < 16; ++iI)
	{
		szMD5[ iJ++] = aHex[ (sDigest[iI] >> 4 )& 0x0F ];
		szMD5[ iJ++] = aHex[  sDigest[iI]       & 0x0F ];
	}

return STLW::string(szMD5, 32);
}

//
// Calculate MD5 sum for given string
//
STLW::string MD5Bin(const STLW::string & sSource)
{
	MD5_CTX oMD5Context;
	MD5_Init(&oMD5Context);

	MD5_Update(&oMD5Context, (const unsigned char *)sSource.c_str(), sSource.size());

	unsigned char sDigest[16];
	MD5_Final(sDigest, &oMD5Context);

return STLW::string((CHAR_P)sDigest, 16);
}
#endif // AS_MD5_SUPPORT

} // namespace CAS
// End.
