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
 *      cas-xt.cpp
 *
 * $CAS$
 */
#include "ASGetOpt.hpp"
#include "STLString.hpp"

#include <CDT.hpp>
#include <CTPP2Exception.hpp>
#include <CTPP2FileLogger.hpp>
#include <CTPP2FileOutputCollector.hpp>
#include <CTPP2FileSourceLoader.hpp>
#include <CTPP2Parser.hpp>
#include <CTPP2ParserException.hpp>
#include <CTPP2SyscallFactory.hpp>
#include <CTPP2VM.hpp>
#include <CTPP2VMDumper.hpp>
#include <CTPP2VMException.hpp>
#include <CTPP2VMMemoryCore.hpp>
#include <CTPP2VMOpcodeCollector.hpp>
#include <CTPP2VMStackException.hpp>
#include <CTPP2VMSTDLib.hpp>

#include <fstream>

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

using namespace CTPP;
using namespace CAS;

//
// Options
//
const Options oOptions[] = {
                             { 't', "type",         C_HAS_ARG }, // Module type
                             { 'g', "generate",     C_NO_ARG  }, // Generate example module
                             { 'n', "name",         C_HAS_ARG }, // Set module name
                             { 's', "template-dir", C_HAS_ARG }, // Set template directory
                             { 'o', "output-dir",   C_HAS_ARG }, // Set output directory
                             { 0 }
                           };


void Usage(CHAR_P szProgname)
{
	fprintf(stderr, "Usage: %s -t <module type> -g -n <module name>\n", szProgname);
}


INT_32 ProcessTemplate(const STLW::string & sTemplateName, CDT & oData, const STLW::string & sFileName)
{
	FILE * FOutput = fopen(sFileName.c_str(), "wb");
	if (FOutput == NULL)
	{
		fprintf(stderr, "ERROR: cannot open file \"%s\" for writing: %s\n", sFileName.c_str(), strerror(errno));
		return EX_SOFTWARE;
	}

	INT_32 iRetCode = EX_SOFTWARE;

	// Output
	FileOutputCollector oOutputCollector(FOutput);

	// Initialize Standard CTPP library
	SyscallFactory oSyscallFactory(1024);
	// Load standard library
	STDLibInitializer::InitLibrary(oSyscallFactory);

	try
	{

		VMOpcodeCollector  oVMOpcodeCollector;
		StaticText         oSyscalls;
		StaticData         oStaticData;
		StaticText         oStaticText;
		HashTable          oHashTable;
		CTPP2Compiler oCompiler(oVMOpcodeCollector, oSyscalls, oStaticData, oStaticText, oHashTable);

		// Load template
		CTPP2FileSourceLoader oSourceLoader;
		oSourceLoader.LoadTemplate(sTemplateName.c_str());

		// Create template parser
		CTPP2Parser oCTPP2Parser(&oSourceLoader, &oCompiler, sTemplateName);

		// Compile template
		oCTPP2Parser.Compile();

		// Get program core
		UINT_32 iCodeSize = 0;
		const VMInstruction * oVMInstruction = oVMOpcodeCollector.GetCode(iCodeSize);
		// Dump program
		VMDumper oDumper(iCodeSize, oVMInstruction, oSyscalls, oStaticData, oStaticText, oHashTable);
		UINT_32 iSize = 0;
		const VMExecutable * aProgramCore = oDumper.GetExecutable(iSize);

		// Get program core
		const VMMemoryCore oVMMemoryCore(aProgramCore);

		// Run program
		VM oVM(&oSyscallFactory, 10240, 10240, 1024 * 1024);
		UINT_32 iIP = 0;

		FileLogger oLogger(stderr);
		oVM.Init(&oVMMemoryCore, &oOutputCollector, &oLogger);
		oVM.Run(&oVMMemoryCore, &oOutputCollector, iIP, oData, &oLogger);

		iRetCode = EX_OK;
	}
	catch(CTPPParserSyntaxError       & e) { fprintf(stderr, "ERROR: At line %d, pos. %d: %s\n", e.GetLine(), e.GetLinePos(), e.what()); }
	catch(CTPPParserOperatorsMismatch & e) { fprintf(stderr, "ERROR: At line %d, pos. %d: expected %s, but found </%s>\n", e.GetLine(), e.GetLinePos(), e.Expected(), e.Found()); }

	// CDT
	catch(CDTTypeCastException  & e) { fprintf(stderr, "ERROR: Type Cast %s\n", e.what());                                    }
	catch(CDTAccessException    & e) { fprintf(stderr, "ERROR: Array index out of bounds: %s\n", e.what());                   }

	// Virtual machine
	catch(IllegalOpcode         & e) { fprintf(stderr, "ERROR: Illegal opcode 0x%08X at 0x%08X\n", e.GetOpcode(), e.GetIP()); }
	catch(InvalidSyscall        & e)
	{
		if (e.GetIP() != 0)
		{
			VMDebugInfo oVMDebugInfo(e.GetDebugInfo());
			fprintf(stderr, "ERROR: %s at 0x%08X (Template file \"%s\", Line %d, Pos: %d)\n", e.what(), e.GetIP(), e.GetSourceName(), oVMDebugInfo.GetLine(), oVMDebugInfo.GetLinePos());
		}
		else
		{
			fprintf(stderr, "Unsupported syscall: \"%s\"\n", e.what());
		}
	}
	catch(CodeSegmentOverrun    & e) { fprintf(stderr, "ERROR: %s at 0x%08X\n", e.what(),  e.GetIP());                        }
	catch(StackOverflow         & e) { fprintf(stderr, "ERROR: Stack overflow at 0x%08X\n", e.GetIP());                       }
	catch(StackUnderflow        & e) { fprintf(stderr, "ERROR: Stack underflow at 0x%08X\n", e.GetIP());                      }
	catch(ExecutionLimitReached & e) { fprintf(stderr, "ERROR: Execution limit of %d step(s) reached at 0x%08X\n", 1024 & 1024, e.GetIP()); }
	catch(VMException           & e) { fprintf(stderr, "ERROR: VM generic exception: %s at 0x%08X\n", e.what(), e.GetIP()); }

	// CTPP
	catch(CTPPLogicError        & e) { fprintf(stderr, "ERROR: %s\n", e.what());                                              }
	catch(CTPPUnixException     & e) { fprintf(stderr, "ERROR: I/O in %s: %s\n", e.what(), strerror(e.ErrNo()));              }
	catch(CTPPException         & e) { fprintf(stderr, "ERROR: CTPP Generic exception: %s\n", e.what());                      }

	catch(...)                       { fprintf(stderr, "ERROR: Bad thing happened.\n"); }

	// Destroy standard library
	STDLibInitializer::DestroyLibrary(oSyscallFactory);

return iRetCode;
}

