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

#include "ASProfiler.hpp"

#include <sys/time.h>
#include <time.h>

namespace CAS // C++ Application Server
{

//
// Constructor
//
ASProfiler::ASProfiler()
{
	;;
}

//
// Start point of event
//
INT_32 ASProfiler::StartEvent(CCHAR_P szName)
{
	TEventMap::iterator itmEvents = mEvents.find(szName);
	if (itmEvents != mEvents.end())
	{
		clock_gettime(CLOCK_REALTIME, &itmEvents -> second.start_time);
		return 1;
	}

	TimeRec oTimeRec;
	oTimeRec.duration = 0.0;
	clock_gettime(CLOCK_REALTIME, &oTimeRec.start_time);
	oTimeRec.end_time = oTimeRec.start_time;

	mEvents[szName] = oTimeRec;

return 0;
}

//
// End point of event
//
W_FLOAT ASProfiler::EndEvent(CCHAR_P szName)
{
	TEventMap::iterator itmEvents = mEvents.find(szName);
	// No such event
	if (itmEvents == mEvents.end()) { return -1.0; }

	TimeRec & oTimeRec = itmEvents -> second;

	clock_gettime(CLOCK_REALTIME, &oTimeRec.end_time);

	oTimeRec.duration += (oTimeRec.end_time.tv_sec +   1.0E-9 * oTimeRec.end_time.tv_nsec) -
	                     (oTimeRec.start_time.tv_sec + 1.0E-9 * oTimeRec.start_time.tv_nsec);

return oTimeRec.duration;
}

//
// Get duration of event
//
W_FLOAT ASProfiler::GetDuration(CCHAR_P szName) const
{
	TEventMap::const_iterator itmEvents = mEvents.find(szName);
	// No such event
	if (itmEvents == mEvents.end()) { return -1.0; }

// Get duration
return itmEvents -> second.duration;
}

//
// Clear specified event
//
INT_32 ASProfiler::RemoveEvent(CCHAR_P szName)
{
	TEventMap::iterator itmEvents = mEvents.find(szName);
	// No such event
	if (itmEvents == mEvents.end()) { return -1; }

	mEvents.erase(itmEvents);

return 0;
}

//
// Reset profiler
//
void ASProfiler::ResetProfiler()
{
	TEventMap oTMPMap;
	mEvents.swap(oTMPMap);
}

//
// Destructor
//
ASProfiler::~ASProfiler() throw()
{
	;;
}

} // namespace CAS
// End.
