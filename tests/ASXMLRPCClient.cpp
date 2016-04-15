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
 *      ASLoaderTest.cpp
 *
 * $CAS$
 */

// C++ Includes
#include "ASXMLRPCClient.hpp"

#include <CTPP2JSONParser.hpp>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

using namespace CTPP; // CTPP
using namespace CAS; // C++ Application Server

int main(int argc, char ** argv)
{
	if (argc != 5 && argc != 6) { fprintf(stderr, "usage: %s Host Port [Server] Method JSON\n", argv[0]); return EX_USAGE; }
	INT_32 iReturnCode = EX_SOFTWARE;
	try
	{
		CDT oRequest;
		CDT oResponse;
		if(argc == 5)
		{
			CTPP2JSONParser(oRequest).Parse(argv[4], argv[4] + strlen(argv[4]));

			ASXMLRPCClient oRPCClient(argv[1], atoi(argv[2]), 10000, 10000);
			oRPCClient.Call(argv[3], oRequest, oResponse);
			fprintf(stderr, "%s\n%s\n%s\n", argv[3], oRequest.Dump().c_str(), oResponse.Dump().c_str());
		}
		else
		{
			CTPP2JSONParser(oRequest).Parse(argv[5], argv[5] + strlen(argv[5]));

			ASXMLRPCClient oRPCClient(argv[1], atoi(argv[2]), 1000, 1000, argv[3]);
			oRPCClient.Call(argv[4], oRequest, oResponse);
			fprintf(stderr, "%s\n%s\n%s\n", argv[4], oRequest.Dump().c_str(), oResponse.Dump().c_str());
		}

		iReturnCode = EX_OK;
	}
	catch(STLW::exception &e)
	{
		fprintf(stderr, "ERROR: %s\n", e.what());
	}

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

return iReturnCode;
}
// End.