//
// Get parameters
//
INT_32 GetParams(const INT_32    iArgc,
                 CHAR_P        * aArgv,
                 STLW::string  & sModuleName,
                 STLW::string  & sModuleType,
                 UINT_32       & iGenerate,
                 STLW::string  & sTemplateDir,
                 STLW::string  & sOutputDir)
{
	iGenerate = 0;

	// Usage
	if (iArgc == 1) { Usage(aArgv[0]); return EX_USAGE; }

	// Check environment
	CCHAR_P szTemplateDir = getenv("CAS_TEMPLATE_DIR");
	if (szTemplateDir != NULL && *szTemplateDir != '\0') { sTemplateDir = szTemplateDir; }

	UINT_32 iArgNum = 1;
	CCHAR_P szArgValue;
	for(;;)
	{
		INT_32 iOption = GetOpt(oOptions, iArgNum, iArgc, aArgv, szArgValue);
		if (iOption == -1) { break; }
		switch (iOption)
		{
			case 't':
				sModuleType = szArgValue;
				break;
			case 'g':
				iGenerate = 1;
				break;
			case 'n':
				sModuleName = szArgValue;
				break;
			case 's':
				if (szArgValue != NULL && *szArgValue != '\0') { sTemplateDir = szArgValue; }
				break;
			case 'o':
				// Get dir
				if (szArgValue != NULL && *szArgValue != '\0') { sOutputDir = szArgValue; }
				break;
			case '?':
				fprintf(stderr, "Unknown option `%s` or option with missed argument\n", aArgv[iArgNum]);
				return EX_CONFIG;
				break;
		}
	}

	if (iGenerate == 1)
	{
		if (sModuleName.empty()) { fprintf(stderr, "Need module name (-n <module name>)\n"); return EX_CONFIG; }
		if (sModuleType.empty()) { fprintf(stderr, "Need module type (-t <module type>)\n"); return EX_CONFIG; }
	}

return EX_OK;
}

