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
 *      ASLogger.hpp
 *
 * $CAS$
 */
#ifndef _AS_LOGGER_HPP__
#define _AS_LOGGER_HPP__ 1

/**
  @file ASLogger.hpp
  @brief Application server logger subsystem
*/
#include "ASLogCodes.hpp"
#include "ASTypes.hpp"

#include <stdarg.h>

#ifndef C_START_MESSAGE_LEN
    #define C_START_MESSAGE_LEN 512
#endif

namespace CAS // C++ Application Server
{

/**
  @class ASLogger ASLogger.hpp <ASLogger.hpp>
  @brief CAS logger subsystem
*/
class ASLogger
{
protected:
	/** Base priority   */
	UINT_32     iBasePriority;

public:
	/**
	  @brief Constructor
	  @param iIBasePriority - base priority
	*/
	ASLogger(const UINT_32  iIBasePriority = AS_LOG_WARNING);

	/**
	  @brief Change base priority
	  @param iNewPriority - new base priority
	*/
	void SetPriority(const UINT_32  iNewPriority);

	/**
	  @brief Write message to log file
	  @param iPriority - priority level
	  @param szString - message to store in file
	  @param iStringLen - string length
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 WriteLog(const UINT_32  iPriority,
	                        CCHAR_P        szString,
	                        const UINT_32  iStringLen) = 0;

	/**
	  @brief Write message to log file
	  @param iPriority - priority level
	  @param sString - message to store in file
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 WriteLog(const UINT_32  iPriority,
	                        const STLW::string  & sString);

	/**
	  @brief log message with formatted parameters
	  @param iPriority - priority level
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 LogMessage(const UINT_32  iPriority,
	                          CCHAR_P        szFormat, ...);

	/**
	  @brief log message with formatted parameters
	  @param iPriority - priority level
	  @param szFormat - output format
	  @param aArgList - list of arguments
	  @return 0 - if success, -1 - otherwise
	*/
	virtual INT_32 LogMessage(const UINT_32  iPriority,
	                          CCHAR_P        szFormat,
	                          va_list        aArgList);

	/**
	  @brief log message with formatted parameters and AS_LOG_EMERG priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Emerg(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_ALERT priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Alert(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_CRIT priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Crit(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_ERR priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Err(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_ERR priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Error(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_WARNING priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Warn(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_WARNING priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Warning(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_NOTICE priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Notice(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_INFO priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Info(CCHAR_P szFormat, ...);

	/**
	  @brief log message with formatted parameters and AS_LOG_DEBUG priority
	  @param szFormat - output format
	  @return 0 - if success, -1 - otherwise
	*/
	INT_32 Debug(CCHAR_P szFormat, ...);

	/**
	  @brief A destructor
	*/
	virtual ~ASLogger() throw();
};

} // namespace CAS
#endif // _AS_LOGGER_HPP__
// End.
