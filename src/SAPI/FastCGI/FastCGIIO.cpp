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
 *      FastCGIIO.cpp
 *
 * $CAS$
 */
#include "ASLogger.hpp"
#include "FastCGI.hpp"
#include "FastCGIIO.hpp"
#include "FastCGIWorkerContext.hpp"

#include <sys/types.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <stdio.h>

namespace CAS // C++ Application Server
{
//
//  Constructor
//
FastCGIIO::FastCGIIO(const INT_32     iIClientSocket,
                     WorkerContext  & oIWorkerContext): iClientSocket(iIClientSocket),
                                                        oWorkerContext(oIWorkerContext)
{
	;;
}

//
// Read request from the socket
//
INT_32 FastCGIIO::ReadRequest(INT_32        & iPacketType,
                              INT_32        & iRequestId,
                              void          * vData,
                              const INT_32    iDataLen)
{
	ASLogger & oErrorLog = *oWorkerContext.error_log;

	FastCGIHeader oReqHeader;
	if (ReadPacket(&oReqHeader, sizeof(FastCGIHeader)) != sizeof(FastCGIHeader))
	{
		oErrorLog.Error("Can't read packet header: (%d)%s", errno, strerror(errno));
		return -1;
	}

	if (oReqHeader.version != FastCGIHeader::FCGI_VERSION)
	{
		oErrorLog.Error("Invalid FCGI header: version mismatch; need %u, got %u", FastCGIHeader::FCGI_VERSION, oReqHeader.version);
		return -1;
	}

	iPacketType = oReqHeader.type;
	iRequestId  = UINT_32(oReqHeader.request_id_b1 << 8) + oReqHeader.request_id_b0;

	const INT_32 iPacketLen = UINT_32(oReqHeader.content_length_b1 << 8) + oReqHeader.content_length_b0;
	if (iPacketLen == 0) { return 0; }

	const INT_32 iFullLen = iPacketLen + oReqHeader.padding_length;
	if (iFullLen > iDataLen)
	{
		oErrorLog.Error("Invalid FCGI packet; packet length %u > buffer size %u", iFullLen, iDataLen);
		return -1;
	}

	if (ReadPacket(vData, iFullLen) != iFullLen)
	{
		oErrorLog.Error("Can't read packet: (%d)%s", errno, strerror(errno));
		return -1;
	}

return iPacketLen;
}

//
// Write response to the socket
//
INT_32 FastCGIIO::WriteResponse(const INT_32    iPacketType,
                                const INT_32    iRequestId,
                                const void    * vData,
                                const INT_32    iDataLen)
{
	ASLogger & oErrorLog = *oWorkerContext.error_log;

	//                           1  2  3  4  5  6  7
	static CHAR_8 aPadding[] = { 0, 0, 0, 0, 0, 0, 0 };

	INT_32  iBytesLeft = iDataLen;
	CCHAR_P vPacket    = CCHAR_P(vData);
	for (;;)
	{
		const INT_32 iPacketLen = iBytesLeft < 0xFFF8 ? iBytesLeft : 0xFFF8;

		FastCGIHeader oRespHeader;
		oRespHeader.version           = FastCGIHeader::FCGI_VERSION;
		oRespHeader.type              = iPacketType;

		oRespHeader.request_id_b1     = ((iRequestId >> 8) & 0xFF);
		oRespHeader.request_id_b0     = (iRequestId & 0xFF);

		oRespHeader.content_length_b1 = ((iPacketLen >> 8) & 0xFF);
		oRespHeader.content_length_b0 = (iPacketLen & 0xFF);

		oRespHeader.padding_length    = ((iPacketLen + 7) & ~7) - iPacketLen;
		oRespHeader.reserved          = 0;

		if (WritePacket(&oRespHeader, sizeof(FastCGIHeader)) != sizeof(FastCGIHeader))
		{
			oErrorLog.Error("Can't write packet header: (%d)%s", errno, strerror(errno));
			return -1;
		}

		if (iPacketLen != 0)
		{
			if (WritePacket(vPacket, iPacketLen) != iPacketLen)
			{
				oErrorLog.Error("Can't write packet body: (%d)%s", errno, strerror(errno));
				return -1;
			}
		}

		if (oRespHeader.padding_length > 0)
		{
			if (WritePacket(aPadding, oRespHeader.padding_length) != oRespHeader.padding_length)
			{
				oErrorLog.Error("Can't write packet padding: (%d)%s", errno, strerror(errno));
				return -1;
			}
		}

		vPacket    += iPacketLen;
		iBytesLeft -= iPacketLen;
		if (iBytesLeft == 0) { break; }
	}

return iDataLen;
}

//
// Read packet from socket
//
INT_32 FastCGIIO::ReadPacket(void          * vReadBuffer,
                             const INT_32    iPacketLen)
{
	INT_32 iTotalRead = 0;
	INT_32 iNeedRead  = iPacketLen;
	for(;;)
	{
		if (PollSocket(POLLIN) < 0) { return -1; }

		INT_32 iReadBytes = 0;
		do
		{
			iReadBytes = read(iClientSocket, CHAR_P(vReadBuffer) + iTotalRead, iNeedRead);
		}
		while (iReadBytes == -1 && errno == EINTR);
		if (iReadBytes <= 0) { return -1; }

		oWorkerContext.bytes_read += iReadBytes;

		iTotalRead += iReadBytes;
		iNeedRead  -= iReadBytes;
		if (iTotalRead == iPacketLen) { return iPacketLen; }
	}

return -1;
}

//
// Write packet to the socket
//
INT_32 FastCGIIO::WritePacket(const void    * vWriteBuffer,
                              const INT_32    iPacketLen)
{
	INT_32 iTotalSent = 0;
	INT_32 iNeedSend  = iPacketLen;
	for(;;)
	{
		if (PollSocket(POLLOUT) < 0) { return -1; }

		INT_64 iSentBytes = 0;
		do
		{
			iSentBytes = write(iClientSocket, CCHAR_P(vWriteBuffer) + iTotalSent, iNeedSend);
		}
		while (iSentBytes == -1 && errno == EINTR);
		if (iSentBytes <= 0) { return -1; }

		oWorkerContext.bytes_written += iSentBytes;

		iTotalSent += iSentBytes;
		iNeedSend  -= iSentBytes;
		if (iTotalSent == iPacketLen) { return iPacketLen; }
	}

return -1;
}

//
// Poll socket
//
INT_32 FastCGIIO::PollSocket(const INT_32 iEvents)
{
	struct pollfd aFD;
	aFD.fd      = iClientSocket;
	aFD.events  = iEvents;
	aFD.revents = 0;

	INT_32 iNFds = 0;
	do
	{
		iNFds = poll(&aFD, 1, oWorkerContext.config.io_timeout);
	}
	while (iNFds < 0 && errno == EINTR);

	if (iNFds == 1 && (aFD.revents & iEvents)) { return 0; }

	if (iNFds == 0) { errno = ETIMEDOUT; }

return -1;
}

//
// A destructor
//
FastCGIIO::~FastCGIIO() throw()
{
	close(iClientSocket);
}

} // namespace CAS
// End.
