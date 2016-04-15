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
 *      ASProfiler.hpp
 *
 * $CAS$
 */
#ifndef _AS_PROFILER_HPP__
#define _AS_PROFILER_HPP__ 1

/**
  @file ASProfiler.hpp
  @brief Application server profiler
*/
#include "ASTypes.hpp"
#include "STLString.hpp"
#include "STLVector.hpp"

namespace CAS // C++ Application Server
{

/**
  @class ASProfiler ASProfiler.hpp <ASProfiler.hpp>
  @brief Profiler class for C++ Application Server
*/
class ASProfiler
{
public:
	/**
	  @brief Constructor
	*/
	ASProfiler();

	/**
	  @brief Start point of event
	  @param szName - Event name
	  @return Event Id
	*/
	INT_32 StartEvent(CCHAR_P szName);

	/**
	  @brief End point of event
	  @param szName - Event name
	  @return Duration of event in seconds
	*/
	W_FLOAT EndEvent(CCHAR_P szName);

	/**
	  @brief Get duration of event in seconds
	  @param szName - Event name
	  @return Duration of event in seconds
	*/
	W_FLOAT GetDuration(CCHAR_P szName) const;

	/**
	  @brief Remove specified point
	  @param szName - Event name
	*/
	INT_32 RemoveEvent(CCHAR_P szName);

	/**
	  @brief reset profiler
	*/
	void ResetProfiler();

	/**
	  @brief Destructor
	*/
	~ASProfiler() throw();
private:
	// Does not exist
	ASProfiler(const ASProfiler  & oRhs);
	ASProfiler& operator=(const ASProfiler  & oRhs);

	/**
	  @struct Profiler::TimeRec SProfiler.hpp <ASProfiler.hpp>
	  @brief Event time record
	*/
	struct TimeRec
	{
		/** Start time            */
		struct timespec  start_time;
		/** End time              */
		struct timespec  end_time;
		/** Duration of event     */
		W_FLOAT          duration;
	};

	typedef STLW::map<STLW::string, TimeRec>  TEventMap;

	/** Events map      */
	TEventMap    mEvents;
};

} // namespace CAS
#endif // _AS_PROFILER_HPP__
// End.

