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
 *      FastCGIScoreboard.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_SCOREBOARD_HPP__
#define _FAST_CGI_SCOREBOARD_HPP__ 1

#include "ASTypes.hpp"

#include <string.h>

#ifndef C_SCOREBOARD_URI_LEN
#define C_SCOREBOARD_URI_LEN 224
#endif // C_SCOREBOARD_URI_LEN

namespace CAS // C++ Application Server
{
/**
  @class Scoreboard Scoreboard.hpp <Scoreboard.hpp>
  @brief FastCGI scoreboard object
*/
class Scoreboard
{
public:
	struct ScoreboardRec
	{
		/** Initial state                         */
		static const UINT_32  NONE;
		/** Child is accepting connection         */
		static const UINT_32  ACCEPT;
		/** Child is free and wait for connection */
		static const UINT_32  FREE;
		/** Reading request                       */
		static const UINT_32  READ_REQUEST;
		/** Handle request                        */
		static const UINT_32  RUN;
		/** Write response                        */
		static const UINT_32  WRITE_RESPONSE;
		/** Child shutting down                   */
		static const UINT_32  SHUTDOWN;

		/** PID                                   */
		UINT_32     pid;
		/** Worker state                          */
		UINT_32     state;
		/** Number of requests porcessed          */
		UINT_64     requests;
		/** Bytes read                            */
		UINT_64     bytes_read;
		/** Bytes written                         */
		UINT_64     bytes_written;
		/** URI                                   */
		CHAR_8      uri[C_SCOREBOARD_URI_LEN];

		/** Get state */
		CCHAR_P     GetState() const;

		inline ScoreboardRec(): pid(0),
		                        state(NONE),
		                        requests(0),
		                        bytes_read(0),
		                        bytes_written(0)
		{
			memset(uri, 0, C_SCOREBOARD_URI_LEN);
		}
	};

	/**
	  @brief Constructor
	  @param iIMaxRec - max. number of records
	*/
	Scoreboard(const UINT_32  iIMaxRec);

	/**
	  @brief Open IPC scoreboard
	*/
	INT_32 Open(CCHAR_P szIdent);

	/**
	  @brief Close IPC scoreboard
	*/
	INT_32 Close();

	/**
	  @brief Destroy
	*/
	INT_32 Destroy();

	/**
	  @brief Read scoreboard data
	*/
	INT_32 ReadScoreboard(const UINT_32    iPos,
	                      ScoreboardRec  & oScoreboardRec);

	/**
	  @brief Write Scoreboard
	*/
	INT_32 WriteScoreboard(const UINT_32          iPos,
	                       const ScoreboardRec  & oScoreboardRec);

	/**
	  @brief A destructor
	*/
	~Scoreboard() throw();
private:
	/** Max number of records */
	const UINT_32    iMaxRec;

	/** IPC Semaphore Key     */
	INT_32           iSemKeyId;
	/** IPC Key               */
	INT_32           iShmKeyId;

	/** Scoreboard records    */
	ScoreboardRec  * aScoreboard;

	/**
	  @brief Lock semaphore
	*/
	INT_32 Lock();

	/**
	  @brief Unlock semaphore
	*/
	INT_32 Unlock();
};

} // namespace CAS
#endif // _FAST_CGI_SCOREBOARD_HPP__
// End.

