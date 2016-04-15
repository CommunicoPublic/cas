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
 *      ASGlobalConfigHandler.cpp
 *o
 * $CAS$
 */

// C++ Includes
#include "ASGlobalConfigHandler.hpp"

#include "ASConfigErrorHandler.hpp"
#include "ASGenericConfigHandler.hpp"
#include "ASGlobalConfig.hpp"
#include "ASException.hpp"
#include "ASXMLParser.hpp"

#include <sys/param.h>
#include <errno.h>
#include <fcntl.h>
#include <glob.h>
#include <strings.h>
#include <unistd.h>

#define C_FILE_NOT_FOUND -1
#define C_FILE_NO_ACCESS -2
#define C_FILE_FOUND      0

namespace CAS // C++ Application Server
{
/*
<?xml version="1.0" ?>
<CASConfig version="3.8">
	<LibexecDirs>
		<!--
		  Directory for CAS modules
		-->

		<!-- For Linux -->
		<LibexecDir>/usr/lib/cppas</LibexecDir>

		<!-- For FreeBSD -->
		<LibexecDir>/usr/local/libexec/cppas</LibexecDir>

		<!-- For Solaris -->
		<LibexecDir>/opt/SUPcppas/libexec</LibexecDir>

	</LibexecDirs>

	<!-- List of modules -->
	<Modules>
		<!--
		  Information of CAS configuration.
		  Should use for system administrators and heart-beat tests
		-->
		<Module classname="CASConfigInformer" libname="mod_cppas_info.so"/>

		<!--
		  Example informer.
		-->
		<Module classname="CASTestInformer" libname="mod_cppas_test.so"/>

		<!--
		  Example template selector.
		  Is used if <Template type="regexp">
		-->
		<Module classname="CASTestTemplateSelector" libname="mod_cppas_tmpl.so"/>
	</Modules>

</CASConfig>
*/

//
// Constructor
//
ASGlobalConfigHandler::ASGlobalConfigHandler(ASGlobalConfig & oIGLobalConfig): oGlobalConfig(oIGLobalConfig), pGenericHandler(NULL)
{
	;;
}

//
// Constructor
//
ASGlobalConfigHandler::ASGlobalConfigHandler(ASGlobalConfig                    & oIGLobalConfig,
                                             const STLW::vector<STLW::string>  & vIIncludeDirs): oGlobalConfig(oIGLobalConfig),
                                                                                                 vIncludeDirs(vIIncludeDirs),
                                                                                                 pGenericHandler(NULL)
{
	;;
}

//
// Get error description
//
STLW::string ASGlobalConfigHandler::GetError() { return sError; }

//
// A destructor
//
ASGlobalConfigHandler::~ASGlobalConfigHandler() throw()
{
	;;
}

//
// Include wildcard directive
//
INT_32 ASGlobalConfigHandler::IncludeWildcard(CCHAR_P          sWildcard,
                                              CCHAR_P          sElement,
                                              const UINT_32    iLevel,
                                              const UINT_32    iLine,
                                              const UINT_32    iColumn)
{
	CHAR_P szCurrentDir = (CHAR_P)malloc(MAXPATHLEN + 1);
	if (getcwd(szCurrentDir, MAXPATHLEN) == NULL)
	{
		sError.assign("getcwd failed");
		return -1;
	}

	STLW::set<STLW::string>  oFiles;

	bool bFailed = false;
	STLW::vector<STLW::string>::const_iterator itvIncludeDirs = vIncludeDirs.begin();
	while (itvIncludeDirs != vIncludeDirs.end())
	{
		STLW::string sTMP = *itvIncludeDirs;
		if (sTMP.size() != 0)
		{
			if (sTMP[0] != '/') { sTMP = STLW::string(szCurrentDir) + "/" + sTMP; }
			if (sTMP[sTMP.size() - 1] != '/') { sTMP.append("/"); }
			sTMP.append(sWildcard);

			// Find files
			if (FindFiles(sTMP.c_str(), oFiles) == -1)
			{
				bFailed = true;
				break;
			}
		}

		++itvIncludeDirs;
	}
	free(szCurrentDir);

	if (bFailed) { return -1; }

	STLW::set<STLW::string>::const_iterator itoFiles = oFiles.begin();
	while (itoFiles != oFiles.end())
	{
		// Try to open included file
		FILE * F = fopen(itoFiles -> c_str(), "rb");
		// Never happenes, but let it be
		if (F == NULL)
		{
			STLW::string sTMP("Cannot open configuration file \"");
			sTMP.append(*itoFiles);
			sTMP.append("\" for reading.");
			throw UnixException(sTMP.c_str(), errno);
		}

		try
		{
			// Create global config parser object
			ASGlobalConfigHandler oHandler(oGlobalConfig, vIncludeDirs);

			// Create generic XML parser
			ASXMLParser oParser(&oHandler, iLevel);

			// Parse configuration
			if (oParser.ParseFile(F) == -1)
			{
				fclose(F);

				STLW::string sTMP("In included file \"");
				sTMP.append(*itoFiles);
				sTMP.append("\": ");
				sTMP.append(oHandler.GetError().c_str());
				throw LogicError(sTMP.c_str());
			}

			fclose(F);
		}
		catch(...)
		{
			fclose(F);
			throw;
		}

		++itoFiles;
	}

return 0;
}

//
// Find files by mask
//
INT_32 ASGlobalConfigHandler::FindFiles(CCHAR_P                    sWildcard,
                                        STLW::set<STLW::string>  & oFiles)
{
	glob_t oGlob;
	const INT_32 iRC = glob(sWildcard, GLOB_NOSORT, NULL, &oGlob);
	if (iRC != 0 && iRC != GLOB_NOMATCH) { return -1; }

#ifdef GLOB_NOMATCH
	const UINT_32 iElts = oGlob.gl_pathc;
#else
	const UINT_32 iElts = oGlob.gl_matchc;
#endif
	for (UINT_32 iPos = 0; iPos < iElts; ++iPos)
	{
		oFiles.insert(oGlob.gl_pathv[iPos]);
	}

	globfree(&oGlob);

return 0;
}

//
// Include file directive
//
INT_32 ASGlobalConfigHandler::IncludeFile(CCHAR_P          sFileName,
                                          CCHAR_P          sElement,
                                          const UINT_32    iLevel,
                                          const UINT_32    iLine,
                                          const UINT_32    iColumn)
{
	STLW::string sRealName;

	INT_32 iFileFound = C_FILE_NOT_FOUND;
	STLW::string sErrorDescr;
	if (sFileName[0] == '/')
	{
		// Check file
		if (access(sFileName, R_OK) != 0)
		{
			STLW::string sTMP("Cannot open file \"");
			sTMP.append(sFileName);
			sTMP.append("\": ");
			sTMP.append(strerror(errno));
			sError = ASConfigErrorHandler::TagProcessingError(sElement, sTMP, iLine, iColumn);
			return -1;
		}

		iFileFound = C_FILE_FOUND;
		sRealName  = sFileName;
	}
	else
	{
		CHAR_P szCurrentDir = (CHAR_P)malloc(MAXPATHLEN + 1);
		if (getcwd(szCurrentDir, MAXPATHLEN) == NULL)
		{
			sErrorDescr.assign("getcwd failed");
			return -1;
		}

		STLW::vector<STLW::string>::const_iterator itvIncludeDirs = vIncludeDirs.begin();
		while (itvIncludeDirs != vIncludeDirs.end())
		{
			STLW::string sTMP = *itvIncludeDirs;
			if (sTMP.size() != 0)
			{
				if (sTMP[0] != '/') { sTMP = STLW::string(szCurrentDir) + "/" + sTMP; }
				if (sTMP[sTMP.size() - 1] != '/') { sTMP.append("/"); }

				sTMP.append(sFileName);
				if (access(sTMP.c_str(), R_OK) == -1)
				{
					if (errno == EACCES)
					{
						if (!sErrorDescr.empty()) { sErrorDescr.append(", "); }
						sErrorDescr.append("\"");
						sErrorDescr.append(sTMP);
						sErrorDescr.append("\"");
						iFileFound = C_FILE_NO_ACCESS;
					}
				}
				else
				{
					sRealName  = sTMP;
					iFileFound = C_FILE_FOUND;
					break;
				}
			}

			++itvIncludeDirs;
		}
		free(szCurrentDir);
	}

	// File not found anywhere
	if (iFileFound == C_FILE_NOT_FOUND)
	{
		STLW::string sSearchPath("File \"");
		sSearchPath.append(sFileName).append("\" not found; search path: \"");

		STLW::vector<STLW::string>::const_iterator itvIncludeDirs = vIncludeDirs.begin();
		while (itvIncludeDirs != vIncludeDirs.end())
		{
			sSearchPath += *itvIncludeDirs;
			++itvIncludeDirs;
			if (itvIncludeDirs != vIncludeDirs.end()) { sSearchPath += "\", \""; }
		}
		sError = ASConfigErrorHandler::TagProcessingError(sElement, sSearchPath, iLine, iColumn);
		return -1;
	}

	// Invalid permissions on files
	if (iFileFound == C_FILE_NO_ACCESS)
	{
		STLW::string sSearchPath("Invalid permissions on file(s) or file pathes: ");
		sSearchPath.append(sErrorDescr);
		sError = ASConfigErrorHandler::TagProcessingError(sElement, sSearchPath, iLine, iColumn);
		return -1;
	}

	// Try to open included file
	FILE * F = fopen(sRealName.c_str(), "rb");
	// Never happenes, but let it be
	if (F == NULL)
	{
		STLW::string sTMP("Cannot open configuration file \"");
		sTMP.append(sRealName);
		sTMP.append("\" for reading.");
		throw UnixException(sTMP.c_str(), errno);
	}

	try
	{
		// Create global config parser object
		ASGlobalConfigHandler oHandler(oGlobalConfig, vIncludeDirs);

		// Create generic XML parser
		ASXMLParser oParser(&oHandler, iLevel);

		// Parse configuration
		if (oParser.ParseFile(F) == -1)
		{
			fclose(F);

			STLW::string sTMP("In included file \"");
			sTMP.append(sRealName);
			sTMP.append("\": ");
			sTMP.append(oHandler.GetError().c_str());
			throw LogicError(sTMP.c_str());
		}

		fclose(F);
	}
	catch(...)
	{
		fclose(F);
		throw;
	}

return 0;
}

//
// Start of XML Element
//
INT_32 ASGlobalConfigHandler::StartElement(CCHAR_P          sElement,
                                           const UINT_32    iLevel,
                                           const XMLAttr  * aAttr,
                                           const UINT_32    iAttributes,
                                           const UINT_32    iLine,
                                           const UINT_32    iColumn)
{
	// Include directive.
	if (iLevel < 3 && strcasecmp("Include", sElement) == 0)
	{
		// Get file name
		for (UINT_32 iI = 0; iI < iAttributes; ++iI)
		{
			if (strcasecmp("file", aAttr[iI].name) == 0)
			{
				// Check empty file
				if (*aAttr[iI].name == '\0')
				{
					sError = ASConfigErrorHandler::InvalidAttributeValue(sElement, "file", "", iLine, iColumn);
					return -1;
				}

				// Check wildcard
				if (strstr(aAttr[iI].value, "*") != NULL)
				{
					return IncludeWildcard(aAttr[iI].value, sElement, iLevel, iLine, iColumn);
				}

				// Include file
				return IncludeFile(aAttr[iI].value, sElement, iLevel, iLine, iColumn);
			}

			sError = ASConfigErrorHandler::ExtraAttribute(sElement, aAttr[iI].name, iLine, iColumn);
			return -1;
		}

		sError = ASConfigErrorHandler::MissedAttribute(sElement, "version", iLine, iColumn);
		return -1;
	}

	sCharacters.erase();
	switch (iLevel)
	{
		// Level 1: <CASConfig version="3.8">
		case 0:
		{
			// CPPASConfig kept for backward compatibility
			if (strcasecmp("CASConfig", sElement) == 0)
			{
				// Get version
				for (UINT_32 iI = 0; iI < iAttributes; ++iI)
				{
					if (strcasecmp("version", aAttr[iI].name) == 0)
					{
						if (strcasecmp("3.8", aAttr[iI].value) == 0) { return 0; }

						sError = ASConfigErrorHandler::InvalidAttribute(sElement, aAttr[iI].name, iLine, iColumn);
						return -1;
					}
					else
					{
						sError = ASConfigErrorHandler::ExtraAttribute(sElement, aAttr[iI].name, iLine, iColumn);
						return -1;
					}
				}

				sError = ASConfigErrorHandler::MissedAttribute(sElement, "version", iLine, iColumn);
				return -1;
			}
			// Error!
			else
			{
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
		}
		// Level 2: <ConfigIncludeDirs> <LibexecDirs> <Modules>
		case 1:
		{
			// <ConfigIncludeDirs>
			if      (strcasecmp("ConfigIncludeDirs", sElement) == 0) { return 0; }
			// <LibexecDirs>
			else if (strcasecmp("LibexecDirs",       sElement) == 0) { return 0; }
			// <Modules>
			else if (strcasecmp("Modules",           sElement) == 0) { return 0; }
			// Error!
			else
			{
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
		}
		// Level 3: <ConfigIncludeDir> <LibexecDir> <Module>
		case 2:
		{
			if      (strcasecmp("ConfigIncludeDir", sElement) == 0) { return 0; }
			// <LibexecDir>
			else if (strcasecmp("LibexecDir",       sElement) == 0) { return 0; }
			// <Modules>
			else if (strcasecmp("Module",           sElement) == 0)
			{
				oGlobalConfig.modules_list.push_back(ASObjectConfig());
				pModule = &oGlobalConfig.modules_list[oGlobalConfig.modules_list.size() - 1];
				pModule -> object = NULL;

				for (UINT_32 iI = 0; iI < iAttributes; ++iI)
				{
					if      (strcasecmp("Name",       aAttr[iI].name) == 0) { pModule -> name.assign(aAttr[iI].value);       }
					else if (strcasecmp("Library",    aAttr[iI].name) == 0) { pModule -> library.assign(aAttr[iI].value);    }
					else if (strcasecmp("Moduletype", aAttr[iI].name) == 0) { pModule -> moduletype.assign(aAttr[iI].value); }
					else
					{
						sError = ASConfigErrorHandler::InvalidAttribute(sElement, aAttr[iI].name, iLine, iColumn);
						return -1;
					}
				}

				// Check name
				if (pModule -> name.size() == 0)       { sError = ASConfigErrorHandler::MissedAttribute(sElement, "Name",       iLine, iColumn); return -1; }
				// Check name
				if (pModule -> library.size() == 0)    { sError = ASConfigErrorHandler::MissedAttribute(sElement, "Library",    iLine, iColumn); return -1; }
				// Check name
				if (pModule -> moduletype.size() == 0) { sError = ASConfigErrorHandler::MissedAttribute(sElement, "ModuleType", iLine, iColumn); return -1; }

				pGenericHandler   = new ASGenericConfigHandler(pModule -> configuration);
				return 0;
			}
			// Error!
			else
			{
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
		}
		// Level 4....: CDT Config
		default:
			return pGenericHandler -> StartElement(sElement, iLevel, aAttr, iAttributes, iLine, iColumn);
	}

return -1;
}

//
// End of XML Element
//
INT_32 ASGlobalConfigHandler::EndElement(CCHAR_P        sElement,
                                         const UINT_32  iLevel,
                                         const UINT_32  iLine,
                                         const UINT_32  iColumn)
{
	switch (iLevel)
	{
		// Level 1: <CASGlobalConfig version="1.0">
		case 0:
		{
			return 0;
		}
		// Level 2: <ConfigIncludeDir> <LibexecDirs> <Modules>
		case 1:
		{
			return 0;
		}
		// Level 3: <ConfigIncludeDir> <LibexecDir> <Module> <Include>
		case 2:
		{
			// <ConfigIncludeDir>
			if (strcasecmp("ConfigIncludeDir", sElement) == 0)
			{
				vIncludeDirs.push_back(sCharacters);
				return 0;
			}
			// <LibexecDir>
			else if (strcasecmp("LibexecDir", sElement) == 0)
			{
				oGlobalConfig.libexec_dirs.push_back(sCharacters);
				return 0;
			}
			// <Module>
			else if (strcasecmp("Module", sElement) == 0)
			{
				delete pGenericHandler;
				pGenericHandler = NULL;
				return 0;
			}
			// Include
			else if (strcasecmp("Include", sElement) == 0)
			{
				return 0;
			}
			break;
		}

		// Level 4....: CDT Config
		default:
			return pGenericHandler -> EndElement(sElement, iLevel, iLine, iColumn);
	}

	sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);

return -1;
}

//
// Characters insige a element
//
INT_32 ASGlobalConfigHandler::Characters(CCHAR_P        sData,
                                         const UINT_32  iLength,
                                         const UINT_32  iLevel,
                                         const UINT_32  iLine,
                                         const UINT_32  iColumn)
{
	// Generic section of configuration
	if (iLevel >= 3)
	{
		return pGenericHandler -> Characters(sData, iLength, iLevel, iLine, iColumn);
	}
	else
	{
		sCharacters.append(sData, iLength);
	}

return 0;
}

//
// XML Parse Error Handler
//
void ASGlobalConfigHandler::ParseError(CCHAR_P        sReason,
                                       const UINT_32  iLevel,
                                       const UINT_32  iLine,
                                       const UINT_32  iColumn)
{
	sError = ASConfigErrorHandler::InternalError(sReason, iLine, iColumn);
}

} // namespace CAS
// End.
