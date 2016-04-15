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
#include <ASXMLRPCHandler.hpp>
#include <ASXMLParser.hpp>

#include <errno.h>
#include <string.h>
#include <unistd.h>

using namespace CAS; // C++ Application Server
using namespace CTPP;

int main(int argc, char ** argv)
{
	if (argc == 1) { fprintf(stderr, "usage: %s host-config.xml\n", argv[0]); return EX_USAGE; }

	FILE * F = fopen(argv[1], "rb");
	if (F == NULL) { fprintf(stderr, "ERROR: Cannot open `%s` for reading: %s\n", argv[1], strerror(errno)); return EX_SOFTWARE; }

	try
	{
		CDT oXMLRPC;
		ASXMLRPCHandler oHandler(oXMLRPC);
		ASXMLParser oParser(&oHandler);
		if (oParser.ParseFile(F) == -1)
		{
			fprintf(stderr, "ERROR: In file %s: %s\n", argv[1], oHandler.GetError().c_str());
			return EX_CONFIG;
		}
		fclose(F);
		fprintf(stderr, "Result: %s\n", oXMLRPC.RecursiveDump().c_str());
	}
	catch(STLW::exception &e) { fprintf(stderr, "ERROR: %s\n", e.what()); return EX_SOFTWARE; }
	catch(...)                { fprintf(stderr, "ERROR: Ouch!\n"); return EX_SOFTWARE; }

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

return EX_OK;
}
// End.
