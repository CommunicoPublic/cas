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
 *      ASApache13SAPI.hpp
 *
 * $CAS$
 */
#ifndef _AS_APACHE_13_SAPI_HPP__
#define _AS_APACHE_13_SAPI_HPP__ 1

/**
  @file ASApache13SAPI.hpp
  @brief Classes for Apache 1.3 server API
*/

// C++ Includes
#include "ASLogger.hpp"
#include "ASResponseWriter.hpp"

#include <CDT.hpp>
#include <CTPP2OutputCollector.hpp>

struct request_rec;
struct server_rec;

namespace CAS // C++ Application Server
{

/**
  @class ASLoggerApache ASApache13SAPI.hpp <ASApache13SAPI.hpp>
  @brief CAS logger subsystem (Apache 1.3)
*/
class ASLoggerApacheST:
  public ASLogger
{
public:
	/**
	  @brief Constructor
	  @param rsIRequest - Apache 1.3 request object
	*/
	ASLoggerApacheST(server_rec * rsIRequest);

	/**
	  @brief Write message to log file
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
	~ASLoggerApacheST() throw();

private:
	/** Apache request object */
	server_rec           * rsRequest;
};

/**
  @class ASLoggerApache ASApache13SAPI.hpp <ASApache13SAPI.hpp>
  @brief CAS logger subsystem (Apache 1.3)
*/
class ASLoggerApache:
  public ASLogger
{
public:
	/**
	  @brief Constructor
	  @param rsIRequest - Apache 1.3 request object
	*/
	ASLoggerApache(request_rec * rsIRequest);

	/**
	  @brief Write message to log file
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
	~ASLoggerApache() throw();

private:
	/** Apache request object */
	request_rec           * rsRequest;
};

/**
  @class ApacheOutputCollector ASApache13SAPI.hpp <ASApache13SAPI.hpp>
  @brief Output data collector (Apache 1.3)
*/
class ApacheOutputCollector:
  public CTPP::OutputCollector
{
public:
	/**
	  @brief Constructor
	  @param rsIRequest - Apache 1.3 request object
	*/
	ApacheOutputCollector(request_rec * rsIRequest);

	/**
	  @brief Collect data
	  @param vData - data to store
	  @param iDataLength - data length
	  @return 0 - if success, -1 - if any error occured
	*/
	INT_32 Collect(const void * vData, const UINT_32 iDataLength);

	/**
	  @brief A destructor
	*/
	~ApacheOutputCollector() throw();

private:
	/** Apache request object */
	request_rec           * rsRequest;
};

/**
  @class ASApacheResponseWriter ASApache13SAPI.hpp <ASApache13SAPI.hpp>
  @brief CAS response writer (Apache 1.3)
*/
class ASApacheResponseWriter:
  public ASResponseWriter
{
public:
	/**
	  @brief Constructor
	  @param rsIRequest - Apache 1.3 request object
	*/
	ASApacheResponseWriter(request_rec * rsIRequest);

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
	  @brief A destructor
	*/
	~ASApacheResponseWriter() throw();

private:
	/** Apache request object */
	request_rec            * rsRequest;
	/** Output data collector */
	ApacheOutputCollector    oOutputCollector;
};

} // namespace CAS
#endif // _AS_APACHE_13_SAPI_HPP__
// End.
