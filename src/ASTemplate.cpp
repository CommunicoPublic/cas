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
 *      ASTemplate.cpp
 *
 * $CAS$
 */

#include "ASTemplate.hpp"
#include "ASException.hpp"

#include <CTPP2Error.hpp>
#include <CTPP2ErrorCodes.h>
#include <CTPP2FileSourceLoader.hpp>
#include <CTPP2Parser.hpp>
#include <CTPP2ParserException.hpp>
#include <CTPP2VMDumper.hpp>
#include <CTPP2VMFileLoader.hpp>

#include <stdio.h>

namespace CAS // C++ Application Server
{

//
// Constructor
//
ASTemplate::ASTemplate(const STLW::string                & sITemplate,
                       const eTemplateType               & oITemplateType,
                       const STLW::vector<STLW::string>  & vIncludeDirs): sTemplate(sITemplate),
                                                                          oTemplateType(oITemplateType),
                                                                          pLoader(NULL),
                                                                          pDumper(NULL),
                                                                          pVMMemoryCore(NULL)
{
	using namespace CTPP;

	if      (oTemplateType == BYTECODE)
	{
		// Load template
		// TODO: create appropriate multipurpose loader: from shared memory, from file, network, etc...
		try
		{
			// Load program from file
			pLoader = new VMFileLoader(sTemplate.c_str());

			// Get program core
			pVMMemoryCore = pLoader -> GetCore();
		}
		catch(CTPPUnixException & e)
		{
			if (pLoader != NULL) { delete pLoader; }
			STLW::string sTMP("Cannot load bytecode from file \"");
			sTMP.append(sTemplate);
			sTMP.append("\"; in ");
			sTMP.append(e.what());
			sTMP.append("\": ");
			sTMP.append(strerror(e.ErrNo()));
			throw LogicError(sTMP.c_str());
		}
	}
	else if (oTemplateType == SOURCE)
	{
		CTPPError oCTPPError;
		INT_32 iRC = -1;
		try
		{
			// Load template and try to compile it
			// Any error should be catched on uppwer level
			// TODO: create appropriate multipurpose loader: from shared memory, from file, network, etc...
			VMOpcodeCollector  oVMOpcodeCollector;
			StaticText         oSyscalls;
			StaticData         oStaticData;
			StaticText         oStaticText;
			HashTable          oHashTable;
			CTPP2Compiler oCompiler(oVMOpcodeCollector, oSyscalls, oStaticData, oStaticText, oHashTable);

			// Load template
			CTPP2FileSourceLoader oSourceLoader;
			// Set include directories
			oSourceLoader.SetIncludeDirs(vIncludeDirs);
			// Load template
			oSourceLoader.LoadTemplate(sTemplate.c_str());

			// Create template parser
			CTPP2Parser oCTPP2Parser(&oSourceLoader, &oCompiler, sTemplate.c_str());

			// Compile template
			oCTPP2Parser.Compile();

			// Get program core
			UINT_32 iCodeSize = 0;
			const VMInstruction * oVMInstruction = oVMOpcodeCollector.GetCode(iCodeSize);
			// Dump program
			pDumper = new VMDumper(iCodeSize, oVMInstruction, oSyscalls, oStaticData, oStaticText, oHashTable);
			UINT_32 iSize = 0;
			const VMExecutable * aProgramCore = pDumper -> GetExecutable(iSize);

			// Get program core
			pVMMemoryCore = new VMMemoryCore(aProgramCore);
			iRC = 0;
		}
		// Catch parser-specific exceptions
		// Operatos mismatch
		catch (CTPPParserOperatorsMismatch  & e)
		{
			if (pDumper != NULL)       { delete pDumper; }
			if (pVMMemoryCore != NULL) { delete pVMMemoryCore; }

			CHAR_8 szBuffer[1024 + 1];
			snprintf(szBuffer, 1024, "\" syntax error 0x%08X at line %d pos %d: ", CTPP_COMPILER_ERROR | CTPP_SYNTAX_ERROR, e.GetLine(), e.GetLinePos());
			STLW::string sTMP("In file \"");
			sTMP += sTemplate + szBuffer + ": expected \"" + e.Expected() + "\", but found \"" + e.Found() + "\"";
			throw LogicError(sTMP.c_str());
		}
		// SYntax error
		catch (CTPPParserSyntaxError        & e)
		{
			if (pDumper != NULL)       { delete pDumper; }
			if (pVMMemoryCore != NULL) { delete pVMMemoryCore; }

			CHAR_8 szBuffer[1024 + 1];
			snprintf(szBuffer, 1024, "\" syntax error 0x%08X at line %d pos %d: ", CTPP_COMPILER_ERROR | CTPP_SYNTAX_ERROR, e.GetLine(), e.GetLinePos());
			STLW::string sTMP("In file \"");
			sTMP += sTemplate + szBuffer + e.what();
			throw LogicError(sTMP.c_str());
		}
		// All other exceptions with common template
		catch (CTPPUnixException            & e)
		{
			STLW::string sTMP(e.what());
			sTMP.append(": ");
			sTMP.append(strerror(e.ErrNo()));
			oCTPPError = CTPPError(sTemplate, sTMP, CTPP_COMPILER_ERROR | CTPP_UNIX_ERROR, 0, 0, 0);
		}
		catch (CDTRangeException            & e) { oCTPPError = CTPPError(sTemplate, e.what(), CTPP_COMPILER_ERROR | CTPP_RANGE_ERROR,         0, 0, 0); }
		catch (CDTAccessException           & e) { oCTPPError = CTPPError(sTemplate, e.what(), CTPP_COMPILER_ERROR | CTPP_ACCESS_ERROR,        0, 0, 0); }
		catch (CDTTypeCastException         & e) { oCTPPError = CTPPError(sTemplate, e.what(), CTPP_COMPILER_ERROR | CTPP_TYPE_CAST_ERROR,     0, 0, 0); }
		catch (CTPPLogicError               & e) { oCTPPError = CTPPError(sTemplate, e.what(), CTPP_COMPILER_ERROR | CTPP_LOGIC_ERROR,         0, 0, 0); }
		catch (CTPPException                & e) { oCTPPError = CTPPError(sTemplate, e.what(), CTPP_COMPILER_ERROR | CTPP_UNKNOWN_ERROR,       0, 0, 0); }
		catch (std::exception               & e) { oCTPPError = CTPPError(sTemplate, e.what(), CTPP_COMPILER_ERROR | STL_UNKNOWN_ERROR,        0, 0, 0); }
		catch (...)                              { oCTPPError = CTPPError(sTemplate, "Unknown Error", CTPP_COMPILER_ERROR | STL_UNKNOWN_ERROR, 0, 0, 0); }

		// Error?
		if (iRC == -1)
		{
			if (pDumper != NULL)       { delete pDumper; }
			if (pVMMemoryCore != NULL) { delete pVMMemoryCore; }

			CHAR_8 szBuffer[1024 + 1];
			snprintf(szBuffer, 1024, "\" error 0x%08X: ", oCTPPError.error_code);
			STLW::string sTMP("In file \"");
			sTMP += oCTPPError.template_name + szBuffer + oCTPPError.error_descr;
			throw LogicError(sTMP.c_str());
		}
	}
}

//
// Get Bytecode
//
const CTPP::VMMemoryCore * ASTemplate::GetBytecode() const
{
	// Stub
	// TODO: when bytecode was loaded from shared memory, try to reload it every call of this method
	return pVMMemoryCore;
}

//
// A destructor
//
ASTemplate::~ASTemplate() throw()
{
	delete pVMMemoryCore;

	if (pLoader != NULL) { delete pLoader; }

	if (pDumper != NULL) { delete pDumper; }
}

} // namespace CAS
// End.
