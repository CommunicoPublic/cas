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
 *      ASProfiler.cpp
 *
 * $CAS$
 */

/**
  @file ASProfiler.hpp
  @brief Test suite for profiler class
*/

#include "ASProfiler.hpp"

#include <unistd.h>
#include <stdio.h>

using namespace CAS; // C++ Application Server
using namespace CTPP;

int main(int argc, char ** argv)
{
	ASProfiler oProfiler;

	// Sleep 1 sec
	oProfiler.StartEvent("Sleep 1 sec");
	sleep(1);
	W_FLOAT dTime = oProfiler.EndEvent("Sleep 1 sec");
	fprintf(stderr, "Sleep 1 sec...: %f sec\n", dTime);

	// Sleep 1 musec
	oProfiler.StartEvent("Sleep 1 musec");
	usleep(10000);
	dTime = oProfiler.EndEvent("Sleep 1 musec");
	fprintf(stderr, "Sleep 1 musec.: %f sec\n", dTime);

	fprintf(stderr, "2 events: %f, %f\n", oProfiler.GetDuration("Sleep 1 sec"), oProfiler.GetDuration("Sleep 1 musec"));

return EX_OK;
}
// End.
