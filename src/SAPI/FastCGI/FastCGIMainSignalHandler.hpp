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
 *      FastFGIMainSignalHandler.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_MAIN_SIGNAL_HANDLER_HPP__
#define _FAST_CGI_MAIN_SIGNAL_HANDLER_HPP__ 1

#include "FastCGISignalHandler.hpp"
#include "FastCGIWorkerContext.hpp"

namespace CAS // C++ Application Server
{
/**
  @class MainSignalHandler FastFGIMainSignalHandler.hpp <FastFGIMainSignalHandler.hpp>
  @brief Main process signal handler
*/
class MainSignalHandler:
  public UserSignalHandler
{
public:
	/**
	  @brief A Constructor
	  @param oIWorkerContext - worker context
	  @param mIProcMap - Process PID-to-scoreboard-slot map
	*/
	MainSignalHandler(WorkerContext            & oIWorkerContext,
	                  WorkerContext::TProcMap  & mIProcMap);

	/**
	  @brief A destructor
	*/
	~MainSignalHandler() throw();

private:
	// FWD
	MainSignalHandler(const MainSignalHandler  & oRhs);
	MainSignalHandler& operator=(const MainSignalHandler  & oRhs);

	/**
	  @brief Handle signal
	  @param iSignal - signal
	*/
	void Handler(const INT_32 iSignal);

	/** Worker context                     */
	WorkerContext            & oWorkerContext;
	/** Process PID-to-scoreboard-slot map */
	WorkerContext::TProcMap  & mProcMap;
};

} // namespace CAS
#endif //_FAST_CGI_MAIN_SIGNAL_HANDLER_HPP__
// End.
