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
 *      ASLoggerFile.hpp
 *
 * $CAS$
 */
#ifndef _AS_LOGGER_FILE_HPP__
#define _AS_LOGGER_FILE_HPP__ 1

/**
  @file ASLoggerFile.hpp
  @brief Application server logger subsystem; write messages to file
*/

#include "ASLogger.hpp"

#include <stdio.h>

namespace CAS // C++ Application Server
{

/**
  @class ASLogger ASLogger.hpp <ASLogger.hpp>
  @brief CAS logger subsystem
*/
class ASLoggerFile:
  public ASLogger
{
public:
	/**
	  @brief A constructor
	  @param oIF - log file descriptor
	*/
	ASLoggerFile(FILE * oIF);

	/**
	  @brief Write message to log file
	  @param iPriority - priority level
	  @param szString - message to store in file
	  @param iStringLen - string length
	  @return 0- if success, -1 - if any error occured
	*/
	INT_32 WriteLog(const UINT_32  iPriority,
	                CCHAR_P        szString,
	                const UINT_32  iStringLen);

	/**
	  @brief A destructor
	*/
	~ASLoggerFile() throw();
private:
	// Does not exist
	ASLoggerFile(const ASLoggerFile  & oRhs);
	ASLoggerFile& operator=(const ASLoggerFile  & oRhs);

	FILE     * F;
};

} // namespace CAS
#endif // _AS_LOGGER_HPP__
// End.
