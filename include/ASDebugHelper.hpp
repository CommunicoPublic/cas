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
 *      ASDebugReporter.hpp
 *
 * $CAS$
 */
#ifndef _AS_DEBUG_HELPER_HPP__
#define _AS_DEBUG_HELPER_HPP__ 1

/**
  @file ASDebugHelper.hpp
  @brief Application server logger subsystem
*/
#include "ASTypes.hpp"
#include "STLString.hpp"

namespace CAS // C++ Application Server
{
class ASLogger;

#ifdef EXTRA_DEBUG_MODE
    #define USE_DEBUG_HELPER 1
#endif

#ifdef USE_DEBUG_HELPER
    #define DEBUG_HELPER(x, y)          CAS::ASDebugHelper oDebugHelper((x), (y));
    #define DEBUG_HELPER_MESSAGE(y...)  oDebugHelper.WriteLog(y);
#else
    #define DEBUG_HELPER(x, y)          { ;; }
    #define DEBUG_HELPER_MESSAGE(y...)  { ;; }
#endif

/**
  @struct ASDebugHelper ASDebugHelper.hpp <ASDebugHelper.hpp>
  @brief CAS logger subsystem helper
*/
class ASDebugHelper
{
public:
	/**
	  @brief Constructor
	  @param pILogger - logger object
	  @param sIData - Block description
	*/
	ASDebugHelper(ASLogger            * pILogger,
	              const STLW::string  & sIData);

	/**
	  @brief Write message to log
	  @param szMessage - string to write
	*/
	UINT_32 WriteLog(CCHAR_P szMessage, ...);

	/**
	  @brief Destructor
	*/
	~ASDebugHelper() throw();

	/**
	  @brief initialize reporter
	*/
	static void InitReporter();
private:
	// Does not exist
	ASDebugHelper(const ASDebugHelper  & oRhs);
	ASDebugHelper& operator =(const ASDebugHelper  & oRhs);

	/** Logger object     */
	ASLogger       * pLogger;
	/** Block description */
	STLW::string     sData;
	/** Recursion level   */
	static INT_32    iLevel;
	/** Internal flag     */
	INT_32           iFlag;
};

} // namespace CAS
#endif // _AS_DEBUG_HELPER_HPP__
// End.
