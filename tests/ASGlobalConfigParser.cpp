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
 *      ASGlobalConfigParser.cpp
 *
 * $CAS$
 */

// C++ Includes
#include <ASGlobalConfigHandler.hpp>
#include <ASGlobalConfig.hpp>
#include <ASUtil.hpp>
#include <ASXMLParser.hpp>

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace CAS; // C++ Application Server
using namespace CTPP;

void DumpData(UINT_32 iLevel, const CDT & oData, std::string &sResult);

std::string Dump(const CDT & oData)
{
	 std::string sResult;
	 DumpData(2, oData, sResult);
return sResult;
}

//
// Dump to string
//
void DumpData(UINT_32 iLevel, const CDT & oData, std::string &sResult)
{
	CHAR_8 szBuf[512 + 1];

	++iLevel;
	switch (oData.GetType())
	{
		case CDT::UNDEF:
		case CDT::INT_VAL:
		case CDT::REAL_VAL:
		case CDT::STRING_VAL:
		case CDT::POINTER_VAL:
			sResult += "\"" + oData.GetString() + "\"\n";
			break;

		case CDT::ARRAY_VAL:
			{
				sResult += "\n";
				for (UINT_32 iI = 0; iI < oData.Size(); ++iI)
				{
					for (UINT_32 iJ = 0; iJ < iLevel; iJ++) { sResult += "      "; }
					snprintf(szBuf, 512, "%u", iI);
					sResult += szBuf;
					sResult += " : ";
					DumpData(iLevel, oData.GetCDT(iI), sResult);
				}
			}
			break;

		case CDT::HASH_VAL:
			{
				sResult += "\n";
				CDT::ConstIterator itHash = oData.Begin();
				while (itHash != oData.End())
				{
					for (UINT_32 iJ = 0; iJ < iLevel; iJ++) { sResult += "      "; }
					sResult += itHash -> first;
					sResult += " => ";
					DumpData(iLevel, itHash -> second, sResult);
					++itHash;
				}
			}
			break;

		default:
			sResult += "Invalid type";
	}
}

int main(int argc, char ** argv)
{
	const char * szConfigFile = NULL;
	if (argc == 1)
	{
		fprintf(stderr, "Global config not given, ");
		szConfigFile = getenv("CAS_GLOBAL_CONFIG");
		if (szConfigFile != NULL)
		{
			fprintf(stderr, " using %s from ENVIRONMENT", szConfigFile);
		}
		else
		{
			szConfigFile = CAS_GLOBAL_CONFIG_FILE;
			fprintf(stderr, " using %s as DEFAULT\n", szConfigFile);
		}
	}
	else if (argc == 2) { szConfigFile = argv[1]; }
	else { fprintf(stderr, "usage: %s [global-config.xml]\n", argv[0]); return EX_USAGE; }

	FILE * F = fopen(szConfigFile, "rb");
	if (F == NULL) { fprintf(stderr, "ERROR: Cannot open `%s` for reading: %s\n", szConfigFile, strerror(errno)); return EX_SOFTWARE; }

	// Store path to file as include directory
	CCHAR_P szTMP = szConfigFile + strlen(szConfigFile);
	while (szTMP != szConfigFile && *szTMP != '/' && *szTMP != '\\') { --szTMP; }

	STLW::vector<STLW::string> vIncludeDirs;
	if (szTMP != szConfigFile) { vIncludeDirs.push_back(STLW::string(szConfigFile, (szTMP - szConfigFile))); }

	try
	{
		ASGlobalConfig oGlobalConfig;
		ASGlobalConfigHandler oHandler(oGlobalConfig, vIncludeDirs);
		ASXMLParser oParser(&oHandler);
		if (oParser.ParseFile(F) == -1)
		{
			fprintf(stderr, "ERROR: In file %s: %s\n", szConfigFile, oHandler.GetError().c_str());
			return EX_CONFIG;
		}
		fclose(F);

		fprintf(stdout, "  Libexec dirs:\n");
		UINT_32 iI = 0;
		for(; iI < oGlobalConfig.libexec_dirs.size(); ++iI)
		{
			fprintf(stdout, "      %s\n", oGlobalConfig.libexec_dirs[iI].c_str());
		}
		fprintf(stdout, "\n  Modules:\n");
		for(iI = 0; iI < oGlobalConfig.modules_list.size(); ++iI)
		{

			fprintf(stdout, "      Name:         %s\n"
			                "      Type:         %s\n",
			                                            oGlobalConfig.modules_list[iI].name.c_str(),
			                                            oGlobalConfig.modules_list[iI].moduletype.c_str());

			STLW::string sTMP = CheckFile(oGlobalConfig.libexec_dirs, oGlobalConfig.modules_list[iI].library);
			if (sTMP.size() == 0)
			{
				fprintf(stdout, "      *** ERROR: Cannot find Library file: %s\n", oGlobalConfig.modules_list[iI].library.c_str());
			}
			else
			{
				fprintf(stdout, "      Library file: %s\n", CheckFile(oGlobalConfig.libexec_dirs, oGlobalConfig.modules_list[iI].library).c_str());
			}
			STLW::string sData = Dump(oGlobalConfig.modules_list[iI].configuration);
			if (!sData.empty() && sData != "\"\"\n") { fprintf(stdout, "      Configuration: %s\n", sData.c_str()); }
			// TBD
			fprintf(stdout, "\n");
		}
	}
        catch(STLW::exception &e) { fprintf(stderr, "ERROR: %s\n", e.what()); return EX_SOFTWARE; }
        catch(...)                { fprintf(stderr, "ERROR: Ouch!\n"); return EX_SOFTWARE; }

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

return EX_OK;
}
// End.
