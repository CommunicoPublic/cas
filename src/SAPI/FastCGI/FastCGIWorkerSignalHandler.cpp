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
 *      FastCGIWorkerSignalHandler.hpp
 *
 * $CAS$
 */
#include <sys/types.h>
#include <sysexits.h>
#include <unistd.h>

#include "ASDebugHelper.hpp"
#include "ASLogger.hpp"
#include "FastCGIWorkerSignalHandler.hpp"
#include "FastCGIWorkerContext.hpp"

namespace CAS // C++ Application Server
{
//
// Constructor
//
WorkerSignalHandler::WorkerSignalHandler(WorkerContext  & oIWorkerContext): oWorkerContext(oIWorkerContext) { ;; }

//
// A signal handler
//
void WorkerSignalHandler::Handler(const INT_32 iSignal)
{
	ASLogger & oErrorLog = *oWorkerContext.error_log;

	switch (iSignal)
	{
		// Ignore it
		case SIGPIPE:
			;;
			break;

		// Shutdown server immediatly
		case SIGINT:
			{
				DEBUG_HELPER(&oErrorLog, "WorkerSignalHandler::Handler");

				oErrorLog.Notice("Worker %d catched SIGINT. Shutting down immediately", oWorkerContext.pid);

				Scoreboard::ScoreboardRec oRec;
				oWorkerContext.scoreboard.ReadScoreboard(oWorkerContext.scoreboard_pos, oRec);
				oRec.state = Scoreboard::ScoreboardRec::SHUTDOWN;
				oWorkerContext.scoreboard.WriteScoreboard(oWorkerContext.scoreboard_pos, oRec);

				oWorkerContext.scoreboard.Close();

				DEBUG_HELPER_MESSAGE("Child terminated by SIGINT");
				exit(EX_OK);
			}
			break;

		// Commit all transactions, then shut down
		case SIGTERM:
			oWorkerContext.shutdown = true;
			oErrorLog.Notice("Worker %d catched SIGTERM. Commit all transactions and shut down", oWorkerContext.pid);
			break;

		// Re-open logs, TBD
		case SIGUSR1:
			oErrorLog.Notice("Worker %d catched SIGUSR1. Flushing all logs", oWorkerContext.pid);
			break;

		default:
			oErrorLog.Notice("Unknown signal (worker %d) catched: %d", oWorkerContext.pid, iSignal);
	}
}

//
// Destructor
//
WorkerSignalHandler::~WorkerSignalHandler() throw() { ;; }

} // namespace CAS
// End.
