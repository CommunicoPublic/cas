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
 *      FastCGIWorkerContext.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_WORKER_CONTEXT_HPP__
#define _FAST_CGI_WORKER_CONTEXT_HPP__ 1

#include "FastCGIScoreboard.hpp"
#include "FastCGIWorkerConfig.hpp"
#include "STLList.hpp"
#include "STLMap.hpp"

namespace CAS // C++ Application Server
{
// FWD
class ASLogger;
class ServerContext;

/**
  @struct WorkerContext FastCGIWorkerContext.hpp <FastCGIWorkerContext.hpp>
  @brief FastCGI worker context
*/
struct WorkerContext
{
	/** List of free scoreboard slots  */
	typedef STLW::list<UINT_32>          TList;
	/** Process PID-to-scoreboard-slot */
	typedef STLW::map<UINT_32, UINT_32>  TProcMap;

	/** Worker configuration            */
	WorkerConfig       config;

	/** Socket type                    */
	INT_32             socket_type;
	/** Socket descriptor              */
	INT_32             socket;
	/** Read buffer size               */
	INT_32             read_buffer_size;

	/** Bytes read                     */
	UINT_64            bytes_read;
	/** Bytes written                  */
	UINT_64            bytes_written;

	/** PID                            */
	UINT_64            pid;

	/** Shutdown flag                  */
	bool               shutdown;

	/** Scoreboard position            */
	UINT_32            scoreboard_pos;
	/** Shared memory scoreboard
	                           object  */
	Scoreboard         scoreboard;

	/** Server context                 */
	ServerContext    * server_context;

	/** Error log                      */
	ASLogger         * error_log;
	/** Transfer log                   */
	ASLogger         * transfer_log;

	/**
	  @brief Constructor
	*/
	inline WorkerContext(const WorkerConfig & oWorkerConfig): config(oWorkerConfig),
	                                                          socket(-1),
	                                                          read_buffer_size(65536),
	                                                          bytes_read(0),
	                                                          bytes_written(0),
	                                                          pid(0),
	                                                          shutdown(false),
	                                                          scoreboard_pos(0),
	                                                          scoreboard(oWorkerConfig.max_clients),
	                                                          server_context(NULL),
	                                                          error_log(NULL),
	                                                          transfer_log(NULL)
	{
		;;
	}
};

} // namespace CAS
#endif // _FAST_CGI_WORKER_CONTEXT_HPP__
// End.

