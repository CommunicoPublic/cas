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
 *      FastCGIMainProcess.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_MAIN_PROCESS_HPP__
#define _FAST_CGI_MAIN_PROCESS_HPP__ 1

#include "FastCGIWorkerContext.hpp"
#include "FastCGIMainSignalHandler.hpp"

namespace CAS // C++ Application Server
{
/**
  class MainProcess FastCGIWorkerContext.hpp <FastCGIWorkerContext.hpp>
  @brief FastCGI worker process
*/
class MainProcess
{
public:
	/**
	  @brief Constructor
	*/
	MainProcess(const WorkerConfig  & oWorkerConfig);

	/**
	  @brief Setup main process
	*/
	INT_32 Setup();

	/**
	  @brief Handle FastCGI child processes
	*/
	INT_32 Run();

	/**
	  @brief A destructor
	*/
	~MainProcess() throw();
private:
	// Does not exist
	MainProcess(const MainProcess & oRhs);
	MainProcess& operator=(const MainProcess & oRhs);

	/** List of free scoreboard contextes */
	WorkerContext::TList       oFreeList;
	/** List of used scoreboard contextes */
	WorkerContext::TList       oUsedList;
	/** Process-to-scoreboard map         */
	WorkerContext::TProcMap    mProcMap;
	/** Worker context                    */
	WorkerContext              oWorkerContext;
	/** Main signal handler               */
	MainSignalHandler          oMainSignalHandler;
	/** Server namager flag               */
	bool                       bServerManagerStarted;

	/**
	  @brief Spawn child process
	*/
	INT_32 SpawnChild();

	/**
	  @brief Child function
	*/
	INT_32 ChildFn(const UINT_32 iScoreboardPos);

	/**
	 @brief Wait for childs
	*/
	void Wait();

	/**
	  @brief Lock socket
	*/
	INT_32 Lock();

	/**
	  @brief Lock socket
	*/
	INT_32 Unlock();
};

} // namespace CAS
#endif // _FAST_CGI_MAIN_PROCESS_HPP__
// End.
