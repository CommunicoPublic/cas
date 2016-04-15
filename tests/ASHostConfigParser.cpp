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
 *      ASHostConfigParser.cpp
 *
 * $CAS$
 */

// C++ Includes
#include <ASHostConfigHandler.hpp>
#include <ASHostConfig.hpp>
#include <ASLocation.hpp>
#include <ASLocationURI.hpp>
#include <ASRequest.hpp>
#include <ASUtil.hpp>
#include <ASXMLParser.hpp>

#include <errno.h>
#include <string.h>
#include <unistd.h>

using namespace CAS; // C++ Application Server
using namespace CTPP;

static STLW::string GetAllowedMethods(const UINT_32 & iMethods)
{
	STLW::string sMethods;

	if (iMethods == 0) { return "None"; }

	if ((iMethods & ASRequest::GET)     != 0) { sMethods.append("GET, ");     }
	if ((iMethods & ASRequest::PUT)     != 0) { sMethods.append("PUT, ");     }
	if ((iMethods & ASRequest::POST)    != 0) { sMethods.append("POST, ");    }
	if ((iMethods & ASRequest::DELETE)  != 0) { sMethods.append("DELETE, ");  }
	if ((iMethods & ASRequest::CONNECT) != 0) { sMethods.append("CONNECT, "); }
	if ((iMethods & ASRequest::OPTIONS) != 0) { sMethods.append("OPTIONS, "); }
	if ((iMethods & ASRequest::TRACE)   != 0) { sMethods.append("TRACE, ");   }

	sMethods.erase(sMethods.size() - 2);
return sMethods;
}

static void DumpData(UINT_32 iLevel, const CDT & oData, std::string &sResult);

static void Dump(const CDT & oData)
{
	std::string sResult;
	DumpData(2, oData, sResult);

	if (!sResult.empty() && sResult != "\"\"\n" && sResult != "\n") { fprintf(stdout, "          Configuration: `%s`", sResult.c_str()); }
}

