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
 *      CTPPView.hpp
 *
 * $CAS$
 */
#include <CTPP2SyscallFactory.hpp>
#include <CTPP2FileSourceLoader.hpp>
#include <CTPP2Parser.hpp>
#include <CTPP2ParserException.hpp>
#include <CTPP2StringOutputCollector.hpp>
#include <CTPP2VMException.hpp>
#include <CTPP2VMStackException.hpp>
#include <CTPP2VMExecutable.hpp>
#include <CTPP2VMMemoryCore.hpp>
#include <CTPP2VMOpcodeCollector.hpp>
#include <CTPP2VMSTDLib.hpp>
#include <CTPP2VMDebugInfo.hpp>

#include <CTPP2Error.hpp>
#include <CTPP2Exception.hpp>
#include <CTPP2VM.hpp>
#include <CTPP2VMDumper.hpp>

#include "CTPPView.hpp"
#include <stdio.h>

namespace TEST_NS
{

EXPORT_HANDLER(CTPPView)
using namespace CAS;

// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// Class CTPPView
//

//
// A constructor
//
CTPPView::CTPPView(): pSyscallFactory(NULL), pVM(NULL), pLoader(NULL)
{
	;;
}
//
// Constructor
//
FunctionConfig::FunctionConfig(): udf(NULL)
{
	;;
}

//
// Get handler name
//
CCHAR_P CTPPView::GetObjectName() const { return "CTPPView"; }

//
// Global initialization
//
INT_32 CTPPView::InitModule(CTPP::CDT  & oConfiguration,
                             ASPool     & oModulesPool,
                             ASPool     & oObjectPool,
                             ASLogger   & oLogger)
{
	using namespace CTPP;
	DEBUG_HELPER(&oLogger, "CTPPView::InitModule");

	CTPPError oCTPPError;
	INT_32 iRC  = VIEW_ERROR;
	try
	{
		// Get max. number of handlers
		UINT_32 iMaxHandlers = atoi(oConfiguration.GetCDT("MaxFunctions").GetString().c_str());

		// Min. safe value
		if (iMaxHandlers <= 1024) { oLogger.Warn("MaxHandlers less than minimal safe value (1024), set default"); iMaxHandlers = 1024; }

		// Argument stack size
		iArgStackSize  = atoi(oConfiguration.GetCDT("ArgStackSize").GetString().c_str());
		// Code stack size
		iCodeStackSize = atoi(oConfiguration.GetCDT("CodeStackSize").GetString().c_str());
		// Execution steps limit
		iStepsLimit    = atoi(oConfiguration.GetCDT("StepsLimit").GetString().c_str());

		if (iArgStackSize  == 0) { oLogger.Warn("ArgStackSize == 0, set default (10240)");     iArgStackSize  = 10240;       }
		if (iCodeStackSize == 0) { oLogger.Warn("CodeStackSize == 0, set default (10240)");    iCodeStackSize = 10240;       }
		if (iStepsLimit    == 0) { oLogger.Warn("StepsLimit == 0, set default (1024 * 1024)"); iStepsLimit    = 1024 * 1024; }

		STLW::string sInitPrefix = oConfiguration.GetCDT("InitPrefix").GetString();
		if (sInitPrefix.empty()) { oLogger.Warn("InitPrefix undefined, set default (\"_init\")"); sInitPrefix = "_init"; }

		DEBUG_HELPER_MESSAGE("Create syscall factory");

		// Create syscall factory
		pSyscallFactory = new SyscallFactory(iMaxHandlers);

		DEBUG_HELPER_MESSAGE("Load standard library");

		// Load standard library
		STDLibInitializer::InitLibrary(*pSyscallFactory);

		DEBUG_HELPER_MESSAGE("Load CTPP2 external functions");

		// Get list of include directories
		CDT oList = oConfiguration.GetCDT("LibexecDirs");

		// Library include directories
		STLW::vector<STLW::string> vLibexecDirs;
		if (oList.GetType() == CDT::HASH_VAL)
		{
			oList = oList.GetCDT("LibexecDir");
			if (oList.GetType() == CDT::STRING_VAL) { vLibexecDirs.push_back(oList.GetString()); }
			else if (oList.GetType() == CDT::ARRAY_VAL)
			{
				for(UINT_32 iPos = 0; iPos < oList.Size(); ++iPos)
				{
					vLibexecDirs.push_back(oList[iPos].GetString());
				}
			}
			// Nothing to do
			else if (oList.GetType() == CDT::UNDEF) { ;; }
			// Error
			else
			{
				oLogger.Error("Invalid type of \"LibexecDir\" directive.");
				DEBUG_HELPER_MESSAGE("Invalid type of \"LibexecDir\" directive.");

				return VIEW_ERROR;
			}
		}

		// Get list of functions
		oList = oConfiguration.GetCDT("Functions");
		if (oList.GetType() == CDT::HASH_VAL)
		{
			oList = oList.GetCDT("Function");
			if (oList.GetType() == CDT::HASH_VAL)
			{
				// Get function library and class name
				FunctionConfig oFunctionConfig;
				oFunctionConfig.library = oList.GetCDT("library").GetString();
				if (oFunctionConfig.library.empty())
				{
					oLogger.Error("Library file of function is empty");
					DEBUG_HELPER_MESSAGE("Library file of function is empty");

					return VIEW_ERROR;
				}

				oFunctionConfig.name = oList.GetCDT("name").GetString();
				if (oFunctionConfig.name.empty())
				{
					oLogger.Error("Name of function is empty");
					DEBUG_HELPER_MESSAGE("Name of function is empty");

					return VIEW_ERROR;
				}

				oFunctionConfig.config = oList.GetCDT("config").GetString();
				oFunctionConfig.udf = NULL;
				vFunctions.push_back(oFunctionConfig);
			}
			else if (oList.GetType() == CDT::ARRAY_VAL)
			{
				for(UINT_32 iPos = 0; iPos < oList.Size(); ++iPos)
				{
					// Get function library and class name
					FunctionConfig oFunctionConfig;
					oFunctionConfig.name = oList[iPos].GetCDT("name").GetString();
					if (oFunctionConfig.name.empty())
					{
						oLogger.Error("Library file of function is empty");
						DEBUG_HELPER_MESSAGE("Library file of function is empty");

						return VIEW_ERROR;
					}

					oFunctionConfig.library = oList[iPos].GetCDT("library").GetString();
					if (oFunctionConfig.name.empty())
					{
						oLogger.Error("Name of function is empty");
						DEBUG_HELPER_MESSAGE("Name of function is empty");

						return VIEW_ERROR;
					}

					oFunctionConfig.config = oList[iPos].GetCDT("config").GetString();
					oFunctionConfig.udf = NULL;
					vFunctions.push_back(oFunctionConfig);
				}
			}
			// Nothing to do
			else if (oList.GetType() == CDT::UNDEF) { ;; }
			// Error
			else
			{
				oLogger.Error("Invalid type of \"Function\" directive.");
				DEBUG_HELPER_MESSAGE("Invalid type of \"Function\" directive.");

				return VIEW_ERROR;
			}
		}

		if (vFunctions.size() != 0)
		{
			// Dynamic loader
			pLoader = new ASLoader<CTPP::SyscallHandler>(sInitPrefix);

			for(UINT_32 iI = 0; iI < vFunctions.size(); ++iI)
			{
				STLW::string sFile = CheckFile(vLibexecDirs, vFunctions[iI].library, &oLogger);
				// Check error
				if (sFile.empty()) { return VIEW_ERROR; }
				// Create object
				vFunctions[iI].udf = pLoader -> GetObject(sFile.c_str(), vFunctions[iI].name.c_str());
				// Initialize object
				vFunctions[iI].udf -> InitHandler(vFunctions[iI].config);
				// Register handler
				pSyscallFactory -> RegisterHandler(vFunctions[iI].udf);
			}
		}

		// Get list of template dirs
		oList = oConfiguration.GetCDT("TemplateIncludeDirs");
		if (oList.GetType() == CDT::HASH_VAL)
		{
			oList = oList.GetCDT("TemplateIncludeDir");
			if (oList.GetType() == CDT::STRING_VAL)
			{
				vIncludeDirs.push_back(oList.GetString());
			}
			else if (oList.GetType() == CDT::ARRAY_VAL)
			{
				for(UINT_32 iPos = 0; iPos < oList.Size(); ++iPos)
				{
					vIncludeDirs.push_back(oList[iPos].GetString());
				}
			}
			// Nothing to do
			else if (oList.GetType() == CDT::UNDEF) { ;; }
			// Error
			else
			{
				oLogger.Error("Invalid type of \"IncludeDir\" directive.");
				DEBUG_HELPER_MESSAGE("Invalid type of \"IncludeDir\" directive.");
				return VIEW_ERROR;
			}
		}

		// Create virtual machine
		pVM = new VM(pSyscallFactory, iArgStackSize, iCodeStackSize, iStepsLimit);

		iRC = VIEW_OK;
	}
	catch (CTPPUnixException      & e)
	{
		STLW::string sError(e.what());
		sError += ": ";
		sError += strerror(e.ErrNo());
		oCTPPError = CTPPError("", sError, CTPP_VM_ERROR | CTPP_UNIX_ERROR, 0, 0, 0);
	}
	catch (CDTRangeException      & e) { oCTPPError = CTPPError("", e.what(), CTPP_DATA_ERROR | CTPP_RANGE_ERROR,         0, 0, 0); }
	catch (CDTAccessException     & e) { oCTPPError = CTPPError("", e.what(), CTPP_DATA_ERROR | CTPP_ACCESS_ERROR,        0, 0, 0); }
	catch (CDTTypeCastException   & e) { oCTPPError = CTPPError("", e.what(), CTPP_DATA_ERROR | CTPP_TYPE_CAST_ERROR,     0, 0, 0); }
	catch (CTPPLogicError         & e) { oCTPPError = CTPPError("", e.what(), CTPP_DATA_ERROR | CTPP_LOGIC_ERROR,         0, 0, 0); }
	catch (CTPPException          & e) { oCTPPError = CTPPError("", e.what(), CTPP_DATA_ERROR | CTPP_UNKNOWN_ERROR,       0, 0, 0); }
	catch (std::exception         & e) { oCTPPError = CTPPError("", e.what(), CTPP_DATA_ERROR | STL_UNKNOWN_ERROR,        0, 0, 0); }
	catch (...)                        { oCTPPError = CTPPError("", "Unknown Error", CTPP_DATA_ERROR | STL_UNKNOWN_ERROR, 0, 0, 0); }

	// Error occured
	if (iRC == VIEW_ERROR)
	{
		CHAR_8 szBuffer[1024 + 1];
		snprintf(szBuffer, 1024, ": (error code 0x%08X);", oCTPPError.error_code);
		oCTPPError.error_descr.append(szBuffer);
		DEBUG_HELPER_MESSAGE(oCTPPError.error_descr.c_str());

		oLogger.Emerg(oCTPPError.error_descr.c_str());
	}

return iRC;
}

//
// Write HTTP response
//
INT_32 CTPPView::WriteResponse(CTPP::CDT         & oData,
                               ASResponse        & oResponse,
                               ASResponseWriter  & oResponseWriter,
                               ASPool            & oGlobalPool,
                               ASPool            & oVhostPool,
                               ASPool            & oRequestPool,
                               CTPP::CDT         & oConfiguration,
                               ASObject          * pContextData,
                               CTPP::CDT         & oIMC,
                               ASLogger          & oLogger)
{
	using namespace CTPP;
	DEBUG_HELPER(&oLogger, "WriteResponse");

	CTPPError oCTPPError;
	INT_32 iRC  = VIEW_ERROR;
	UINT_32 iIP = 0;
	try
	{
		DEBUG_HELPER_MESSAGE("Write response headers");
		// Write response headers
		oResponseWriter.WriteHeader(oResponse);

		DEBUG_HELPER_MESSAGE("Write response body");

		// Nothing to do?
		STLW::string sTemplateName = oIMC["template"].GetString();
		if (sTemplateName.empty())
		{
			oLogger.Error("Template name not set (IMC key \"template\" is empty or not set)");
			return 0;
		}

		ASTemplate * pTemplate = NULL;
		// Get template from pool
		STLW::map<STLW::string, ASTemplate *>::const_iterator itmTemplates = mTemplates.find(sTemplateName);
		// Try to load template, if not found
		if (itmTemplates != mTemplates.end()) { pTemplate = itmTemplates -> second; }
		else
		{
			pTemplate = ParseTemplate(sTemplateName, oLogger);
		}

		// Check error
		if (pTemplate == NULL) { return -1; }

		// Get bytecode
		const VMMemoryCore * pVMMemoryCore = pTemplate -> GetBytecode();
		if (pVMMemoryCore == NULL)
		{
			oLogger.Emerg("Cannot get bytecode object from template `%s`", sTemplateName.c_str());
			return -1;
		}

//		struct timeval sTimeValLocBegin;
//		gettimeofday(&sTimeValLocBegin, NULL);

		ASCTPPLogger oCTPPLogger(&oLogger);
		// Run virtual machine and write output to the output collector
		pVM -> Init(pVMMemoryCore, oResponseWriter.GetBodyOutputCollector(), &oCTPPLogger);
		pVM -> Run(pVMMemoryCore, oResponseWriter.GetBodyOutputCollector(), iIP, oData, &oCTPPLogger);

//		struct timeval sTimeValLocEnd;
//		gettimeofday(&sTimeValLocEnd, NULL);

//		fprintf(stderr, "Execution of bytecode completed in %f seconds.\n", (1.0 * (sTimeValLocEnd.tv_sec - sTimeValLocBegin.tv_sec) + 1.0 * (sTimeValLocEnd.tv_usec - sTimeValLocBegin.tv_usec) / 1000000));

		iRC = VIEW_OK;
	}
	catch (ZeroDivision           & e) { oCTPPError = CTPPError(e.GetSourceName(), e.what(), CTPP_VM_ERROR | CTPP_ZERO_DIVISION_ERROR,           VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP()); }
	catch (ExecutionLimitReached  & e) { oCTPPError = CTPPError(e.GetSourceName(), e.what(), CTPP_VM_ERROR | CTPP_EXECUTION_LIMIT_REACHED_ERROR, VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP()); }
	catch (CodeSegmentOverrun     & e) { oCTPPError = CTPPError(e.GetSourceName(), e.what(), CTPP_VM_ERROR | CTPP_CODE_SEGMENT_OVERRUN_ERROR,    VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP()); }
	catch (InvalidSyscall         & e)
	{
		if (e.GetIP() != 0)
		{
			oCTPPError = CTPPError(e.GetSourceName(), e.what(), CTPP_VM_ERROR | CTPP_INVALID_SYSCALL_ERROR,         VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP());
		}
		else
		{
			oCTPPError = CTPPError(e.GetSourceName(), std::string("Unsupported syscall: \"") + e.what() + "\"", CTPP_VM_ERROR | CTPP_INVALID_SYSCALL_ERROR,         VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP());
		}
	}
	catch (IllegalOpcode          & e) { oCTPPError = CTPPError(e.GetSourceName(), e.what(), CTPP_VM_ERROR | CTPP_ILLEGAL_OPCODE_ERROR,          VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP()); }
	catch (StackOverflow          & e) { oCTPPError = CTPPError(e.GetSourceName(), e.what(), CTPP_VM_ERROR | CTPP_STACK_OVERFLOW_ERROR,          VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP()); }
	catch (StackUnderflow         & e) { oCTPPError = CTPPError(e.GetSourceName(), e.what(), CTPP_VM_ERROR | CTPP_STACK_UNDERFLOW_ERROR,         VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP()); }
	catch (VMException            & e) { oCTPPError = CTPPError(e.GetSourceName(), e.what(), CTPP_VM_ERROR | CTPP_VM_GENERIC_ERROR,              VMDebugInfo(e.GetDebugInfo()).GetLine(), VMDebugInfo(e.GetDebugInfo()).GetLinePos(), e.GetIP()); }
	catch (CTPPUnixException      & e) { oCTPPError = CTPPError("", e.what(), CTPP_VM_ERROR | CTPP_UNIX_ERROR,                    0, 0, iIP); }
	catch (CDTRangeException      & e) { oCTPPError = CTPPError("", e.what(), CTPP_VM_ERROR | CTPP_RANGE_ERROR,                   0, 0, iIP); }
	catch (CDTAccessException     & e) { oCTPPError = CTPPError("", e.what(), CTPP_VM_ERROR | CTPP_ACCESS_ERROR,                  0, 0, iIP); }
	catch (CDTTypeCastException   & e) { oCTPPError = CTPPError("", e.what(), CTPP_VM_ERROR | CTPP_TYPE_CAST_ERROR,               0, 0, iIP); }
	catch (CTPPLogicError         & e) { oCTPPError = CTPPError("", e.what(), CTPP_VM_ERROR | CTPP_LOGIC_ERROR,                   0, 0, iIP); }
	catch (CTPPException          & e) { oCTPPError = CTPPError("", e.what(), CTPP_VM_ERROR | CTPP_UNKNOWN_ERROR,                 0, 0, iIP); }
	catch (std::exception         & e) { oCTPPError = CTPPError("", e.what(), CTPP_VM_ERROR | STL_UNKNOWN_ERROR,                  0, 0, iIP); }
	catch (...)                        { oCTPPError = CTPPError("", "Unknown Error", CTPP_VM_ERROR | STL_UNKNOWN_ERROR,           0, 0, iIP); }

	// Error occured
	if (iRC == VIEW_ERROR)
	{
		CHAR_8 szBuffer[1024 + 1];
		snprintf(szBuffer, 1024, ": (error code 0x%08X); IP: 0x%08X", oCTPPError.error_code, oCTPPError.ip);
		oCTPPError.error_descr.append(szBuffer);
		if (oCTPPError.line != 0 && oCTPPError.pos != 0)
		{
			oCTPPError.error_descr.append("; file \"");
			oCTPPError.error_descr.append(oCTPPError.template_name);
			snprintf(szBuffer, 1024, "\", line %d, pos %d", oCTPPError.line, oCTPPError.pos);
			oCTPPError.error_descr.append(szBuffer);
		}
		DEBUG_HELPER_MESSAGE(oCTPPError.error_descr.c_str());
		oLogger.Emerg(oCTPPError.error_descr.c_str());
	}

return iRC;
}

//
//
//
ASTemplate * CTPPView::ParseTemplate(const STLW::string  & sTemplateName,
                                     ASLogger            & oLogger)
{

	try
	{
		ASTemplate * pTemplate = new ASTemplate(sTemplateName, ASTemplate::SOURCE, vIncludeDirs);
		mTemplates[sTemplateName] = pTemplate;
		return pTemplate;
	}
	catch(STLW::exception & e)
	{
		oLogger.Emerg("Cannot load template `%s`: %s", sTemplateName.c_str(), e.what());
		return NULL;
	}

// Make compiler happy
return NULL;
}

//
// A destructor
//
CTPPView::~CTPPView() throw()
{
	STLW::map<STLW::string, ASTemplate *>::iterator itmTemplates = mTemplates.begin();
	while (itmTemplates != mTemplates.end())
	{
		delete itmTemplates -> second;
		++itmTemplates;
	}

	if (pSyscallFactory != NULL)
	{
		for(UINT_32 iI = 0; iI < vFunctions.size(); ++iI)
		{
			CTPP::SyscallHandler * pUDF = vFunctions[iI].udf;
			if (pUDF != NULL)
			{
				// Initialize object
				vFunctions[iI].udf -> DestroyHandler(vFunctions[iI].config);

				pSyscallFactory -> RemoveHandler(pUDF -> GetName());
				delete(pUDF);
			}
		}
		// Destroy standard library
		CTPP::STDLibInitializer::DestroyLibrary(*pSyscallFactory);
		delete pSyscallFactory;
	}

	if (pVM     != NULL) { delete pVM; }
	if (pLoader != NULL) { delete pLoader; }
}

    // End.
} // namespace TEST_NS

