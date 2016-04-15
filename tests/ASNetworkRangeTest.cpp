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
 *      ASNetworkRangeTest.cpp
 *
 * $CAS$
 */

/**
  @file ASNetworkRangeTest.hpp
  @brief Test suite for network ranges
*/

#include "ASNetworkRange.hpp"

#include <stdio.h>

using namespace CAS; // C++ Application Server
using namespace CTPP;

int main(int argc, char ** argv)
{
	NetworkRange<INT_32> oNetworkRange;

	INT_32 iRC = 0;

	iRC = oNetworkRange.AddNetwork("127/8",            1);
	fprintf(stderr, "127/8 %d\n", iRC);

	iRC = oNetworkRange.AddNetwork("127.0/8",          1);
	fprintf(stderr, "127.0/8 %d\n", iRC);

	iRC = oNetworkRange.AddNetwork("10.0/16",          2);
	fprintf(stderr, "10.0/16 %d\n", iRC);

	iRC = oNetworkRange.AddNetwork("172.16.0/12",      3);
	fprintf(stderr, "172.16.0/12 = %d\n", iRC);

	iRC = oNetworkRange.AddNetwork("192.168.1.0/24",   4);
	fprintf(stderr, "192.168.1.0/24 = %d\n", iRC);

	iRC = oNetworkRange.AddNetwork("12.12.12.12/32",   5);
	fprintf(stderr, "12.12.12.12/32 = %d\n", iRC);

	INT_32 iFound = 0;
	iRC = oNetworkRange.SearchAddress("127.0.0.2", iFound);
	fprintf(stderr, "iRC = %d iFound = %d\n", iRC, iFound);

	iRC = oNetworkRange.SearchAddress("10.0.0.8", iFound);
	fprintf(stderr, "iRC = %d iFound = %d\n", iRC, iFound);

	iRC = oNetworkRange.SearchAddress("192.168.1.192", iFound);
	fprintf(stderr, "iRC = %d iFound = %d\n", iRC, iFound);

	iRC = oNetworkRange.SearchAddress("12.12.12.1", iFound);
	fprintf(stderr, "iRC = %d iFound = %d\n", iRC, iFound);

return EX_OK;
}
// End.
