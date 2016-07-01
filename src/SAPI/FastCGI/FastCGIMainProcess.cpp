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
 *      FastCGIMainProcess.cpp
 *
 * $CAS$
 */
#include "ASDebugHelper.hpp"
#include "ASLogCodes.hpp"
#include "ASServerManager.hpp"
#include "FastCGILoggerSyslog.hpp"
#include "FastCGIMainProcess.hpp"
#include "FastCGISetProcTitle.h"
#include "FastCGIServerContext.hpp"
#include "FastCGIWorkerSignalHandler.hpp"
#include "FastCGIWorker.hpp"

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <netinet/tcp.h>
#include <netinet/in.h>

#include <errno.h>
#include <fcntl.h>
#include <sysexits.h>
#include <unistd.h>

namespace CAS // C++ Application Server
{
//
//  Constructor
//
MainProcess::MainProcess(const WorkerConfig  & oWorkerConfig): oWorkerContext(oWorkerConfig),
                                                               oMainSignalHandler(oWorkerContext, mProcMap),
                                                               bServerManagerStarted(false)
{
	if (oWorkerConfig.debug)
	{
		oWorkerContext.error_log    = new LoggerSyslog(stderr, false);
		oWorkerContext.transfer_log = new LoggerSyslog(stderr, true);

		oWorkerContext.error_log -> SetPriority(AS_LOG_DEBUG);
	}
	else
	{
		oWorkerContext.error_log    = new LoggerSyslog(NULL, false);
		oWorkerContext.transfer_log = new LoggerSyslog(NULL, true);

		oWorkerContext.error_log -> SetPriority(AS_LOG_WARN);
	}

	oWorkerContext.transfer_log -> SetPriority(AS_LOG_DEBUG);

	// Create server context
	oWorkerContext.server_context = new ServerContext;
	// Create server manager
	oWorkerContext.server_context -> server_manager = new ASServerManager(oWorkerContext.error_log);
}

//
// Setup main process
//
INT_32 MainProcess::Setup()
{
	ASLogger & oErrorLog = *oWorkerContext.error_log;
	DEBUG_HELPER(&oErrorLog, STLW::string("MainProcess::Setup: \"") + oWorkerContext.config.global_config_file + "\"");

	oErrorLog.Info("Global configuration: %s", oWorkerContext.config.global_config_file.c_str());
	oErrorLog.Info("Host configuration: %s", oWorkerContext.config.host_config_file.c_str());

	CHAR_8 szProcTitle[1024];
	snprintf(szProcTitle, 1024, "cas-fcgid: main process %s %s",
	                            oWorkerContext.config.global_config_file.c_str(),
	                            oWorkerContext.config.host_config_file.c_str());
	_setproctitle(szProcTitle);

	DEBUG_HELPER_MESSAGE("Register signal handlers");
	SignalHandler & oSigHandler = SignalHandler::Instance();
	oSigHandler.Clear();

	oSigHandler.RegisterHandler(SIGINT,  &oMainSignalHandler);
	oSigHandler.RegisterHandler(SIGTERM, &oMainSignalHandler);
	oSigHandler.RegisterHandler(SIGUSR1, &oMainSignalHandler);
	oSigHandler.RegisterHandler(SIGPIPE, &oMainSignalHandler);

	DEBUG_HELPER_MESSAGE("Create & open IPC scoreboard segment");
	// Open IPC scoreboard
	if (oWorkerContext.scoreboard.Open(oWorkerContext.config.procname.c_str()) == -1)
	{
		oErrorLog.Emerg("Can't create shared memory scoreboard segment");
		return -1;
	}

	if      (!oWorkerContext.config.path.empty())
	{
		DEBUG_HELPER_MESSAGE("Create & open unix socket");

		// Socket type
		oWorkerContext.socket_type = PF_UNIX;
		// Socket
		oWorkerContext.socket = socket(PF_UNIX, SOCK_STREAM, 0);
		if (oWorkerContext.socket == -1) { return -1; }

		struct sockaddr_un oSockAddr;
		memset(&oSockAddr, 0, sizeof(struct sockaddr_un));

		oSockAddr.sun_family = PF_UNIX;

		strncpy(oSockAddr.sun_path, oWorkerContext.config.host.c_str(), sizeof(oSockAddr.sun_path));
		unlink(oSockAddr.sun_path);
		chmod(oWorkerContext.config.host.c_str(), oWorkerContext.config.mode);

		int iTMP = 1;
		setsockopt(oWorkerContext.socket, SOL_SOCKET, SO_REUSEADDR, &iTMP, sizeof(int));
		iTMP = 1;
		setsockopt(oWorkerContext.socket, SOL_SOCKET, SO_REUSEPORT, &iTMP, sizeof(int));

		// Bind to interface
		if (bind(oWorkerContext.socket, (struct sockaddr *)(&oSockAddr), sizeof(oSockAddr)) == -1)
		{
			close(oWorkerContext.socket);
			return -1;
		}

		oErrorLog.Info("Listening on %s:%04o", oWorkerContext.config.host.c_str(), oWorkerContext.config.mode);
	}
	else if (!oWorkerContext.config.host.empty())
	{
		if (oWorkerContext.config.host[0] == '[')
		{
			DEBUG_HELPER_MESSAGE("Create & open IPv6 socket");

			// Socket type
			oWorkerContext.socket_type = PF_INET6;
			// Socket
			oWorkerContext.socket = socket(PF_INET6, SOCK_STREAM, 0);
			if (oWorkerContext.socket == -1) { return -1; }

			struct sockaddr_in6 oSockAddr;
			memset(&oSockAddr, 0, sizeof(struct sockaddr_in6));

			oSockAddr.sin6_family = PF_INET6;
			oSockAddr.sin6_port   = htons(oWorkerContext.config.port);

			if (oWorkerContext.config.host == "[*]") { oSockAddr.sin6_addr = in6addr_any; }
			else
			{
				if (inet_pton(PF_INET6, oWorkerContext.config.host.c_str(), &oSockAddr.sin6_addr) == 0)
				{
					close(oWorkerContext.socket);
					return -1;
				}
			}

			int iTMP = 1;
			setsockopt(oWorkerContext.socket, SOL_SOCKET, SO_REUSEADDR, &iTMP, sizeof(int));
			iTMP = 1;
			setsockopt(oWorkerContext.socket, SOL_SOCKET, SO_REUSEPORT, &iTMP, sizeof(int));

			// Bind to interface
			if (bind(oWorkerContext.socket, (struct sockaddr *)(&oSockAddr), sizeof(oSockAddr)) == -1)
			{
				close(oWorkerContext.socket);
				return -1;
			}

			oErrorLog.Info("Listening on IPv6 [%s]:%u", oWorkerContext.config.host.c_str(), oWorkerContext.config.port);
		}
		else
		{
			DEBUG_HELPER_MESSAGE("Create & open IPv4 socket");

			// Socket type
			oWorkerContext.socket_type = PF_INET;
			// Socket
			oWorkerContext.socket = socket(PF_INET, SOCK_STREAM, 0);
			if (oWorkerContext.socket == -1) { return -1; }

			struct sockaddr_in oSockAddr;
			memset(&oSockAddr, 0, sizeof(struct sockaddr_in));

			oSockAddr.sin_family = PF_INET;
			oSockAddr.sin_port   = htons(oWorkerContext.config.port);

			if (oWorkerContext.config.host == "*") { oSockAddr.sin_addr.s_addr = htonl(INADDR_ANY); }
			else
			{
				if (inet_aton(oWorkerContext.config.host.c_str(), &oSockAddr.sin_addr) == 0)
				{
					close(oWorkerContext.socket);
					return -1;
				}
			}

			int iTMP = 1;
			setsockopt(oWorkerContext.socket, SOL_SOCKET, SO_REUSEADDR, &iTMP, sizeof(int));
			iTMP = 1;
			setsockopt(oWorkerContext.socket, SOL_SOCKET, SO_REUSEPORT, &iTMP, sizeof(int));

			// Bind to interface
			if (bind(oWorkerContext.socket, (struct sockaddr *)(&oSockAddr), sizeof(oSockAddr)) == -1)
			{
				close(oWorkerContext.socket);
				return -1;
			}
			oErrorLog.Info("Listening on IPv4 %s:%u", oWorkerContext.config.host.c_str(), oWorkerContext.config.port);
		}
	}
	else
	{
		DEBUG_HELPER_MESSAGE("No socket info given");
		oErrorLog.Emerg("No socket info given");
		return -1;
	}

	DEBUG_HELPER_MESSAGE("Set socket options");

	DEBUG_HELPER_MESSAGE("Initialize server manager");
	if (oWorkerContext.server_context -> server_manager -> InitManager(oWorkerContext.config.global_config_file.c_str(), &oErrorLog) == -1)
	{
		DEBUG_HELPER_MESSAGE("Error in InitManager");
		return -1;
	}
	bServerManagerStarted = true;

	DEBUG_HELPER_MESSAGE("Initialize server");
	// Return NULL if any error occured
	if (oWorkerContext.server_context -> server_manager -> InitServer(oWorkerContext.config.hostname.c_str(), oWorkerContext.config.host_config_file.c_str(), &oErrorLog) == -1)
	{
		DEBUG_HELPER_MESSAGE("Error in InitServer");
		return -1;
	}

	DEBUG_HELPER_MESSAGE("Getting server");
	oWorkerContext.server_context -> server = oWorkerContext.server_context -> server_manager -> GetServer(oWorkerContext.config.hostname.c_str(), &oErrorLog);
	if(oWorkerContext.server_context -> server == NULL)
	{
		DEBUG_HELPER_MESSAGE("Error in GetServer");
		return -1;
	}

	DEBUG_HELPER_MESSAGE("Setup completed");

return 0;
}

//
// Run process
//
INT_32 MainProcess::Run()
{
	ASLogger & oErrorLog = *oWorkerContext.error_log;
	DEBUG_HELPER(&oErrorLog, "MainProcess::Run");

	DEBUG_HELPER_MESSAGE("Clearing scoreboard");
	// Clear data
	for(INT_32 iPos = 0; iPos < oWorkerContext.config.max_clients; ++iPos)
	{
		Scoreboard::ScoreboardRec oRec;
		oWorkerContext.scoreboard.WriteScoreboard(iPos, oRec);
		oFreeList.push_back(iPos);
	}

	if (oWorkerContext.config.foreground && oWorkerContext.config.debug)
	{
		CCHAR_P szDebugMode = "DEBUG MODE: Running ONLY ONE HANDLER IN DEBUG MODE";
		DEBUG_HELPER_MESSAGE(szDebugMode);
		oErrorLog.Emerg(szDebugMode);
		fprintf(stderr, "%s\n", szDebugMode);

		INT_32 iRC = ChildFn(0);

		DEBUG_HELPER_MESSAGE("Exiting");
		close(oWorkerContext.socket);
	return iRC;
	}

	DEBUG_HELPER_MESSAGE("Creating workers");
	oErrorLog.Info("Spawning %u worker processes", oWorkerContext.config.min_free_workers);
	// Create list of processes
	for(INT_32 iPos = 0; iPos < oWorkerContext.config.min_free_workers; ++iPos)
	{
		SpawnChild();
	}

	DEBUG_HELPER_MESSAGE("Manage processes");
	for(;;)
	{
		// Sleep some time
		sleep(1);

		// Wait for child process(es)
		Wait();
		if (oWorkerContext.shutdown) { break; }

		STLW::string sStatInfo;
		// Get number of free processes
		INT_32 iFreeProcesses = 0;
		WorkerContext::TProcMap::const_iterator itmProcMap = mProcMap.begin();
		while (itmProcMap != mProcMap.end())
		{
			const UINT_32 iScoreboardPos = itmProcMap -> second;

			Scoreboard::ScoreboardRec oRec;
			oWorkerContext.scoreboard.ReadScoreboard(iScoreboardPos, oRec);
			if (oRec.state == Scoreboard::ScoreboardRec::FREE)
			{
				// Shutdown transactionally
				if (iFreeProcesses > oWorkerContext.config.max_free_workers)
				{
					DEBUG_HELPER_MESSAGE("Shutting down worker");

					const UINT_32 iPid = itmProcMap -> first;
					oErrorLog.Info("Shutting down worker %u", iPid);

					kill(iPid, SIGTERM);
				}
				++iFreeProcesses;
			}

			CHAR_8 szStatInfo[1024];
			snprintf(szStatInfo, 1023, " [pid: %u, %s, req: %llu, r: %llu, w: %llu]",
			                           oRec.pid, oRec.GetState(), (long long unsigned)oRec.requests, (long long unsigned)oRec.bytes_read, (long long unsigned)oRec.bytes_written);
			sStatInfo.append(szStatInfo);
			++itmProcMap;
		}
		oErrorLog.Debug("Procs:%s", sStatInfo.c_str());

		const INT_32 iProcNum = oWorkerContext.config.min_free_workers - iFreeProcesses;
		if (iProcNum > 0)
		{
			oErrorLog.Info("Spawning %u worker processes", iProcNum);
			// Spawn processes, if need
			for(INT_32 iProc = 0; iProc < iProcNum; ++iProc) { SpawnChild(); }
		}
	}

	DEBUG_HELPER_MESSAGE("Exiting");
	close(oWorkerContext.socket);

return 0;
}

//
// Spawn child process
//
INT_32 MainProcess::SpawnChild()
{
	ASLogger & oTransferLog = *oWorkerContext.transfer_log;
	ASLogger & oErrorLog    = *oWorkerContext.error_log;
	DEBUG_HELPER(&oErrorLog, "MainProcess::SpawnChild");

	if (oFreeList.empty())
	{
		oErrorLog.Error("Max. clients reached; can't create new process", oWorkerContext.config.max_clients);
		return -1;
	}

	const UINT_32 iScoreboardPos = *oFreeList.rbegin();

	const pid_t iPid = fork();
	if (iPid == 0)
	{
		dynamic_cast<LoggerSyslog &>(oTransferLog).Reopen();
		dynamic_cast<LoggerSyslog &>(oErrorLog).Reopen();

		DEBUG_HELPER_MESSAGE("Worker process");

		SignalHandler & oSigHandler = SignalHandler::Instance();
		oSigHandler.Clear();

		WorkerSignalHandler oWorkerSignalHandler(oWorkerContext);
		oSigHandler.RegisterHandler(SIGINT,  &oWorkerSignalHandler);
		oSigHandler.RegisterHandler(SIGTERM, &oWorkerSignalHandler);
		oSigHandler.RegisterHandler(SIGUSR1, &oWorkerSignalHandler);
		oSigHandler.RegisterHandler(SIGPIPE, &oWorkerSignalHandler);

		// Close on exec
		fcntl(oWorkerContext.socket, F_SETFL, oWorkerContext.socket | FD_CLOEXEC);

		// Store PID
		oWorkerContext.pid            = getpid();
		// Store scoreboard position
		oWorkerContext.scoreboard_pos = iScoreboardPos;

		oErrorLog.Info("Worker %u; scoreboard %u started", oWorkerContext.pid, oWorkerContext.scoreboard_pos);

		// Execute child function
		const INT_32 iRC = ChildFn(iScoreboardPos);

		// Close server socket
		close(oWorkerContext.socket);

		// Ok
		if (iRC == EX_OK) { oErrorLog.Info("Worker %u terminated normally", oWorkerContext.pid); }
		// Error
		else
		{
			// Prevent fast restarting
			sleep(5);
			oErrorLog.Error("Child %u terminated with error %d", oWorkerContext.pid, iRC);
		}

		// Shutdown subsystem
		oWorkerContext.server_context -> server_manager -> ShutdownManager(&oErrorLog);

		DEBUG_HELPER_MESSAGE("Worker process terminated");

		delete oWorkerContext.server_context -> server_manager;
		delete oWorkerContext.server_context;

		delete oWorkerContext.error_log;
		delete oWorkerContext.transfer_log;

		oWorkerContext.scoreboard.Close();

		exit(iRC);
		// Make compiler happy, never reach here
		return iRC;
	}

	if (iPid == -1) { return -1; }

	DEBUG_HELPER_MESSAGE("Worker process registered in pool");

	mProcMap[iPid] = iScoreboardPos;
	oFreeList.pop_back();
	oUsedList.push_back(iScoreboardPos);

return 0;
}

//
// Child function
//
INT_32 MainProcess::ChildFn(const UINT_32 iScoreboardPos)
{
	ASLogger & oErrorLog    = *oWorkerContext.error_log;
	ASLogger & oTransferLog = *oWorkerContext.transfer_log;
	DEBUG_HELPER(&oErrorLog, "MainProcess::ChildFn");

	if (listen(oWorkerContext.socket, oWorkerContext.config.listen) == -1)
	{
		oErrorLog.Emerg("Can't listen on socket");
		return EX_SOFTWARE;
	}

	Scoreboard::ScoreboardRec oRec;
	oRec.pid = oWorkerContext.pid;
	oWorkerContext.scoreboard.WriteScoreboard(iScoreboardPos, oRec);

	WorkerProcess oWorkerProcess(oWorkerContext);
	for(;;)
	{
		DEBUG_HELPER_MESSAGE("Worker process");

		CHAR_8 szProcTitle[1024];
		snprintf(szProcTitle, 1024, "cas-fcgid: worker process");
		_setproctitle(szProcTitle);

		oWorkerContext.scoreboard.ReadScoreboard(iScoreboardPos, oRec);
		oRec.uri[0] = '\0';
		oRec.state  = Scoreboard::ScoreboardRec::FREE;
		oWorkerContext.scoreboard.WriteScoreboard(iScoreboardPos, oRec);

		CHAR_8 szClientIP[129];
		INT_32 iClientPort   = 0;
		INT_32 iClientSocket = 0;

		if (Lock() == -1)
		{
			if (oWorkerContext.shutdown) { break; }

			oErrorLog.Emerg("Can't lock listen socket: %s", strerror(errno));
			return EX_SOFTWARE;
		}

		if (oWorkerContext.shutdown) { break; }

		oWorkerContext.scoreboard.ReadScoreboard(iScoreboardPos, oRec);
		oRec.state = Scoreboard::ScoreboardRec::ACCEPT;
		oWorkerContext.scoreboard.WriteScoreboard(iScoreboardPos, oRec);

		DEBUG_HELPER_MESSAGE("Accepting connection");
		switch(oWorkerContext.socket_type)
		{
			case PF_UNIX:
				{
					struct sockaddr_un oSockAddr;
					socklen_t          iSockAddrLen = sizeof(oSockAddr);
					memset(&oSockAddr, 0, iSockAddrLen);
					iClientSocket = accept(oWorkerContext.socket, (struct sockaddr *)&oSockAddr, &iSockAddrLen);

					if (iClientSocket != -1)
					{
						snprintf(szClientIP, 128, "%s", "unix");
						oErrorLog.Info("Worker %u: client %s connected", oWorkerContext.pid, szClientIP);
					}
				}
				break;
			case PF_INET:
				{
					struct sockaddr_in oSockAddr;
					socklen_t          iSockAddrLen = sizeof(oSockAddr);
					memset(&oSockAddr, 0, iSockAddrLen);
					iClientSocket = accept(oWorkerContext.socket, (struct sockaddr *)&oSockAddr, &iSockAddrLen);

					if (iClientSocket != -1 && inet_ntop(PF_INET, &oSockAddr.sin_addr, szClientIP, 16) != NULL)
					{
						iClientPort = ntohs(oSockAddr.sin_port);
						oErrorLog.Info("Worker %u: client %s:%u connected", oWorkerContext.pid, szClientIP, iClientPort);
					}
				}
				break;
			case PF_INET6:
				{
					struct sockaddr_in6 oSockAddr;
					socklen_t           iSockAddrLen = sizeof(oSockAddr);
					memset(&oSockAddr, 0, iSockAddrLen);
					iClientSocket = accept(oWorkerContext.socket, (struct sockaddr *)&oSockAddr, &iSockAddrLen);

					if (iClientSocket != -1 && inet_ntop(PF_INET6, &oSockAddr.sin6_addr, szClientIP, 64) != NULL)
					{
						iClientPort = ntohs(oSockAddr.sin6_port);
						oErrorLog.Info("Worker %u: client [%s]:%u connected", oWorkerContext.pid, szClientIP, iClientPort);
					}

				}
				break;
		}

		// Shutdown server
		if (oWorkerContext.shutdown)
		{
			close(iClientSocket);
			break;
		}

		if (Unlock() == -1)
		{
			// Close socket
			close(iClientSocket);
			oErrorLog.Emerg("Can't unlock listen socket: %s", strerror(errno));
			return EX_SOFTWARE;
		}

		if (iClientSocket == -1)
		{
			DEBUG_HELPER_MESSAGE("Error in accept");
			oErrorLog.Error("Can't accept connection: %s", strerror(errno));

			return EX_SOFTWARE;
		}
		// Do useful work
		else
		{
			DEBUG_HELPER_MESSAGE("Connection accepted");

			int iTMP = 1;
			setsockopt(iClientSocket, SOL_SOCKET, SO_REUSEADDR, &iTMP, sizeof(int));
			iTMP = 1;
			setsockopt(iClientSocket, SOL_SOCKET, SO_REUSEPORT, &iTMP, sizeof(int));
			iTMP = 1;
			setsockopt(iClientSocket, SOL_SOCKET, SO_KEEPALIVE, &iTMP, sizeof(int));
			iTMP = 1;
			setsockopt(iClientSocket, IPPROTO_TCP, TCP_NODELAY, &iTMP, sizeof(int));

			fcntl(iClientSocket, F_SETFL, fcntl(iClientSocket, F_GETFL) | O_NONBLOCK);

			dynamic_cast<LoggerSyslog &>(oTransferLog).SetProps(szClientIP, iClientPort);
			dynamic_cast<LoggerSyslog &>(oErrorLog).SetProps(szClientIP, iClientPort);

			try
			{
				oWorkerProcess.HandleConnection(iClientSocket);
			}
			catch(STLW::exception & e)
			{
				oErrorLog.Emerg("Exception in HandleConnection: %s; exiting", e.what());
				close(iClientSocket);
				return EX_SOFTWARE;
			}
			catch(...)
			{
				oErrorLog.Emerg("Unknown exception in HandleConnection; exiting");
				close(iClientSocket);
				return EX_SOFTWARE;
			}

			// Close socket. Not need here, see FastCGIIO.cpp
			// close(iClientSocket);

			dynamic_cast<LoggerSyslog &>(oTransferLog).SetProps("", 0);
			dynamic_cast<LoggerSyslog &>(oErrorLog).SetProps("", 0);
		}

		// Check number of requests
		if (oWorkerContext.config.max_requests >= 0)
		{
			--oWorkerContext.config.max_requests;
			if (oWorkerContext.config.max_requests == 0)
			{
				DEBUG_HELPER_MESSAGE("Max. requests reached");
				oErrorLog.Info("Max. requests reached; exiting");

				oWorkerContext.shutdown = true;
			}
		}

		// Check shutdown flag
		if (oWorkerContext.shutdown) { break; }
	}

	DEBUG_HELPER_MESSAGE("Shutting down worker");

	oWorkerContext.scoreboard.ReadScoreboard(iScoreboardPos, oRec);
	oRec.state = Scoreboard::ScoreboardRec::SHUTDOWN;
	oWorkerContext.scoreboard.WriteScoreboard(iScoreboardPos, oRec);

return EX_OK;
}

//
// Lock socket
//
INT_32 MainProcess::Lock()
{
	do
	{
		struct flock oFlock;
		oFlock.l_type   = F_WRLCK;
		oFlock.l_start  = 0;
		oFlock.l_whence = SEEK_SET;
		oFlock.l_len    = 0;
		if      (fcntl(oWorkerContext.socket, F_SETLKW, &oFlock) != -1) { break; }
		else if (errno != EINTR || oWorkerContext.shutdown) { return -1; }
	}
	while (1);

return 0;
}

//
// Lock socket
//
INT_32 MainProcess::Unlock()
{
	do
	{
		INT_32 iErrNo = errno;
		for(;;)
		{
			struct flock oFlock;
			oFlock.l_type   = F_UNLCK;
			oFlock.l_start  = 0;
			oFlock.l_whence = SEEK_SET;
			oFlock.l_len    = 0;
			if (fcntl(oWorkerContext.socket, F_SETLK, &oFlock) != -1) { break; }
			else if (errno != EINTR) { return -1; }
		}
		errno = iErrNo;
	}
	while (0);

return 0;
}

//
// Wait for childs
//
void MainProcess::Wait()
{
	ASLogger & oErrorLog = *oWorkerContext.error_log;
	DEBUG_HELPER(&oErrorLog, "MainProcess::Wait");

	for (;;)
	{
		INT_32 iStatus = 0;
		const INT_32 iChildPid = waitpid(-1, &iStatus, WNOHANG);
		if (iChildPid <= 0) { break; }

		DEBUG_HELPER_MESSAGE("Worker terminated");

		if (WIFSIGNALED(iStatus))
		{
			oErrorLog.Error("Worker %u crashed/killed with signal %u; respawning", iChildPid, WTERMSIG(iStatus));
		}
		else if (WIFEXITED(iStatus))
		{
			const INT_32 iChildRC = WEXITSTATUS(iStatus);
			if (iChildRC == EX_CONFIG)
			{
				oErrorLog.Emerg("Worker %u terminated with status EX_CONFIG; cannot continue", iChildPid);
				exit(EX_CONFIG);
			}

			if      (iChildRC == EX_SOFTWARE) { oErrorLog.Emerg("Worker %u terminated with status EX_SOFTWARE; respawnind", iChildPid); }
			else if (iChildRC == EX_OK)       { oErrorLog.Info("Worker %u terminated normally; respawnind", iChildPid); }
			else
			{
				oErrorLog.Error("Worker %u terminated with exit code %u; respawning", iChildPid, iChildRC);
			}
		}

		const UINT_32 iScoreboardPos = mProcMap[iChildPid];

		// Re-init record
		Scoreboard::ScoreboardRec oRec;
		oWorkerContext.scoreboard.WriteScoreboard(iScoreboardPos, oRec);

		oFreeList.push_back(iScoreboardPos);
		mProcMap.erase(iChildPid);

		DEBUG_HELPER_MESSAGE("Worker removed from pool");
	}
}

//
// A destructor
//
MainProcess::~MainProcess() throw()
{
	if (bServerManagerStarted)
	{
		oWorkerContext.server_context -> server_manager -> ShutdownManager(oWorkerContext.error_log);
	}

	delete oWorkerContext.server_context -> server_manager;
	delete oWorkerContext.server_context;

	delete oWorkerContext.error_log;
	delete oWorkerContext.transfer_log;

	oWorkerContext.scoreboard.Destroy();
}

} // namespace CAS
// End.
