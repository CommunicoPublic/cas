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
 *      FastCGIWorkerConfig.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_WORKER_CONFIG_HPP__
#define _FAST_CGI_WORKER_CONFIG_HPP__ 1

#include "ASNetworkRange.hpp"
#include "STLString.hpp"

#ifndef C_MAX_CLIENTS
#define C_MAX_CLIENTS 100
#endif // C_MAX_CLIENTS

#ifndef C_LISTEN_QUEUE
#define C_LISTEN_QUEUE 128
#endif // C_LISTEN_QUEUE

#ifndef C_IO_TIMEOUT
#define C_IO_TIMEOUT 10000
#endif // C_IO_TIMEOUT

#ifndef C_MAX_REQUESTS
#define C_MAX_REQUESTS -1
#endif // C_MAX_REQUESTS

namespace CAS // C++ Application Server
{
/**
  @struct WorkerConfig FastCGIWorkerConfig.hpp <FastCGIWorkerConfig.hpp>
  @brief FastCGI worker config
*/
struct WorkerConfig
{
	/** Process name                   */
	STLW::string           procname;
	/** Host name                      */
	STLW::string           hostname;
	/** Global config file             */
	STLW::string           global_config_file;
	/** Host config file               */
	STLW::string           host_config_file;
	/** Server status uri              */
	STLW::string           status_uri;
	/** Real IP address                */
	STLW::string           realip_header;
	/** Pid file                       */
	STLW::string           pid_file;
	/** Debug mode                     */
	bool                   debug;
	/** Run in foreground              */
	bool                   foreground;
	/** Host                           */
	STLW::string           host;
	/** Port                           */
	INT_32                 port;
	/** Unix socket path               */
	STLW::string           path;
	/** Unix socket mode               */
	INT_32                 mode;
	/** Listen queue size              */
	INT_32                 listen;
	/** I/O timeout                    */
	INT_32                 io_timeout;
	/** Mx. number of clients          */
	INT_32                 max_clients;
	/** Min. number of free workers    */
	INT_32                 min_free_workers;
	/** Max. number of free workers    */
	INT_32                 max_free_workers;
	/** Max. number of requests        */
	INT_32                 max_requests;

	/** User                           */
	STLW::string           user;
	/** Grou                           */
	STLW::string           group;
	/** UID                            */
	INT_32                 uid;
	/** GID                            */
	INT_32                 gid;
	/** Additional groups              */
	STLW::vector<INT_32>   gids;

	/** Allowed for connection networks    */
	NetworkRange<UINT_32>  allowed_nets;
	/** Allowed networks for server-status */
	NetworkRange<UINT_32>  allowed_status_nets;

	/**
	  @brief Constructor
	*/
	inline WorkerConfig(): realip_header("Remote-Addr"),
	                       debug(false),
	                       foreground(false),
	                       port(0),
	                       mode(0),
	                       listen(C_LISTEN_QUEUE),
	                       io_timeout(C_IO_TIMEOUT),
	                       max_clients(C_MAX_CLIENTS),
	                       min_free_workers(0),
	                       max_free_workers(0),
	                       max_requests(C_MAX_REQUESTS),
	                       uid(0),
	                       gid(0)
	{
		;;
	}
};

} // namespace CAS
#endif // _FAST_CGI_WORKER_CONFIG_HPP__
// End.

