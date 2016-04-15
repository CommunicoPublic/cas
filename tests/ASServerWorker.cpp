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
 *      ASServerWorker.cpp
 *
 * $CAS$
 */

// C Includes
#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>

// C++ Includes
#include "ASLogger.hpp"
#include "ASLoggerFile.hpp"
#include "ASRequest.hpp"
#include "ASResponse.hpp"
#include "ASResponseCodes.hpp"
#include "ASResponseWriter.hpp"
#include "ASServer.hpp"
#include "ASServerManager.hpp"

#include <CDT.hpp>
#include <CTPP2FileOutputCollector.hpp>
#include <CTPP2JSONFileParser.hpp>

#include <errno.h>
#include <string.h>
#include <unistd.h>

using namespace CAS; // C++ Application Server
using namespace CTPP;


class ASFileResponseWriter:
  public ASResponseWriter
{
public:
	ASFileResponseWriter(FILE * oIF);

	INT_32 WriteHeader(ASResponse & oResponse);

	/**
	  @brief Get output data collector
	  @return output data collector
	*/
	CTPP::OutputCollector * GetBodyOutputCollector();

	~ASFileResponseWriter() throw();
private:
	FILE                  * F;
	CTPP::OutputCollector * pOutputCollector;
};

ASFileResponseWriter::ASFileResponseWriter(FILE * oIF): F(oIF)
{
	pOutputCollector = new CTPP::FileOutputCollector(F);
}

INT_32 ASFileResponseWriter::WriteHeader(ASResponse & oResponse)
{
	// Write HTTP Response
	CCHAR_P szResponseCode = GetResponseCode(oResponse.GetHTTPResponseCode());

	fprintf(F, "HTTP/1.0 %s\r\n", szResponseCode);

	// Write content-type
	const STLW::string & sContentType = oResponse.GetContentType();
	if (!sContentType.empty())
	{
		fprintf(F, "Content-Type: ");
		fwrite(sContentType.data(), sContentType.size(), 1, F);
	}
	fwrite("\r\n", 2, 1, F);
	fprintf(F, "Connection: Close\r\n");
	fprintf(F, "X-Powered-By: C++ Application Server v%s(%s)\r\n", AS_VERSION, AS_IDENT);

	// Write headers
	STLW::vector<STLW::pair<STLW::string, STLW::string> > & vHeaders = oResponse.GetHeaders();
	STLW::vector<STLW::pair<STLW::string, STLW::string> >::const_iterator itvHeaders = vHeaders.begin();
	while (itvHeaders != vHeaders.end())
	{
		if (itvHeaders -> first.size() != 0 && itvHeaders -> second.size() != 0)
		{
			fwrite(itvHeaders -> first.data(),  itvHeaders -> first.size(),  1, F);
			fwrite(": ", 2, 1, F);
			fwrite(itvHeaders -> second.data(), itvHeaders -> second.size(), 1, F);
			fwrite("\r\n", 2, 1, F);
		}
		++itvHeaders;
	}

	// Write Cookies
	STLW::vector<ASCookie> & vCookies = oResponse.GetCookies();
	STLW::vector<ASCookie>::iterator itvCookies = vCookies.begin();
	while(itvCookies != vCookies.end())
	{
		fprintf(F, "Set-Cookie: %s\r\n", itvCookies -> GetCookie());
		++itvCookies;
	}

	fwrite("\r\n", 2, 1, F);
// All done
return 0;
}

CTPP::OutputCollector * ASFileResponseWriter::GetBodyOutputCollector() { return pOutputCollector; }

ASFileResponseWriter::~ASFileResponseWriter() throw()
{
	delete pOutputCollector;
}