//
// Copy file
//
INT_32 CopyFile(const STLW::string & sSourceDir, const STLW::string & sSourceFile, const STLW::string & sDestinationDir)
{
	STLW::string sTMP = sSourceDir + "/" + sSourceFile;
	std::ifstream ifs(sTMP.c_str(), std::ios::binary);

	sTMP = sDestinationDir + "/" + sSourceFile;
	std::ofstream ofs(sTMP.c_str(), std::ios::binary);

	ofs << ifs.rdbuf();

	ifs.close();
	ofs.close();

return 0;
}

//
// Copy all *.cmake files to specified directory
//
INT_32 CopyDirectory(const STLW::string & sTemplateDir, const STLW::string & sCmakeFilesDir)
{
	DIR * oDir = opendir(sTemplateDir.c_str());
	if (oDir == NULL) { return -1; }

	for(;;)
	{
		struct dirent * pDirEntry = readdir(oDir);
		if (pDirEntry == NULL) { closedir(oDir); return 0; }

		// Copy only "*.cmake"
		if (strstr(pDirEntry -> d_name, ".cmake") != NULL)
		{
			fprintf(stdout, "    Copying [FILE] from %s/%s to %s\n", sTemplateDir.c_str(), pDirEntry -> d_name, sCmakeFilesDir.c_str());
			if (CopyFile(sTemplateDir, pDirEntry -> d_name, sCmakeFilesDir) == -1) { break; }
		}
	}
	closedir(oDir);
return -1;
}

