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
#include "FastCGISignalHandler.hpp"

namespace CAS // C++ Application Server
{
/** User handler Factory */
STLW::vector<UserSignalHandler *> SignalHandler::mHandler[C_MAX_SIGNAL_NUM];

/** Signal Action */
struct sigaction SignalHandler::sSA;

//
// Private Constructor
//
SignalHandler::SignalHandler()
{
	sSA.sa_handler = Handler;
	sigemptyset(&sSA.sa_mask);
	sSA.sa_flags = 0;
}

//
// Create instance
//
SignalHandler & SignalHandler::Instance()
{
	static SignalHandler oHanlder;

return oHanlder;
}

//
// Register user Handler
//
INT_32 SignalHandler::RegisterHandler(const INT_32 iSignal, UserSignalHandler * pHandler)
{
	if (iSignal > C_MAX_SIGNAL_NUM || iSignal < 0) { return -1; }

	if (!sigismember(&sSA.sa_mask, iSignal)) { sigaction(iSignal, &sSA, NULL); }

	mHandler[iSignal].push_back(pHandler);

return 0;
}

//
// Register user Handler
//
INT_32 SignalHandler::RemoveHandler(const INT_32 iSignal, UserSignalHandler * pHandler)
{
	if (iSignal > C_MAX_SIGNAL_NUM || iSignal < 0) { return -1; }

	STLW::vector<UserSignalHandler *>::iterator itV = mHandler[iSignal].begin();
	const STLW::vector<UserSignalHandler *>::iterator itVEnd = mHandler[iSignal].end();
	while (itV != itVEnd)
	{
		if (pHandler == *itV) { mHandler[iSignal].erase(itV); return 0; }
		itV++;
	}

return -1;
}

//
// Static C Handler
//
void SignalHandler::Handler(INT_32 iSignal)
{
	try
	{
		STLW::vector<UserSignalHandler *>::iterator itV = mHandler[iSignal].begin();
		const STLW::vector<UserSignalHandler *>::iterator itVEnd = mHandler[iSignal].end();
		while (itV != itVEnd)
		{
			try { (*itV) -> Handler(iSignal); }
			catch(...) { ;; }
			itV++;
		}
	}
	catch(...) { ;; }
}

//
// Remove all signal handlers
//
void SignalHandler::Clear()
{
	sigemptyset(&sSA.sa_mask);

	for(UINT_32 iSignal = 0; iSignal < C_MAX_SIGNAL_NUM; ++iSignal)
	{
		std::vector<UserSignalHandler *> vTMP;
		mHandler[iSignal].swap(vTMP);
	}
}

} // namespace CAS
// End.