int main(int argc, char ** argv)
{
	if (argc != 5) { fprintf(stderr, "usage: %s global-config.xml host.name host-config.xml request.json\n", argv[0]); return EX_USAGE; }

	INT_32 iReturnCode = -1;
	// Create logger
	ASLoggerFile oLogger(stderr);
	// Server manager
	ASServerManager * pServerManager = NULL;

	try
	{
		// Create server manager
		pServerManager = new ASServerManager(&oLogger);

		// Initialize manager
		if (pServerManager -> InitManager(argv[1], &oLogger) == -1)
		{
			fprintf(stderr, "ERROR: Cannot init server manager with configuration \"%s\"\n", argv[1]);
			pServerManager -> ShutdownManager(&oLogger);
			delete pServerManager;
			return EX_SOFTWARE;
		}

		// Initialize virtual server
		if (pServerManager -> InitServer(argv[2], argv[3], &oLogger) == -1)
		{
			fprintf(stderr, "ERROR: Cannot init server \"%s\" with configuration \"%s\"\n", argv[2], argv[3]);
			pServerManager -> ShutdownManager(&oLogger);
			delete pServerManager;
			return EX_SOFTWARE;
		}

		// Get server
		ASServer * pASServer = pServerManager -> GetServer(argv[2], &oLogger);
		if (pASServer == NULL)
		{
			fprintf(stderr, "ERROR: Cannot get ASServer object for host %s\n", argv[2]);
		}
		else
		{
			// Response writer
			ASFileResponseWriter oFileResponseWriter(stdout);

			// Create JSON parser
			CDT oRequestData;
			CTPP2JSONFileParser(oRequestData).Parse(argv[4]);

			// Get Request URI
			STLW::string oRequestURI = oRequestData["LocationURI"].GetString();

			// Request object
			ASRequest  oRequest;
			STLW::string sRequestMethod = oRequestData["RequestMethod"].GetString();
			if      (strcasecmp("GET",  sRequestMethod.c_str()) == 0) { oRequest.request_method = ASRequest::GET; }
			else if (strcasecmp("POST", sRequestMethod.c_str()) == 0) { oRequest.request_method = ASRequest::POST; }

			oRequest.uri       = oRequestURI;
			oRequest.host      = oRequestData["Host"].GetString();
			oRequest.port      = oRequestData["Port"].GetInt();
			oRequest.remote_ip = oRequestData["RemoteIP"].GetString();
			oRequest.headers   = oRequestData["Headers"];
			oRequest.cookies   = oRequestData["Cookies"];
			oRequest.arguments = oRequestData["Arguments"];

			// Check location
			INT_32 iLocationId = pASServer -> CheckLocationURI(oRequestURI.c_str(), oRequest, &oLogger);
			if (iLocationId == -1)
			{
				fprintf(stderr, "ERROR: Unhandled location %s\n", oRequestURI.c_str());
			}
			// Handle request
			else
			{
				ASHostConfig * pConfig = pASServer -> GetConfig();
				oRequest.location_name = pConfig -> locations[iLocationId].name;

				ASServer::ASRequestContext oContext(iLocationId, pConfig -> locations[iLocationId]);
				if (pASServer -> HandleRequest(oContext, oRequest, &oFileResponseWriter, &oLogger) == -1)
				{
					fprintf(stderr, "ERROR: Fatal error in HandleRequest(location %s)\n", oRequestURI.c_str());
				}
			}
		}
		iReturnCode = 0;
		pServerManager -> ShutdownManager(&oLogger);
		delete pServerManager;
	}
	catch(std::exception & e) { fprintf(stderr, "ERROR: %s\n", e.what()); }
	catch(...)                { fprintf(stderr, "ERROR: Ouch!\n");        }

	if (iReturnCode != 0)
	{
		if (pServerManager != NULL)
		{
			pServerManager -> ShutdownManager(&oLogger);
			delete pServerManager;
		}
		fclose(stdin);
		fclose(stdout);
		fclose(stderr);

		return EX_SOFTWARE;
	}

	/* Resources usage */
	struct rusage sRUsage;
	if (getrusage(RUSAGE_SELF, &sRUsage) == 0)
	{
		fprintf(stderr, "mod_cas_handler: rusage(utime %f, stime %f, rss: %lu, text: %lu, data: %lu, stack: %lu, pgrec: %lu, faults: %lu, swaps: %lu, bklin: %lu, blkout: %lu, msgsnd: %lu, msgrciv: %lu, nsig: %lu, volsw: %lu, involsw: %lu\n",
		               (1.0 * sRUsage.ru_utime.tv_sec + 1.0 * sRUsage.ru_utime.tv_usec / 1000000),
		               (1.0 * sRUsage.ru_stime.tv_sec + 1.0 * sRUsage.ru_stime.tv_usec / 1000000),
		               sRUsage.ru_maxrss,
		               sRUsage.ru_ixrss,
		               sRUsage.ru_idrss,
		               sRUsage.ru_isrss,
		               sRUsage.ru_minflt,
		               sRUsage.ru_majflt,
		               sRUsage.ru_nswap,
		               sRUsage.ru_inblock,
		               sRUsage.ru_oublock,
		               sRUsage.ru_msgsnd,
		               sRUsage.ru_msgrcv,
		               sRUsage.ru_nsignals,
		               sRUsage.ru_nvcsw,
		               sRUsage.ru_nivcsw);
	}

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

return EX_OK;
}
// End.
