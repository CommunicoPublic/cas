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
 *      ASHostConfigHandler.cpp
 *
 * $CAS$
 */

// C++ Includes
#include "ASHostConfigHandler.hpp"

#include "ASConfigErrorHandler.hpp"
#include "ASGenericConfigHandler.hpp"
#include "ASException.hpp"
#include "ASHostConfig.hpp"
#include "ASLocationURI.hpp"
#include "ASRequest.hpp"
#include "ASXMLParser.hpp"
#include "ASUtil.hpp"

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
//
// Adjust path
//
static STLW::string AdjustPath(const STLW::string & sPath, const STLW::string & sMainPath)
{
	if (sPath.empty() || sPath[0] == '/' || sMainPath.empty()) { return sPath; }

	STLW::string sTMP(sMainPath);
	if (sTMP[sTMP.size() -1] != '/') { sTMP.append("/", 1); }

	sTMP.append(sPath);

return sTMP;
}

//
// Parse variables with abbreviatures, e.g. 15M = 15 * 1024 * 1024
//
static INT_64 ParseSize(const STLW::string & sData)
{
	CCHAR_P szData = sData.c_str();
	CCHAR_P szDataEnd = szData + sData.size();
	int iSize = 0;
	if (sscanf(szData, "%d", &iSize) == 0) { return 0; }

	while (--szDataEnd != szData && *szDataEnd == ' ') { ;; }

	switch(*szDataEnd)
	{
		case 'G':
		case 'g':
			iSize *= 1024;
		case 'M':
		case 'm':
			iSize *= 1024;
		case 'K':
		case 'k':
			iSize *= 1024;
	}
return iSize;
}

//
// Parse list of supported methods
//
static UINT_32 ParseMethods(const STLW::string & sData)
{
	CCHAR_P szTMP = sData.c_str();
	CCHAR_P szTMPEnd = szTMP + sData.size();
	UINT_32 iMethods = 0;

	for(;;)
	{
		while (szTMP != szTMPEnd && (*szTMP == ' ' || *szTMP == ',' || *szTMP == '|' || *szTMP == ';')) { ++szTMP; }
		if (szTMP == szTMPEnd) { break; }

		if      (StrCaseStr(szTMP, "GET")     == szTMP + 3) { iMethods |= ASRequest::GET;     szTMP += 3; }
		else if (StrCaseStr(szTMP, "PUT")     == szTMP + 3) { iMethods |= ASRequest::PUT;     szTMP += 3; }
		else if (StrCaseStr(szTMP, "POST")    == szTMP + 4) { iMethods |= ASRequest::POST;    szTMP += 4; }
		else if (StrCaseStr(szTMP, "DELETE")  == szTMP + 6) { iMethods |= ASRequest::DELETE;  szTMP += 6; }
		else if (StrCaseStr(szTMP, "CONNECT") == szTMP + 7) { iMethods |= ASRequest::CONNECT; szTMP += 7; }
		else if (StrCaseStr(szTMP, "OPTIONS") == szTMP + 7) { iMethods |= ASRequest::OPTIONS; szTMP += 7; }
		else if (StrCaseStr(szTMP, "TRACE")   == szTMP + 5) { iMethods |= ASRequest::TRACE;   szTMP += 5; }
		else
		{
			STLW::string sTMP("Invalid method name \"");
			sTMP.append(szTMP);
			sTMP.append("\"");
			throw CTPP::CTPPLogicError(sTMP.c_str());
		}
	}

return iMethods;
}

//
// Constructor
//
ASHostConfigHandler::ASHostConfigHandler(ASHostConfig & oIHostConfig): oHostConfig(oIHostConfig),
                                                                       pGenericHandler(NULL),
                                                                       oCurrentState(ROOT),
                                                                       pLocation(NULL),
                                                                       iDefaultResponseCode(0),
                                                                       iAllowedMethods(0xFFFFFFFF),
                                                                       iMaxPostSize(-1),
                                                                       iMaxFileSize(-1)
{
	;;
}

