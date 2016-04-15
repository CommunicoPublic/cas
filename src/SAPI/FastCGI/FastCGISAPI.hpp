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
 *      FastCGISAPI.hpp
 *
 * $CAS$
 */
#ifndef _FAST_CGI_SAPI_HPP__
#define _FAST_CGI_SAPI_HPP__ 1

/**
  @file FastCGISAPI.hpp
  @brief Classes for FastCGI server API
*/

// C++ Includes
#include "ASLogger.hpp"
#include "ASResponseWriter.hpp"

#include <CDT.hpp>
#include <CTPP2OutputCollector.hpp>

namespace CAS // C++ Application Server
{
// FWD
class FastCGIIO;
struct WorkerContext;

/**
  @class FastCGIOutputCollector FastCGISAPI.hpp <FastCGISAPI.hpp>
  @brief Output data collector (FastCGI)
*/
class FastCGIOutputCollector:
  public CTPP::OutputCollector
{
public:
	/**
	  @brief Constructor
	  @param oIFastCGIIO - FastCGI I/O object
	  @param iIReqId - Request Id
	*/
	FastCGIOutputCollector(FastCGIIO     & oIFastCGIIO,
	                       const INT_32    iIReqId);

	/**
	  @brief Collect data
	  @param vData - data to store
	  @param iDataLength - data length
	  @return 0 - if success, -1 - if any error occured
	*/
	INT_32 Collect(const void     * vData,
	               const UINT_32    iDataLength);

	/**
	  @brief A destructor
	*/
	~FastCGIOutputCollector() throw();

private:
	friend class FastCGIResponseWriter;

	/** FastCGI I/O object */
	FastCGIIO        & oFastCGIIO;
	/** Request Id         */
	const INT_32       iReqId;
	/** Output buffer size */
	UINT_32            iOutputBufferSize;
	/** Output buffer      */
	void *             vOutputBuffer;
	/** Output buffer pos. */
	UINT_32            iOutputBufferPos;

	/**
	  @brief Flush buffer
	*/
	INT_32 Flush();
};

/**
  @class ResponseWriter FastCGISAPI.hpp <FastCGISAPI.hpp>
  @brief CAS response writer (FastCGI)
*/
class FastCGIResponseWriter:
  public ASResponseWriter
{
public:
	/**
	  @brief Constructor
	  @param oIFastCGIIO - FastCGI I/O object
	  @param iIReqId - Request Id
	*/
	FastCGIResponseWriter(WorkerContext & oIWorkerContext,
	                      FastCGIIO     & oIFastCGIIO,
	                      const INT_32    iIReqId);

	/**
	  @brief Write HTTP header
	  @param oResponse - response object
	*/
	INT_32 WriteHeader(ASResponse & oResponse);

	/**
	  @brief Get output data collector
	  @return output data collector
	*/
	CTPP::OutputCollector * GetBodyOutputCollector();

	/**
	  @brief Flush buffer
	*/
	INT_32 Flush();

	/**
	  @brief A destructor
	*/
	~FastCGIResponseWriter() throw();

private:
	/** Worker context          */
	WorkerContext           & oWorkerContext;
	/** FastCGI I/O object      */
	FastCGIIO               & oFastCGIIO;
	/** Output data collector   */
	FastCGIOutputCollector    oOutputCollector;
};

} // namespace CAS
#endif // _FAST_CGI_SAPI_HPP__
// End.
