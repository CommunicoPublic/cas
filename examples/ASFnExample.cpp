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
 *      ASFnExample.cpp
 *
 * $CAS$
 */

#include <CDT.hpp>
#include <CTPP2OutputCollector.hpp>
#include <CTPP2VMSyscall.hpp>

#include <stdio.h>

using namespace CTPP;

/**
  @brief Magic string
*/
#define C_INIT_SYM_PREFIX "_init"

/**
  @brief Export a class
*/
#define EXPORT_HANDLER(T) extern "C" { void * T##_init(void) { try { return new T(); } catch (...) { return NULL; } } }

/**
  @class ASFnExample ASFnExample.cpp <ASFnExample.cpp>
  @brief Example CTPP loadable function
*/
class ASFnExample:
  public SyscallHandler
{
public:
	/**
	  @brief A destructor
	*/
	~ASFnExample() throw();

	/**
	  @brief Constructor
	*/
	ASFnExample();
private:
	/**
	  @brief Copy constructor
	  @param oRhs - object to copy
	*/
	ASFnExample(const ASFnExample & oRhs);

	/**
	  @brief Copy operator =
	  @param oRhs - object to copy
	*/
	ASFnExample & operator =(const ASFnExample & oRhs);

	/**
	  @brief Pre-execution handler setup
	  @param oCollector - output data collector
	  @param oCDT - CTPP2 parameters
	  @param oSyscalls - Syscalls segment
	  @param oStaticData - Static data segment
	  @param oStaticText - Static text segment
	  @return 0 - if success, -1 - if any error occured
	*/
	INT_32 PreExecuteSetup(OutputCollector          & oCollector,
	                       CDT                      & oCDT,
	                       const ReducedStaticText  & oSyscalls,
	                       const ReducedStaticData  & oStaticData,
	                       const ReducedStaticText  & oStaticText);
	/**
	  @brief Handler
	  @param aArguments - list of arguments
	  @param iArgNum - number of arguments
	  @param oCDTRetVal - return value
	*/
	INT_32 Handler(CDT * aArguments, const UINT_32  iArgNum, CDT & oCDTRetVal, Logger & oLogger);

	/**
	  @brief Get function name
	*/
	CCHAR_P GetName() const;

};

EXPORT_HANDLER(ASFnExample)

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class ASFnExample
//

//
// Constructor
//
ASFnExample::ASFnExample()
{
	;;
}

//
// Pre-execution handler setup
//
INT_32 ASFnExample::PreExecuteSetup(OutputCollector          & oCollector,
                                    CDT                      & oCDT,
                                    const ReducedStaticText  & oSyscalls,
                                    const ReducedStaticData  & oStaticData,
                                    const ReducedStaticText  & oStaticText)
{

return 0;
}

//
// Handler
//
INT_32 ASFnExample::Handler(CDT * aArguments, const UINT_32  iArgNum, CDT & oCDTRetVal, Logger & oLogger)
{
	oCDTRetVal = CDT(CDT::STRING_VAL);

	for(INT_32 iI = iArgNum - 1; iI >= 0; --iI) { oCDTRetVal.Append(aArguments[iI].GetString()); }

return 0;
}

//
// Get function name
//
CCHAR_P ASFnExample::GetName() const { return "cas_example"; }

//
// A destructor
//
ASFnExample::~ASFnExample() throw() { ;; }

// End.