int main(int argc, char ** argv)
{
	STLW::string  sModuleName;
	STLW::string  sModuleType;
	UINT_32       iGenerate;
	STLW::string  sTemplateDir = CAS_SHAREDIR; sTemplateDir.append("/xt");
	STLW::string  sOutputDir;

	INT_32 iRetCode = GetParams(argc, argv, sModuleName, sModuleType, iGenerate, sTemplateDir, sOutputDir);
	if (iRetCode != EX_OK) { return iRetCode; }

	if (iGenerate == 1)
	{
		fprintf(stdout, "Using templates from directory \"%s\"\n", sTemplateDir.c_str());
		fprintf(stdout, "Output directory is \"%s\"\n", sOutputDir.c_str());

		STLW::string sModuleLCName(sModuleName);
		// Check module name
		for (UINT_32 iPos = 0; iPos < sModuleName.size(); ++iPos)
		{
			UCHAR_8 uCH = sModuleName[iPos];
			if ((uCH >= 'a' && uCH <= 'z') ||
			    (uCH >= 'A' && uCH <= 'Z') ||
			    (uCH >= '0' && uCH <= '9'))
			{
				if (iPos == 0 && (uCH >= '0' && uCH <= '9'))
				{
					fprintf(stderr, "Module name \"%s\" shoud not start from digit\n", sModuleName.c_str());
					return EX_CONFIG;
				}
			}
			else
			{
				fprintf(stderr, "Invalid module name \"%s\"\n", sModuleName.c_str());
				return EX_CONFIG;
			}
			sModuleLCName[iPos] = uCH | 0x20;
		}

		STLW::string sTemplate(sTemplateDir);
		if (sTemplate.size() && sTemplate[sTemplate.size() - 1] != '/') { sTemplate.append("/"); }

		STLW::string sCMakeListsTemplate(sTemplate);
		sCMakeListsTemplate.append("CMakeLists.tmpl");

		// Check module type
		// Object
		if      (strcasecmp("object",  sModuleType.c_str()) == 0) { sTemplate.append("ObjectTemplate.tmpl"); }
		// Module
		else if (strcasecmp("module",  sModuleType.c_str()) == 0) { sTemplate.append("ModuleTemplate.tmpl");  }
		// View
		else if (strcasecmp("handler", sModuleType.c_str()) == 0) { sTemplate.append("HandlerTemplate.tmpl"); }
		// Handler
		else if (strcasecmp("view",    sModuleType.c_str()) == 0) { sTemplate.append("ViewTemplate.tmpl");    }
		// Pre-request Handler
		else if (strcasecmp("pre-request-handler", sModuleType.c_str()) == 0) { sTemplate.append("PreRequestHandlerTemplate.tmpl"); }
		else
		{
			fprintf(stderr, "Invalid module type \"%s\"\n", sModuleType.c_str());
			return EX_CONFIG;
		}

		// Check access to template file
		if (access(sTemplate.c_str(), R_OK) == -1)
		{
			fprintf(stderr, "ERROR: try to load \"%s\": %s\n", sTemplate.c_str(), strerror(errno));
			return EX_SOFTWARE;
		}

		// Check access to template file
		if (access(sCMakeListsTemplate.c_str(), R_OK) == -1)
		{
			fprintf(stderr, "ERROR: try to load \"%s\": %s\n", sCMakeListsTemplate.c_str(), strerror(errno));
			return EX_SOFTWARE;
		}

		if (sOutputDir.size() && sOutputDir[sOutputDir.size() - 1] != '/') { sOutputDir.append("/"); }
		sOutputDir.append(sModuleName);

		fprintf(stdout, "Creating [DIR]  %s\n", sOutputDir.c_str());
		if (mkdir(sOutputDir.c_str(), 0755) == -1)
		{
			fprintf(stderr, "ERROR: try to create directory \"%s\": %s\n", sOutputDir.c_str(), strerror(errno));
			return EX_SOFTWARE;
		}

		STLW::string sIncludeDir(sOutputDir); sIncludeDir.append("/include");
		fprintf(stdout, "Creating [DIR]  %s\n", sIncludeDir.c_str());
		if (mkdir(sIncludeDir.c_str(), 0755) == -1)
		{
			fprintf(stderr, "ERROR: try to create directory \"%s\": %s\n", sIncludeDir.c_str(), strerror(errno));
			return EX_SOFTWARE;
		}

		STLW::string sSourceDir(sOutputDir); sSourceDir.append("/src");
		fprintf(stdout, "Creating [DIR]  %s\n", sSourceDir.c_str());
		if (mkdir(sSourceDir.c_str(), 0755) == -1)
		{
			fprintf(stderr, "ERROR: try to create directory \"%s\": %s\n", sSourceDir.c_str(), strerror(errno));
			return EX_SOFTWARE;
		}

		STLW::string sCmakeFilesDir(sOutputDir); sCmakeFilesDir.append("/cmake");
		fprintf(stdout, "Creating [DIR]  %s\n", sCmakeFilesDir.c_str());
		if (mkdir(sCmakeFilesDir.c_str(), 0755) == -1)
		{
			fprintf(stderr, "ERROR: try to create directory \"%s\": %s\n", sCmakeFilesDir.c_str(), strerror(errno));
			return EX_SOFTWARE;
		}

		// Copy files
		CopyDirectory(sTemplateDir, sCmakeFilesDir);

		// Fill data
		CDT oData(CDT::HASH_VAL);
		oData["module_name"]      = sModuleName;
		oData["module_lc_name"]   = sModuleLCName;
		oData["xt_filepath"]      = sTemplateDir;
		oData["cmake_includedir"] = "cmake";

		// Write C++ source file
		sOutputDir.append("/");
		STLW::string sOutputFileName(sOutputDir);
		sOutputFileName.append("src/");
		sOutputFileName.append(sModuleName);
		sOutputFileName.append(".cpp");
		fprintf(stdout, "Creating [FILE] %s\n", sOutputFileName.c_str());

		// Read & process template
		iRetCode = ProcessTemplate(sTemplate, oData, sOutputFileName);
		if (iRetCode != EX_OK) { return iRetCode; }

		// Write CmakeLists.txt
		sOutputFileName.assign(sOutputDir);
		sOutputFileName.append("CMakeLists.txt");
		fprintf(stdout, "Creating [FILE] %s\n", sOutputFileName.c_str());

		// Read & process template
		iRetCode = ProcessTemplate(sCMakeListsTemplate, oData, sOutputFileName);
		if (iRetCode != EX_OK) { return iRetCode; }
	}

return iRetCode;
}
// End.