//
// Constructor
//
ASHostConfigHandler::ASHostConfigHandler(ASHostConfig                       & oIHostConfig,
                                         eCurrentState                      & oICurrentState,
                                         const STLW::vector<eCurrentState>  & vIStatePath,
                                         ASLocation                         * pILocation,
                                         const UINT_32                        iIDefaultResponseCode,
                                         const STLW::string                 & sIDefaultContentType,
                                         const UINT_32                        iIAllowedMethods,
                                         const INT_64                         iIMaxPostSize,
                                         const INT_64                         iIMaxFileSize,
                                         const STLW::vector<STLW::string>   & vIConfigIncludeDirs): oHostConfig(oIHostConfig),
                                                                                                    pGenericHandler(NULL),
                                                                                                    oCurrentState(oICurrentState),
                                                                                                    vStatePath(vIStatePath),
                                                                                                    pLocation(pILocation),
                                                                                                    iDefaultResponseCode(iIDefaultResponseCode),
                                                                                                    sDefaultContentType(sIDefaultContentType),
                                                                                                    iAllowedMethods(iIAllowedMethods),
                                                                                                    iMaxPostSize(iIMaxPostSize),
                                                                                                    iMaxFileSize(iIMaxFileSize),
                                                                                                    vIncludeDirs(vIConfigIncludeDirs)
{
	;;
}

//
// Get error description
//
STLW::string ASHostConfigHandler::GetError() { return sError; }

//
// A destructor
//
ASHostConfigHandler::~ASHostConfigHandler() throw()
{
	if (pGenericHandler != NULL) { delete pGenericHandler; }
}

//
// Include wildcard directive
//
INT_32 ASHostConfigHandler::IncludeWildcard(CCHAR_P          sWildcard,
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
			// Create host config parser object
			ASHostConfigHandler oHandler(oHostConfig,
			                             oCurrentState,
			                             vStatePath,
			                             pLocation,
			                             iDefaultResponseCode,
			                             sDefaultContentType,
			                             iAllowedMethods,
			                             iMaxPostSize,
			                             iMaxFileSize,
			                             vIncludeDirs);

			// Create generic XML parser
			ASXMLParser oParser(&oHandler, iLevel);

			// Parse configuration
			if (oParser.ParseFile(F) == -1)
			{
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
		return 0;

		++itoFiles;
	}

return -1;
}

