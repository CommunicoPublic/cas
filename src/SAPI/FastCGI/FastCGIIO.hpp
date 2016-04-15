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
 *      FastCGIIO.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_IO_HPP__
#define _FAST_CGI_IO_HPP__ 1

#include "ASTypes.hpp"

namespace CAS // C++ Application Server
{
// FWD
struct WorkerContext;

/**
  @class FastCGIIO FastCGIIO.hpp <FastCGIIO.hpp>
  @brief FastCGI imput/outpout writer
*/
class FastCGIIO
{
public:
	/**
	  @brief Constructor
	  @param iIClientSocket - client socket
	  @param oIWorkerContext - FastCGI worker context
	*/
	FastCGIIO(const INT_32     iIClientSocket,
	          WorkerContext  & oIWorkerContext);

	/**
	  @brief Read request from the socket
	*/
	INT_32 ReadRequest(INT_32        & iPacketType,
	                   INT_32        & iRequestId,
	                   void          * vData,
	                   const INT_32    iDataLen);

	/**
	  @brief Write response to the socket
	*/
	INT_32 WriteResponse(const INT_32    iPacketType,
	                     const INT_32    iRequestId,
	                     const void    * vData,
	                     const INT_32    iDataLen);

	/**
	  @brief Read packet from socket
	*/
	INT_32 ReadPacket(void          * vReadBuffer,
	                  const INT_32    iPacketLen);

	/**
	  @brief Write packet to the socket
	*/
	INT_32 WritePacket(const void    * vReadBuffer,
	                   const INT_32    iPacketLen);

	/**
	  @brief A destructor
	*/
	~FastCGIIO() throw();
private:
	// Does not exist
	FastCGIIO(const FastCGIIO & oRhs);
	FastCGIIO& operator=(const FastCGIIO & oRhs);

	/** Client socket              */
	const INT_32     iClientSocket;
	/** Worker context             */
	WorkerContext  & oWorkerContext;

	/**
	  @brief Poll socket
	*/
	INT_32 PollSocket(const INT_32  iEvents);
};

} // namespace CAS
#endif // _FAST_CGI_IO_HPP__
// End.
