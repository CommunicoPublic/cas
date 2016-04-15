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
 *      ASCTPPLogger.hpp
 *
 * $CAS$
 */
#ifndef _AS_CTPP_LOGGER_HPP__
#define _AS_CTPP_LOGGER_HPP__ 1

/**
  @file ASCTPPLogger.hpp
  @brief Application server logger subsystem; CAS-CTPP bridge
*/

#include <CTPP2Logger.hpp>

namespace CAS // C++ Application Server
{
// FWD
class ASLogger;

/**
  @class ASLogger ASLogger.hpp <ASLogger.hpp>
  @brief CAS logger subsystem
*/
class ASCTPPLogger:
  public CTPP::Logger
{
public:
	/**
	  @brief A constructor
	  @param pILogger - CAS logger
	*/
	ASCTPPLogger(ASLogger * pILogger);

	/**
	  @brief Write message to log
	  @param iPriority - priority level
	  @param szString - message to store in file
	  @param iStringLen - string length
	*/
	INT_32 WriteLog(const UINT_32  iPriority,
	                CCHAR_P        szString,
	                const UINT_32  iStringLen);

	/**
	  @brief A destructor
	*/
	~ASCTPPLogger() throw();
private:
	// Does not exist
	ASCTPPLogger(const ASCTPPLogger  & oRhs);
	ASCTPPLogger& operator=(const ASCTPPLogger  & oRhs);

	/** Logger object  */
	ASLogger    * pLogger;
};

} // namespace CAS
#endif // _AS_CTPP_LOGGER_HPP__
// End.
