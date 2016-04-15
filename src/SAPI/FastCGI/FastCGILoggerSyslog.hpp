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
 *      FastCGILoggerSyslog.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_LOGGER_SYSLOG_HPP__
#define _FAST_CGI_LOGGER_SYSLOG_HPP__ 1

/**
  @file LoggerSyslog.hpp
  @brief Application server logger subsystem; write messages to syslog
*/

#include "ASLogger.hpp"
#include "STLString.hpp"

#include <stdio.h>

namespace CAS // C++ Application Server
{

/**
  @class LoggerSyslog FastCGILoggerSyslog.hpp <FastCGILoggerSyslog.hpp>
  @brief CAS logger subsystem
*/
class LoggerSyslog:
  public ASLogger
{
public:
	/**
	  @brief A constructor
	  @param oIF - log file descriptor
	*/
	LoggerSyslog(FILE       * oIF,
	             const bool   bITransferLog);

	/**
	  @brief Set client properties
	*/
	void SetProps(CCHAR_P  szClientIP,
	              INT_32   iClientPort);

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
	  @brief Re-open log
	*/
	void Reopen();

	/**
	  @brief A destructor
	*/
	~LoggerSyslog() throw();
private:
	/** Logger            */
	FILE        * F;
	/** Transfer log flag */
	const bool    bTransferLog;

	/** Syslog options */
	INT_32        iOptions;
	/** Facility       */
	INT_32        iFacility;
	/** Ident          */
	STLW::string  sIdent;

	/** Client IP   */
	STLW::string  sClientIP;
	/** Client port */
	INT_32        iClientPort;

	/** Get logger priority */
	CCHAR_P GetPriority(const INT_32 iPriority);
};

} // namespace CAS
#endif // _FAST_CGI_LOGGER_SYSLOG_HPP__
// End.