//
// Find files by mask
//
INT_32 ASHostConfigHandler::FindFiles(CCHAR_P                    sWildcard,
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
INT_32 ASHostConfigHandler::IncludeFile(CCHAR_P          sFileName,
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
		// Create host config parser object
		ASHostConfigHandler oHandler(oHostConfig,
		                             oCurrentState,
		                             vStatePath,
		                             pLocation,
		                             iDefaultResponseCode,
		                             sDefaultContentType,
		                             iAllowedMethods,
		                             iMaxPostSize,
		                             iMaxFileSize,
		                             vIncludeDirs);

		// Create generic XML parser
		ASXMLParser oParser(&oHandler, iLevel);

		// Parse configuration
		if (oParser.ParseFile(F) == -1)
		{
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
INT_32 ASHostConfigHandler::StartElement(CCHAR_P          sElement,
                                         const UINT_32    iLevel,
                                         const XMLAttr  * aAttr,
                                         const UINT_32    iAttributes,
                                         const UINT_32    iLine,
                                         const UINT_32    iColumn)
{
	try
	{
		vStatePath.push_back(oCurrentState);
		sCharacters.erase();

		// Special case: Include directive
		if (iLevel < 5 && oCurrentState != MODULE && strcasecmp("Include", sElement) == 0)
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

			sError = ASConfigErrorHandler::MissedAttribute(sElement, "file", iLine, iColumn);
			return -1;
		}

		switch(iLevel)
		{
			// Level 1: <CASHostConfig version="3.8">
			case 0:
			{
				if (strcasecmp("CASHostConfig", sElement) == 0)
				{
					// No attribute "Version"
					if      (iAttributes == 0) { sError = ASConfigErrorHandler::MissedAttribute(sElement, "Version", iLine, iColumn); return -1; }

					STLW::string sVersion;
					for (UINT_32 iI = 0; iI < iAttributes; ++iI)
					{
						if (strcasecmp("Version", aAttr[iI].name) == 0) { sVersion = aAttr[iI].value; }
						else { sError = ASConfigErrorHandler::ExtraAttribute(sElement, aAttr[iI].name, iLine, iColumn); return -1; }
					}

					// Need version 3.8
					if (strcasecmp("3.8", sVersion.c_str()) == 0) { return 0; }

					sError = ASConfigErrorHandler::InvalidAttributeValue(sElement, "Version", sVersion, iLine, iColumn);
					return -1;
				}
				// Error!
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
			break;
			// Level 1: ServerName, Directories, ConfigIncludeDirs, Request, Response, Modules, Locations
			case 1:
			{
				// Check attributes
				if (iAttributes != 0) { sError = ASConfigErrorHandler::ExtraAttribute(sElement, aAttr[0].name, iLine, iColumn); return -1; }

				else if (strcasecmp("ServerName",          sElement) == 0) { return 0; }
				else if (strcasecmp("Directories",         sElement) == 0) { oCurrentState = DIRECTORIES;           return 0; }
				else if (strcasecmp("ConfigIncludeDirs",   sElement) == 0) { oCurrentState = CONFIG_INCLUDE_DIRS;   return 0; }
				else if (strcasecmp("Request",             sElement) == 0) { oCurrentState = REQUEST;               return 0; }
				else if (strcasecmp("Response",            sElement) == 0) { oCurrentState = RESPONSE;              return 0; }
				else if (strcasecmp("Modules",             sElement) == 0) { oCurrentState = MODULES;               return 0; }
				else if (strcasecmp("Locations",           sElement) == 0) { oCurrentState = LOCATIONS;             return 0; }

				// Error!
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
			break;
			// Level 2: ConfigIncludeDir, ServerRoot, TempDir, AllowedMethods, MaxFileSize
			//          MaxPostSize, RequestParser,
			//          DefaultResponseCode, DefaultContentType, Location
			case 2:
			{
				if      (strcasecmp("ConfigIncludeDir",    sElement) == 0)
				{
					if (oCurrentState != CONFIG_INCLUDE_DIRS) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("ServerRoot",          sElement) == 0)
				{
					if (oCurrentState != DIRECTORIES) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("TempDir",             sElement) == 0)
				{
					if (oCurrentState != DIRECTORIES) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("AllowedMethods", sElement) == 0)
				{
					if (oCurrentState != REQUEST) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("MaxFileSize",    sElement) == 0)
				{
					if (oCurrentState != REQUEST) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("MaxPostSize",    sElement) == 0)
				{
					if (oCurrentState != REQUEST) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("RequestParser",   sElement) == 0)
				{
					if (oCurrentState != REQUEST) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					// No attribute "Version"
					if      (iAttributes != 0) { sError = ASConfigErrorHandler::MissedAttribute(sElement, "Version", iLine, iColumn); return -1; }

					STLW::string sContentType;
					STLW::string sHandlerName;
					for (UINT_32 iI = 0; iI < iAttributes; ++iI)
					{
						if      (strcasecmp("ContentType", aAttr[iI].name) == 0) { sContentType = aAttr[iI].value; }
						else if (strcasecmp("Parser",      aAttr[iI].name) == 0) { sHandlerName = aAttr[iI].value; }
						else { sError = ASConfigErrorHandler::ExtraAttribute(sElement, aAttr[iI].name, iLine, iColumn); return -1; }
					}
					if (sContentType.empty()) { sError = ASConfigErrorHandler::MissedAttribute(sElement, "ContentType", iLine, iColumn); return -1; }
					if (sHandlerName.empty()) { sError = ASConfigErrorHandler::MissedAttribute(sElement, "Parser",      iLine, iColumn); return -1; }

					// Check content-type
					STLW::map<STLW::string, ASRequestParser *>::const_iterator itRequestParsers = mRequestParsers.find(sContentType);
					if (itRequestParsers == mRequestParsers.end())
					{
//						mRequestParsers[sContentType] = sHandlerName;
						return 0;
					}

					sError = ASConfigErrorHandler::InvalidAttributeValue(sElement, "ContentType", sContentType.append("Already has one request parser"), iLine, iColumn);
					return -1;
				}
				else if (strcasecmp("DefaultResponseCode", sElement) == 0)
				{
					if (oCurrentState != RESPONSE) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("DefaultContentType",  sElement) == 0)
				{
					if (oCurrentState != RESPONSE) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("Module",              sElement) == 0)
				{
					if (oCurrentState != MODULES) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					for (UINT_32 iI = 0; iI < iAttributes; ++iI)
					{
						if   (strcasecmp("name", aAttr[iI].name) == 0) { sModuleName = aAttr[iI].value; }
						else { sError = ASConfigErrorHandler::InvalidAttribute(sElement, aAttr[iI].name, iLine, iColumn); return -1; }
					}

					ASModuleConfig oModuleConfig;
					oModuleConfig.module_name   = sModuleName;
					oModuleConfig.module_config = CTPP::CDT(CTPP::CDT::HASH_VAL);
					oHostConfig.modules_config.push_back(oModuleConfig);
					pGenericHandler   = new ASGenericConfigHandler(oHostConfig.modules_config.rbegin() -> module_config);

					oCurrentState = MODULE;
					return 0;
				}
				else if (strcasecmp("Location", sElement) == 0)
				{
					// Check tags hierarchy: Locations -> Location
					if (oCurrentState != LOCATIONS) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					// Get location name
					if      (iAttributes == 0) { sError = ASConfigErrorHandler::MissedAttribute(sElement, "Name", iLine, iColumn); return -1; }

					STLW::string sName;
					for (UINT_32 iI = 0; iI < iAttributes; ++iI)
					{
						if (strcasecmp("Name", aAttr[iI].name) == 0) { sName = aAttr[iI].value; }
						else { sError = ASConfigErrorHandler::ExtraAttribute(sElement, aAttr[iI].name, iLine, iColumn); return -1; }
					}

					// Create new location
					oHostConfig.locations.push_back(ASLocation());

					pLocation = &oHostConfig.locations[oHostConfig.locations.size() - 1];
					pLocation -> name                  = sName;
					pLocation -> default_response_code = iDefaultResponseCode;
					pLocation -> default_content_type  = sDefaultContentType;
					pLocation -> allowed_methods       = iAllowedMethods;
					pLocation -> max_file_size         = iMaxFileSize;
					pLocation -> max_post_size         = iMaxPostSize;

					oCurrentState = LOCATION;
					return 0;
				}

				// Error!
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
			break;
			// Level 3: DefaultResponseCode, DefaultContentType, URIList, Controller, Handlers, Finalizers, Views
			case 3:
			{
				if (strcasecmp("Request", sElement) == 0)
				{
					// Check tags hierarchy: Location -> Request
					if (oCurrentState != LOCATION) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					oCurrentState = REQUEST;
					return 0;
				}
				else if (strcasecmp("Response", sElement) == 0)
				{
					// Check tags hierarchy: Location -> Response
					if (oCurrentState != LOCATION) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					oCurrentState = RESPONSE;
					return 0;
				}
				else if (strcasecmp("URIList", sElement) == 0)
				{
					// Check tags hierarchy: Location -> URIList
					if (oCurrentState != LOCATION) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					oCurrentState = URILIST;
					return 0;
				}
				else if (strcasecmp("RequestHooks", sElement) == 0)
				{
					// Check tags hierarchy: Location -> RequestHooks
					if (oCurrentState != LOCATION) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					oCurrentState = REQUEST_HOOKS;
					return 0;
				}
				else if (strcasecmp("Controller", sElement) == 0)
				{
					// Check tags hierarchy: Location -> Controller
					if (oCurrentState != LOCATION) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					oCurrentState = CONTROLLER;
					return 0;
				}
				else if (strcasecmp("Handlers", sElement) == 0)
				{
					// Check tags hierarchy: Location -> Handlers
					if (oCurrentState != LOCATION) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					oCurrentState = HANDLERS;
					return 0;
				}
				else if (strcasecmp("Finalizers", sElement) == 0)
				{
					// Check tags hierarchy: Location -> Finalizers
					if (oCurrentState != LOCATION) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					oCurrentState = FINALIZERS;
					return 0;
				}
				else if (strcasecmp("Views", sElement) == 0)
				{
					// Check tags hierarchy: Location -> Views
					if (oCurrentState != LOCATION) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					oCurrentState = VIEWS;
					return 0;
				}
				else if (oCurrentState == MODULE)
				{
					INT_32 iRC = pGenericHandler -> StartElement(sElement, iLevel, aAttr, iAttributes, iLine, iColumn);
					if (iRC != 0) { sError = pGenericHandler -> GetError(); }

					return iRC;
				}

				// Error!
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
			break;
			// Level 4: URI, Handler
			case 4:
			{
				if      (strcasecmp("AllowedMethods", sElement) == 0)
				{
					// Check tags hierarchy: Request -> AllowedMethods
					if (oCurrentState != REQUEST) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("MaxFileSize", sElement) == 0)
				{
					// Check tags hierarchy: Request -> MaxFileSize
					if (oCurrentState != REQUEST) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("MaxPostSize", sElement) == 0)
				{
					// Check tags hierarchy: Request -> MaxPostSize
					if (oCurrentState != REQUEST) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("DefaultResponseCode", sElement) == 0)
				{
					// Check tags hierarchy: Response -> DefaultResponseCode
					if (oCurrentState != RESPONSE) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("DefaultContentType",  sElement) == 0)
				{
					// Check tags hierarchy: Location -> DefaultContentType
					if (oCurrentState != RESPONSE) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }
					return 0;
				}
				else if (strcasecmp("URI", sElement) == 0)
				{
					// Check tags hierarchy: Location -> Handlers
					if (oCurrentState != URILIST) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					// Plain or regexp
					sType.assign("plain");

					for (UINT_32 iI = 0; iI < iAttributes; ++iI)
					{
						if      (strcasecmp("type",   aAttr[iI].name) == 0) { sType   = aAttr[iI].value; }
						else { sError = ASConfigErrorHandler::InvalidAttribute(sElement, aAttr[iI].name, iLine, iColumn); return -1; }
					}

					return 0;
				}
				else if (strcasecmp("Handler", sElement) == 0)
				{
					// Check tags hierarchy: RequestHooks/Controller.Handlers/Finalizers/Views -> Handler
					if (oCurrentState != REQUEST_HOOKS &&
					    oCurrentState != CONTROLLER    &&
					    oCurrentState != HANDLERS      &&
					    oCurrentState != FINALIZERS    &&
					    oCurrentState != VIEWS) { sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn); return -1; }

					// Handler name
					STLW::string sName;

					for (UINT_32 iI = 0; iI < iAttributes; ++iI)
					{
						if (strcasecmp("name",   aAttr[iI].name) == 0) { sName   = aAttr[iI].value; }
						else { sError = ASConfigErrorHandler::InvalidAttribute(sElement, aAttr[iI].name, iLine, iColumn); return -1; }
					}

					// Empty template name
					if (sName.empty()) { sError = ASConfigErrorHandler::InvalidAttributeValue(sElement, "name", "", iLine, iColumn); return -1; }

					switch(oCurrentState)
					{
						case REQUEST_HOOKS:
							{

								pLocation -> pre_request_handlers.push_back(ASPreRequestHandlerConfig());
								ASPreRequestHandlerConfig  * pHookConfig = &(pLocation -> pre_request_handlers[pLocation -> pre_request_handlers.size() - 1]);

								pHookConfig -> name         = sName;
								pHookConfig -> object       = NULL;
								pHookConfig -> context_data = NULL;

								pGenericHandler   = new ASGenericConfigHandler(pHookConfig -> handler_config);
							}
							break;

						case CONTROLLER:
							pLocation -> controller.name         = sName;
							pLocation -> controller.object       = NULL;
							pLocation -> controller.context_data = NULL;

							pGenericHandler   = new ASGenericConfigHandler(pLocation -> controller.handler_config);
							break;

						case HANDLERS:
							pLocation -> handlers.push_back(ASHandlerConfig());
							pHandlerConfig = &(pLocation -> handlers[pLocation -> handlers.size() - 1]);

							pHandlerConfig -> name         = sName;
							pHandlerConfig -> object       = NULL;
							pHandlerConfig -> context_data = NULL;

							pGenericHandler   = new ASGenericConfigHandler(pHandlerConfig -> handler_config);
							break;

						case FINALIZERS:
							pLocation -> finalizers.push_back(ASHandlerConfig());
							pHandlerConfig = &(pLocation -> finalizers[pLocation -> finalizers.size() - 1]);

							pHandlerConfig -> name         = sName;
							pHandlerConfig -> object       = NULL;
							pHandlerConfig -> context_data = NULL;

							pGenericHandler   = new ASGenericConfigHandler(pHandlerConfig -> handler_config);
							break;

						case VIEWS:
							{
								// Duplicate view name
								ASViewMap::iterator itmViews = pLocation -> views.find(sName);
								if (itmViews != pLocation -> views.end())
								{
									sError = ASConfigErrorHandler::InvalidAttributeValue(sElement, "name", "Duplicate view name", iLine, iColumn);
									return -1;
								}

								ASViewConfig oViewConfig;
								oViewConfig.name         = sName;
								oViewConfig.object       = NULL;
								oViewConfig.context_data = NULL;
								STLW::pair<ASViewMap::iterator, bool> oResult = pLocation -> views.insert(STLW::pair<STLW::string, ASViewConfig>(sName, oViewConfig));
								pGenericHandler = new ASGenericConfigHandler(oResult.first -> second.view_config);
							}
							break;

						default:
							{ ;; }
					}

					return 0;
				}
				else if (oCurrentState == MODULE)
				{
					INT_32 iRC = pGenericHandler -> StartElement(sElement, iLevel, aAttr, iAttributes, iLine, iColumn);
					if (iRC != 0) { sError = pGenericHandler -> GetError(); }

					return iRC;
				}

				// Error!
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
			break;
			// Levels 5 and above: Configuration, external parser
			default:
				INT_32 iRC = pGenericHandler -> StartElement(sElement, iLevel, aAttr, iAttributes, iLine, iColumn);
				if (iRC != 0) { sError = pGenericHandler -> GetError(); }

				return iRC;
		}

		sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
	}
	catch(CTPP::CTPPLogicError & e) { sError = ASConfigErrorHandler::TagProcessingError(sElement, e.what(), iLine, iColumn); }
	catch(STLW::exception      & e) { sError = ASConfigErrorHandler::TagProcessingError(sElement, e.what(), iLine, iColumn); }
	catch(...)                      { sError = ASConfigErrorHandler::TagProcessingError(sElement, "Unknown error occured", iLine, iColumn); }

return -1;
}

//
// End of XML Element
//
INT_32 ASHostConfigHandler::EndElement(CCHAR_P        sElement,
                                       const UINT_32  iLevel,
                                       const UINT_32  iLine,
                                       const UINT_32  iColumn)
{
	try
	{
		STLW::vector<eCurrentState>::iterator itvStatePath = vStatePath.end();
		--itvStatePath;
		oCurrentState = *itvStatePath;
		vStatePath.erase(itvStatePath);

		// Special case: Include directive
		if (iLevel < 5 && oCurrentState != MODULE && strcasecmp("Include", sElement) == 0) { return 0; }

		switch(iLevel)
		{
			// Level 1: <CASHostConfig version="3.4">
			case 0:
			{
				// Nothing to do
				return 0;
			}
			break;
			// Level 1: TempDir, ServerName, DefaultResponseCode, Locations
			case 1:
			{
				if      (strcasecmp("ServerName",          sElement) == 0) { oHostConfig.host_name = sCharacters; return 0; }
				else if (strcasecmp("ConfigIncludeDirs",   sElement) == 0) { return 0; }
				else if (strcasecmp("Directories",         sElement) == 0) { return 0; }
				else if (strcasecmp("Request",             sElement) == 0) { return 0; }
				else if (strcasecmp("Response",            sElement) == 0) { return 0; }
				else if (strcasecmp("Modules",             sElement) == 0) { return 0; }
				else if (strcasecmp("Locations",           sElement) == 0) { return 0; }

				return -1;
			}
			break;
			// Level 2: Location
			case 2:
			{
				if      (strcasecmp("ServerRoot",          sElement) == 0)
				{
					oHostConfig.server_root  = sCharacters;
					if (!oHostConfig.server_root.empty() && oHostConfig.server_root[oHostConfig.server_root.size() - 1] != '/')
					{
						oHostConfig.server_root.append("/", 1);
					}
					return 0;
				}
				else if (strcasecmp("ConfigIncludeDir",    sElement) == 0) { vIncludeDirs.push_back(AdjustPath(sCharacters, oHostConfig.server_root)); return 0; }
				else if (strcasecmp("TempDir",             sElement) == 0) { oHostConfig.tempfile_dir = AdjustPath(sCharacters, oHostConfig.server_root); return 0; }
				else if (strcasecmp("AllowedMethods",      sElement) == 0) { iAllowedMethods = ParseMethods(sCharacters); return 0; }
				else if (strcasecmp("MaxFileSize",         sElement) == 0) { iMaxFileSize    = ParseSize(sCharacters);    return 0; }
				else if (strcasecmp("MaxPostSize",         sElement) == 0) { iMaxPostSize    = ParseSize(sCharacters);    return 0; }
				else if (strcasecmp("DefaultResponseCode", sElement) == 0)
				{
					int iTMP = 0;
					sscanf(sCharacters.c_str(), "%d", &iTMP);
					if (iTMP == 0)
					{
						sError = ASConfigErrorHandler::InvalidTagValue(sElement, sCharacters, iLine, iColumn);
						return -1;
					}

					iDefaultResponseCode = iTMP;
					return 0;
				}
				else if (strcasecmp("DefaultContentType",  sElement) == 0)
				{
					sDefaultContentType.assign(sCharacters);
					return 0;
				}
				else if (strcasecmp("Module",              sElement) == 0)
				{
					delete pGenericHandler;
					pGenericHandler = NULL;
					return 0;
				}
				else if (strcasecmp("Location",            sElement) == 0)
				{
					if (pLocation -> default_response_code == 0)
					{
						sError = ASConfigErrorHandler::InternalError("Need to set Default HTTP Response Code", iLine, iColumn);
						return -1;
					}
					return 0;
				}
				return -1;
			}
			break;
			// Level 3: URIList, Controller, Handlers, Finalizers
			case 3:
			{
				if      (strcasecmp("Request",    sElement) == 0) { return 0; }
				else if (strcasecmp("Response",   sElement) == 0) { return 0; }
				else if (strcasecmp("URIList",    sElement) == 0) { return 0; }
				else if (strcasecmp("Controller", sElement) == 0) { return 0; }
				else if (strcasecmp("Handlers",   sElement) == 0) { return 0; }
				else if (strcasecmp("Finalizers", sElement) == 0) { return 0; }
				else if (strcasecmp("Views",      sElement) == 0) { return 0; }
				else if (oCurrentState == MODULE)
				{
					INT_32 iRC = pGenericHandler -> EndElement(sElement, iLevel, iLine, iColumn);
					if (iRC != 0) { sError = pGenericHandler -> GetError(); }

					return iRC;
				}
				return -1;
			}
			break;
			// Level 4: URI, Handler
			case 4:
			{
				if      (strcasecmp("AllowedMethods",      sElement) == 0) { pLocation -> allowed_methods = ParseMethods(sCharacters); return 0; }
				else if (strcasecmp("MaxFileSize",         sElement) == 0) { pLocation -> max_file_size   = ParseSize(sCharacters);    return 0; }
				else if (strcasecmp("MaxPostSize",         sElement) == 0) { pLocation -> max_post_size   = ParseSize(sCharacters);    return 0; }
				else if (strcasecmp("DefaultResponseCode", sElement) == 0)
				{
					int iTMP = 0;
					sscanf(sCharacters.c_str(), "%d", &iTMP);
					if (iTMP == 0)
					{
						sError = ASConfigErrorHandler::InvalidTagValue(sElement, sCharacters, iLine, iColumn);
						return -1;
					}
					pLocation -> default_response_code = iTMP;
					return 0;
				}
				else if (strcasecmp("DefaultContentType",  sElement) == 0)
				{
					pLocation -> default_content_type.assign(sCharacters);
					return 0;
				}
				else if (strcasecmp("URI",                 sElement) == 0)
				{
					// Empty template name
					if (sCharacters.empty()) { sError = ASConfigErrorHandler::InvalidAttributeValue(sElement, "name", "", iLine, iColumn); return -1; }

					// URI type
					ASLocationURI::eURIType oURIType = ASLocationURI::PLAIN_URI;
					if      (strcasecmp("plain",  sType.c_str()) == 0) { oURIType = ASLocationURI::PLAIN_URI; }
					else if (strcasecmp("regexp", sType.c_str()) == 0) { oURIType = ASLocationURI::REGEXP_URI; }
					else { sError = ASConfigErrorHandler::InvalidAttributeValue(sElement, "type", sType, iLine, iColumn); return -1; }

					// Store location
					pLocation -> uri_list.push_back(new ASLocationURI(sCharacters, oURIType));

					return 0;
				}
				else if (strcasecmp("Handler", sElement) == 0)
				{
					delete pGenericHandler;
					pGenericHandler = NULL;
					return 0;
				}
				else if (oCurrentState == MODULE)
				{
					INT_32 iRC = pGenericHandler -> EndElement(sElement, iLevel, iLine, iColumn);
					if (iRC != 0) { sError = pGenericHandler -> GetError(); }

					return iRC;
				}

				// Error!
				sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
				return -1;
			}
			break;
			// Levels 5 and above: Configuration, external parser
			default:
				INT_32 iRC = pGenericHandler -> EndElement(sElement, iLevel, iLine, iColumn);
				if (iRC != 0) { sError = pGenericHandler -> GetError(); }

				return iRC;
		}

		sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
	}
	catch(LogicError       & e) { sError = ASConfigErrorHandler::TagProcessingError(sElement, e.what(), iLine, iColumn); }
	catch(STLW::exception  & e) { sError = ASConfigErrorHandler::TagProcessingError(sElement, e.what(), iLine, iColumn); }
	catch(...)                  { sError = ASConfigErrorHandler::TagProcessingError(sElement, "Unknown error occured", iLine, iColumn); }

return -1;
}

//
// Characters insige a element
//
INT_32 ASHostConfigHandler::Characters(CCHAR_P        sData,
                                       const UINT_32  iLength,
                                       const UINT_32  iLevel,
                                       const UINT_32  iLine,
                                       const UINT_32  iColumn)
{
	// Generic section of configuration
	if (iLevel >= 5 || oCurrentState == MODULE)
	{
		INT_32 iRC = pGenericHandler -> Characters(sData, iLength, iLevel, iLine, iColumn);
		if (iRC != 0) { sError = pGenericHandler -> GetError(); }

		return iRC;
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
void ASHostConfigHandler::ParseError(CCHAR_P        sReason,
                                     const UINT_32  iLevel,
                                     const UINT_32  iLine,
                                     const UINT_32  iColumn)
{
	sError = ASConfigErrorHandler::InternalError(sReason, iLine, iColumn);
}

} // namespace CAS
// End.