//
// Dump to string
//
static void DumpData(UINT_32 iLevel, const CDT & oData, std::string &sResult)
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
	if (argc == 1) { fprintf(stderr, "usage: %s host-config.xml\n", argv[0]); return EX_USAGE; }

	FILE * F = fopen(argv[1], "rb");
	if (F == NULL) { fprintf(stderr, "ERROR: Cannot open `%s` for reading: %s\n", argv[1], strerror(errno)); return EX_SOFTWARE; }

	try
	{
		ASHostConfig oHostConfig;
		ASHostConfigHandler oHandler(oHostConfig);
		ASXMLParser oParser(&oHandler);
		if (oParser.ParseFile(F) == -1)
		{
			fprintf(stderr, "ERROR: In file %s: %s\n", argv[1], oHandler.GetError().c_str());
			return EX_CONFIG;
		}
		fclose(F);

		fprintf(stdout, "  Server name:           %s\n", oHostConfig.host_name.c_str());
		fprintf(stdout, "  Server root:           %s\n", oHostConfig.server_root.c_str());
		fprintf(stdout, "  Temp. files directory: %s\n", oHostConfig.tempfile_dir.c_str());

		fprintf(stdout, "  Template include directories:\n");
		UINT_32 iI = 0;
		for(; iI < oHostConfig.template_dirs.size(); ++iI)
		{
			fprintf(stdout, "      %s\n", oHostConfig.template_dirs[iI].c_str());
		}

		fprintf(stdout, "\n  Per-server modules configuration:\n");
		if (oHostConfig.modules_config.size() == 0)
		{
			fprintf(stdout, "    No per-server configuration: -\n");
		}
		else
		{
			STLW::vector<ASModuleConfig>::const_iterator itmModulesConfig = oHostConfig.modules_config.begin();
			while (itmModulesConfig != oHostConfig.modules_config.end())
			{
				fprintf(stdout, "    Module: %s\n", itmModulesConfig -> module_name.c_str());
				Dump(itmModulesConfig -> module_config);
				++itmModulesConfig;
			}
		}

		fprintf(stdout, "\n  Locations:\n");
		for(iI = 0; iI < oHostConfig.locations.size(); ++iI)
		{

			fprintf(stdout, "      Name: %s\n", oHostConfig.locations[iI].name.c_str());
			if (oHostConfig.locations[iI].allowed_methods == 0xFFFFFFFF) { fprintf(stdout, "      Allowed methods:            All\n"); }
			else
			{
				fprintf(stdout, "      Allowed methods:            %s\n", GetAllowedMethods(oHostConfig.locations[iI].allowed_methods).c_str());
			}

			if (oHostConfig.locations[iI].max_post_size != -1)
			{
				fprintf(stdout, "      Max. POST request size:     %lld\n", (long long)oHostConfig.locations[iI].max_post_size);
			}
			else
			{
				fprintf(stdout, "      Max. POST request size:     unlimited\n");
			}

			if (oHostConfig.locations[iI].max_file_size != -1)
			{
				fprintf(stdout, "      Max. size of uploaded file: %lld\n", (long long)oHostConfig.locations[iI].max_file_size);
			}
			else
			{
				fprintf(stdout, "      Max. size of uploaded file: unlimited\n");
			}

			fprintf(stdout, "      Default HTTP Response code: %d\n", oHostConfig.locations[iI].default_response_code);
			fprintf(stdout, "      Default HTTP content type:  %s\n", oHostConfig.locations[iI].default_content_type.c_str());
			fprintf(stdout, "      URI(s):\n");
			UINT_32 iJ = 0;
			for (; iJ < oHostConfig.locations[iI].uri_list.size(); ++iJ)
			{
				ASLocationURI * pLocationURI = oHostConfig.locations[iI].uri_list[iJ];
				fprintf(stdout, "          %s (%s)\n", pLocationURI -> GetURI().c_str(), pLocationURI -> GetURIType() == ASLocationURI::PLAIN_URI ? "Plain": "Regexp");
			}

			if (!oHostConfig.locations[iI].controller.name.empty())
			{
				fprintf(stdout, "\n      Controller: %s\n", oHostConfig.locations[iI].controller.name.c_str());
				Dump(oHostConfig.locations[iI].controller.handler_config);
			}

			fprintf(stdout, "\n      Handlers(s):\n");

			for (iJ = 0; iJ < oHostConfig.locations[iI].handlers.size(); ++iJ)
			{
				ASHandlerConfig & oHandlerConfig = oHostConfig.locations[iI].handlers[iJ];
				fprintf(stdout, "          %s\n", oHandlerConfig.name.c_str());
				Dump(oHostConfig.locations[iI].handlers[iJ].handler_config);
			}

			if (oHostConfig.locations[iI].finalizers.size() != 0)
			{
				fprintf(stdout, "\n      Finalizers(s):\n");
				for (iJ = 0; iJ < oHostConfig.locations[iI].finalizers.size(); ++iJ)
				{
					ASHandlerConfig & oHandlerConfig = oHostConfig.locations[iI].finalizers[iJ];
					fprintf(stdout, "          %s\n", oHandlerConfig.name.c_str());
					Dump(oHostConfig.locations[iI].finalizers[iJ].handler_config);
				}
			}

			if (oHostConfig.locations[iI].views.empty())
			{
				fprintf(stdout, "\n      ERROR: View section does not present\n\n");
				return EX_CONFIG;
			}
			else
			{
				fprintf(stdout, "\n      View:\n");
				ASViewMap::const_iterator itmViews = oHostConfig.locations[iI].views.begin();
				while (itmViews != oHostConfig.locations[iI].views.end())
				{
					fprintf(stdout, "              %s\n", itmViews -> first.c_str());
					Dump(itmViews -> second.view_config);

					++itmViews;
				}
			}

			fprintf(stdout, "\n\n");
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
