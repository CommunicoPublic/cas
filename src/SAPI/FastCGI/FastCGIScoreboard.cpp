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
 *      FastCGIScoreboard.cpp
 *
 * $CAS$
 */
#include "FastCGIScoreboard.hpp"

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>

#include <errno.h>
#include <string.h>

namespace CAS // C++ Application Server
{
// Initial state
const UINT_32  Scoreboard::ScoreboardRec::NONE           = 0x00000000;
// Child is free and wait for connection
const UINT_32  Scoreboard::ScoreboardRec::FREE           = 0x00000001;
// Child is accepting connection
const UINT_32  Scoreboard::ScoreboardRec::ACCEPT         = 0x00000002;
// Reading request
const UINT_32  Scoreboard::ScoreboardRec::READ_REQUEST   = 0x00000003;
// Handle request
const UINT_32  Scoreboard::ScoreboardRec::RUN            = 0x00000004;
// Write response
const UINT_32  Scoreboard::ScoreboardRec::WRITE_RESPONSE = 0x00000005;
// Child shutting down
const UINT_32  Scoreboard::ScoreboardRec::SHUTDOWN       = 0x00000006;

//
// Constructor
//
Scoreboard::Scoreboard(const UINT_32  iIMaxRec): iMaxRec(iIMaxRec),
                                                 iSemKeyId(-1),
                                                 iShmKeyId(-1),
                                                 aScoreboard(NULL)
{
	;;
}

//
// Get state
//
CCHAR_P Scoreboard::ScoreboardRec::GetState() const
{
	// Initial state
	if (state == ScoreboardRec::NONE)           { return "N"; }
	// Child is free and wait for connection
	if (state == ScoreboardRec::FREE)           { return "F"; }
	// Child is accepting connection
	if (state == ScoreboardRec::ACCEPT)         { return "A"; }
	// Reading request
	if (state == ScoreboardRec::READ_REQUEST)   { return "R"; }
	// Handle request
	if (state == ScoreboardRec::RUN)            { return "E"; }
	// Write response
	if (state == ScoreboardRec::WRITE_RESPONSE) { return "W"; }
	// Child shutting down
	if (state == ScoreboardRec::SHUTDOWN)       { return "S"; }

return "-";
}

//
// Open IPC scoreboard
//
INT_32 Scoreboard::Open(CCHAR_P szIdent)
{
	INT_32 iKeyId = ftok(szIdent, 0xAA);
	for(;;)
	{
		iSemKeyId = semget(iKeyId, 1, 0640 | IPC_CREAT | IPC_EXCL);
		if (iSemKeyId >= 0) { break; }

		if (errno != EEXIST) { return -1; }
		++iKeyId;
	}

	iKeyId = ftok(szIdent, 0x55);
	for(;;)
	{
		iShmKeyId = shmget(iKeyId, iMaxRec * sizeof(ScoreboardRec), 0640 | IPC_CREAT | IPC_EXCL);
		if (iShmKeyId >= 0) { break; }

		if (errno != EEXIST) { Destroy(); return -1; }
		++iKeyId;
	}

	aScoreboard = (ScoreboardRec *)shmat(iShmKeyId, NULL, 0);
	if (aScoreboard == NULL) { Destroy(); return -1; }

	memset(aScoreboard, 0, iMaxRec * sizeof(ScoreboardRec));
return 0;
}

//
// Close IPC scoreboard
//
INT_32 Scoreboard::Close()
{
	if (aScoreboard != NULL) { shmdt(aScoreboard); }

return 0;
}

//
// Destroy
//
INT_32 Scoreboard::Destroy()
{
	Close();

	if (iShmKeyId > 0) { shmctl(iShmKeyId, IPC_RMID, NULL); }
	if (iSemKeyId > 0) { semctl(iSemKeyId, 0, IPC_RMID);    }

return 0;
}

//
// Lock semaphore
//
INT_32 Scoreboard::Lock()
{
	struct sembuf oSem[2];
	oSem[0].sem_num = 0;
	oSem[0].sem_op  = 0;
	oSem[0].sem_flg = SEM_UNDO;

	oSem[1].sem_num = 0;
	oSem[1].sem_op  = 1;
	oSem[1].sem_flg = SEM_UNDO | IPC_NOWAIT;

	if (semop(iSemKeyId, oSem, 2) != 0) { return -1; }

return 0;
}

//
// Unlock semaphore
//
INT_32 Scoreboard::Unlock()
{
	struct sembuf oSem[1];
	oSem[0].sem_num = 0;
	oSem[0].sem_op  = -1;
	oSem[0].sem_flg = SEM_UNDO | IPC_NOWAIT;

	if (semop(iSemKeyId, oSem, 1) != 0) { return -1; }

return 0;
}

//
// Read scoreboard data
//
INT_32 Scoreboard::ReadScoreboard(const UINT_32    iPos,
                                  ScoreboardRec  & oScoreboardRec)
{
	if (iPos >= iMaxRec) { errno = E2BIG; return -1; }

	// Lock semaphore
	if (Lock() != 0) { return -1; }

	memcpy(&oScoreboardRec, &aScoreboard[iPos], sizeof(ScoreboardRec));

	// Unlock semaphore
	if (Unlock() != 0) { return -1; }

return 0;
}

//
//  Write Scoreboard
//
INT_32 Scoreboard::WriteScoreboard(const UINT_32          iPos,
                                   const ScoreboardRec  & oScoreboardRec)
{
	if (iPos >= iMaxRec) { errno = E2BIG; return -1; }

	// Lock semaphore
	if (Lock() != 0) { return -1; }

	memcpy(&aScoreboard[iPos], &oScoreboardRec, sizeof(ScoreboardRec));

	// Unlock semaphore
	if (Unlock() != 0) { return -1; }

return 0;
}

//
// A destructor
//
Scoreboard::~Scoreboard() throw() { ;; }

} // namespace CAS
// End.

