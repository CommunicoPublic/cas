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
 *      FastFGIWorkerSignalHandler.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_WORKER_SIGNAL_HANDLER_HPP__
#define _FAST_CGI_WORKER_SIGNAL_HANDLER_HPP__ 1

#include "FastCGISignalHandler.hpp"

namespace CAS // C++ Application Server
{
// FWD
class WorkerContext;

/**
  @class WorkerSignalHandler FastFGIWorkerSignalHandler.hpp <FastFGIWorkerSignalHandler.hpp>
  @brief Worker process signal handler
*/
class WorkerSignalHandler:
  public UserSignalHandler
{
public:
	/**
	  @brief A Constructor
	  @param oWorkerContext - global context
	*/
	WorkerSignalHandler(WorkerContext  & oIWorkerContext);

	/**
	  @brief A destructor
	*/
	~WorkerSignalHandler() throw();

private:
	// FWD
	WorkerSignalHandler(const WorkerSignalHandler  & oRhs);
	WorkerSignalHandler& operator=(const WorkerSignalHandler  & oRhs);

	/**
	  @brief Handle signal
	  @param iSignal - signal
	*/
	void Handler(const INT_32 iSignal);

	/** Worker context */
	WorkerContext  & oWorkerContext;
};

} // namespace CAS
#endif //_FAST_CGI_WORKER_SIGNAL_HANDLER_HPP__
// End.
