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
 *      FastCGISignalHandler.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_SIGNAL_HANDLER_HPP__
#define _FAST_CGI_SIGNAL_HANDLER_HPP__ 1

/**
  @file FastCGISignalHandler.hpp
  @brief C++-style Signal handler
*/

#include "ASTypes.hpp"
#include "STLVector.hpp"

#include <signal.h>

#define C_MAX_SIGNAL_NUM 64

namespace CAS // C++ Application Server
{

/**
  @class UserSignalHandler FastCGISignalHandler.hpp <FastCGISignalHandler.hpp>
  @brief Pure abstract class.
*/
class UserSignalHandler
{
public:
	/**
	  @brief A signal handler
	  @param iSignal - signal number
	*/
	virtual void Handler(const INT_32 iSignal) = 0;
	/**
	  @brief A Virtual Destructor
	*/
	virtual ~UserSignalHandler() throw() { ;; }
};

/**
  @class SignalHandler FastCGISignalHandler.hpp <FastCGISignalHandler.hpp>
  @brief Signal Handler. Singleton.
*/
class SignalHandler
{
public:
	/**
	  @brief Register user Handler
	  @param iSignal - signal
	  @param pHandler - pointer to the handler
	*/
	INT_32 RegisterHandler(const INT_32         iSignal,
	                       UserSignalHandler  * pHandler);

	/**
	  @brief Remove handler from factory
	  @param iSignal - signal
	  @param pHandler - pointer to the handler
	*/
	INT_32 RemoveHandler(const INT_32         iSignal,
	                     UserSignalHandler  * pHandler);

	/**
	  @brief Create singleton instance
	*/
	static SignalHandler & Instance();

	/**
	  @brief Remove all signal handlers
	*/
	void Clear();

private:
	/** User handler Factory */
	static STLW::vector<UserSignalHandler *> mHandler[C_MAX_SIGNAL_NUM];

	/** Signal Action */
	static struct sigaction sSA;

	// Does not exist
	SignalHandler(const SignalHandler & oSignalHandler);
	SignalHandler & operator =(const SignalHandler & oSignalHandler);

	/**
	  @brief Signal Handler
	  @param iSignal - signal
	*/
	static void Handler(INT_32 iSignal);

	/** Private Constructor */
	SignalHandler();
};

} // namespace CAS
#endif // _FAST_CGI_SIGNAL_HANDLER_HPP__
// End.
