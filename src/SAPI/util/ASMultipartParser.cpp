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
 *      ASMultipartParser.ñpp
 *
 * $CAS$
 */

// C++ Includes
#include "ASMultipartParser.hpp"

#include "ASLogger.hpp"
#include "ASSAPIUtil.hpp"

#include <sys/types.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

namespace CAS // C++ Application Server
{
#define C_CONTENT_DISPOSITION      "Content-Disposition"
#define C_CONTENT_TYPE             "Content-Type"

#define C_KEY_CONTENT_DISPOSITION  0x00000001
#define C_KEY_CONTENT_TYPE         0x00000002

// Undefined state of parser
#define C_UNDEF                    0x00000000
// Unrecoverable error
#define C_ERROR                    0x10000000

// Boundary section
#define C_BOUNDARY_IN              0x00010001
#define C_BOUNDARY_IN_SUFFIX1      0x00010002
#define C_BOUNDARY_IN_SUFFIX2      0x00010004
#define C_BOUNDARY_IN_SUFFIX3      0x00010008
#define C_BOUNDARY_IN_SUFFIX4      0x00010010
#define C_END_MIME                 0x000FFFFF
#define C_BOUNDARY_IN_SUFFIX5      0x00010020
#define C_BOUNDARY_IN_SUFFIX6      0x00010040

// Header fields
#define C_HEADER                   0x00020001
#define C_KEY_NAME                 0x00020002
#define C_KEY_SPACE                0x00020004
#define C_KEY_VALUE                0x00020008
#define C_HEADER_CR                0x00020010
#define C_HEADER_LF                0x00020020
#define C_HEADER_CR2               0x00020040
#define C_HEADER_LF2               0x00020080

// Body section
#define C_BODY_FILE                0x00000001
#define C_BODY_VALUE               0x00000002

#define C_FIELD_IS_NAME            0x00100000
#define C_FIELD_IS_FILENAME        0x00200000

#define C_TMP_FNAME_SIZE           1024

//
// A constructor
//
MultipartParser::MultipartParser(CTPP::CDT                   & oIData,
                                 ASFilePool                  & mIFiles,
                                 STLW::vector<STLW::string>  & vIFilelist,
                                 const STLW::string          & sIUploadDir,
                                 const STLW::string          & sIBoundary,
                                 const INT_64                & iIMaxFileSize,
                                 ASServer                    & oIServer,
                                 ASServer::ASRequestContext  & oIRequestContext,
                                 ASRequest                   & oIRequest,
                                 ASLogger                    & oILogger): oData(oIData),
                                                                          mFiles(mIFiles),
                                                                          vFilelist(vIFilelist),
                                                                          sUploadDir(sIUploadDir),
                                                                          sBoundary(sIBoundary),
                                                                          szBoundaryPos(sBoundary.data() + 2),
                                                                          iMaxFileSize(iIMaxFileSize),
                                                                          iState(C_BOUNDARY_IN),
                                                                          iPrevState(C_ERROR),
                                                                          iHeaderKeyType(C_UNDEF),
                                                                          oServer(oIServer),
                                                                          oRequestContext(oIRequestContext),
                                                                          oRequest(oIRequest),
                                                                          oLogger(oILogger)
{
	// Add slash, if need
	if (sUploadDir.size() > 0 && sUploadDir[sUploadDir.size() - 1] != '/') { sUploadDir.append("/", 1); }
}

//
// Parse HTTP header
//
void MultipartParser::ParseContentDisposition()
{
	// form-data; name="submit-name"
	// form-data; name="files"; filename="c:\text.txt"
	static const UINT_32 iFormDataLen = sizeof("form-data") - 1;
	static const UINT_32 iNameLen     = sizeof("name") - 1;
	static const UINT_32 iFilenameLen = sizeof("filename") - 1;

	CCHAR_P szBuffer = sBuffer.c_str();

	// form-data
	// 123456789
	if (strncmp(szBuffer, "form-data", iFormDataLen) != 0) { return; }

	// End of buffer
	CCHAR_P szData    = szBuffer + iFormDataLen;
	CCHAR_P szDataEnd = szBuffer + sBuffer.size();

	// Check ';'
	if (szData == szDataEnd || *szData != ';') { return; }
	++szData;

	// Parse string
	for (;;)
	{
		// Skip spaces
		while (szData != szDataEnd && *szData == ' ') { ++szData; }

		CCHAR_P szParamStart = szData;
		// Find '='
		while (szData != szDataEnd && *szData != '=') { ++szData; }

		UINT_32 iParamType = 0;
		if      (strncmp(szParamStart, "name",     iNameLen)     == 0) { iParamType = C_FIELD_IS_NAME;     }
		else if (strncmp(szParamStart, "filename", iFilenameLen) == 0) { iParamType = C_FIELD_IS_FILENAME; }

		if (szData == szDataEnd || *szData != '=') { break; }
		++szData;

		if (szData == szDataEnd || *szData != '"') { break; }
		++szData;

		szParamStart = szData;

		// Find '"'
		while (szData != szDataEnd && *szData != '"') { ++szData; }
		if (szData == szDataEnd) { break; }

		// Store field name
		if      (iParamType == C_FIELD_IS_NAME) { oFile.name.assign(szParamStart, szData - szParamStart); }
		// Store & process filename
		else if (iParamType == C_FIELD_IS_FILENAME)
		{
			// Store filename with full path
			oFile.full_filename.assign(szParamStart, szData - szParamStart);

			// Nothing to do
			if (oFile.full_filename.empty()) { F = NULL; }
			// Write file
			else
			{
				CCHAR_P szFileName = szData;
				// Store filename without path
				for (;;)
				{
					if (szFileName == szParamStart) { break; }
					if (*szFileName == '/' || *szFileName == '\\') { ++szFileName; break; }
					--szFileName;
				}
				oFile.filename.assign(szFileName, szData - szFileName);
				oFile.filesize = 0;

				// Create temp file name
				CHAR_8 szTMPFileName[C_TMP_FNAME_SIZE];
				snprintf(szTMPFileName, C_TMP_FNAME_SIZE, "u%d.p%d.t%d.r%d", int(getuid()), int(getpid()), int(time(NULL)), int(rand()));
				if (*szTMPFileName == '\0') { F = NULL; }
				else
				{
					oFile.tmp_name.assign(sUploadDir);
					oFile.tmp_name.append(szTMPFileName);

					// Open file for writing
					F = fopen(oFile.tmp_name.c_str(), "wb");
					if (F != NULL)
					{
						vFilelist.push_back(oFile.tmp_name);
					}
					else
					{
						oLogger.Error("CAS: Cannot open file \"%s\" for writing: %s",
						              oFile.tmp_name.c_str(),
						              strerror(errno));
					}
				}
			}
		}

		++szData;
		if (szData == szDataEnd || *szData != ';') { break; }

		++szData;
		if (szData == szDataEnd) { break; }
	}

	oServer.HandleRFC1867File(oRequestContext, oFile.name, oFile.full_filename, oFile.filename, oFile.tmp_name, oRequest, &oLogger);
/*
oLogger.Error("HEADER: NAME `%s`, FULL_FILENAME: `%s` FILENAME `%s`, TMP_NAME `%s`",
              oFile.name.data(),
              oFile.full_filename.data(),
              oFile.filename.data(),
              oFile.tmp_name.data());
*/
}

//
// Commit operation
//
void MultipartParser::CommitSection()
{
	// Write data to file
	if (iPrevState == C_BODY_FILE)
	{
		// Only if file was open
		if (F != NULL)
		{
			fclose(F);

			// Unlink file if file size is greater than iMaxFileSize
			if (iMaxFileSize != -1 && oFile.filesize >= UINT_64(iMaxFileSize))
			{
				oLogger.Error("File \"%s\" not saved: size %llu is larger than max allowed (%llu) bytes",
				              oFile.filename.c_str(),
				              (unsigned long long) (oFile.filesize),
				              (unsigned long long) (iMaxFileSize));

				unlink(oFile.tmp_name.c_str());
				oFile.tmp_name.erase();
			}
			// Store name in pool
			mFiles.AddFile(oFile);

			oLogger.Debug("File: { `%s`, `%s`, `%s`, `%s`, `%s`, %llu }",
			              oFile.name.data(),
			              oFile.full_filename.data(),
			              oFile.filename.data(),
			              oFile.tmp_name.data(),
			              oFile.content_type.data(),
			              (unsigned long long) (oFile.filesize));
		}

		// Clear data
		oFile.name.erase();
		oFile.full_filename.erase();
		oFile.filename.erase();
		oFile.tmp_name.erase();
		oFile.content_type.erase();
		oFile.filesize = 0;
	}
	// Append buffer
	else if (iPrevState == C_BODY_VALUE)
	{
		oLogger.Debug("Param: { `%s`, %llu }",
		              oFile.name.data(),
		              (long long unsigned) (sBuffer.size()));

		StorePair(oData, oFile.name, sBuffer);
		sBuffer.erase();
	}
}

//
// Write buffer to the file
//
void MultipartParser::WriteFile(CCHAR_P  szStart,
                                CCHAR_P  szEnd)
{
	const UINT_32 iBufSize = szEnd - szStart;
	if (iBufSize == 0) { return; }

//oLogger.Error("WRITE %u byte(s)", iBufSize);

	if (F == NULL) { return; }
	if (fwrite(szStart, 1, iBufSize, F) == iBufSize)
	{
		oFile.filesize += iBufSize;
		return;
	}

	oLogger.Error("CAS: Cannot write data(%u bytes) to file \"%s\": %s", UINT_32(iBufSize), oFile.tmp_name.c_str(), strerror(errno));
}

//
// Parse chunk of data
//
void MultipartParser::ParseChunk(CCHAR_P szString, CCHAR_P szStringEnd)
{
	// Last state of FSM, nothing to do
	if (iState == C_BOUNDARY_IN_SUFFIX4 || iState == C_END_MIME) { return; }

	CCHAR_P sStartPos        = szString;
	bool bFoundEndOfBoundary = false;
	bool bFoundEndOfKey      = false;
	bool bFoundEndOfSpace    = false;

	if (iState == C_BOUNDARY_IN)         { goto BOUNDARY_IN;         }
	if (iState == C_BOUNDARY_IN_SUFFIX1) { goto BOUNDARY_IN_SUFFIX1; }
	if (iState == C_BOUNDARY_IN_SUFFIX2) { goto BOUNDARY_IN_SUFFIX2; }
	if (iState == C_BOUNDARY_IN_SUFFIX3) { goto BOUNDARY_IN_SUFFIX3; }
	if (iState == C_BOUNDARY_IN_SUFFIX4) { goto BOUNDARY_IN_SUFFIX4; }
	if (iState == C_BOUNDARY_IN_SUFFIX5) { goto BOUNDARY_IN_SUFFIX5; }
	if (iState == C_BOUNDARY_IN_SUFFIX6) { goto BOUNDARY_IN_SUFFIX6; }

	if (iState == C_HEADER)              { goto HEADER;     }

	if (iState == C_KEY_NAME)            { goto KEY_NAME;   }
	if (iState == C_KEY_SPACE)           { goto KEY_SPACE;  }
	if (iState == C_KEY_VALUE)           { goto KEY_VALUE;  }

	if (iState == C_HEADER_CR)           { goto HEADER_CR;  }
	if (iState == C_HEADER_LF)           { goto HEADER_LF;  }
	if (iState == C_HEADER_CR2)          { goto HEADER_CR2; }
	if (iState == C_HEADER_LF2)          { goto HEADER_LF2; }

	if (iState == C_BODY_FILE)           { goto BODY_FILE;  }
	if (iState == C_BODY_VALUE)          { goto BODY_VALUE; }
//oLogger.Error("PARSE-CHUNK %u", UINT_32(szStringEnd - szString));
	for(;;)
	{
		//
		// \r - \n - dash - dash - BOUNADRY [dash - dash] - \r - \n C_END_MIME
		// +------------------------------+  |      |       |    |
		// |                                 |      |       |    |
		// |                                 |      |       |    BOUNDARY_IN_SUFFIX4, C_END_MIME
		// |                                 |      |       |
		// |                                 |      |       BOUNDARY_IN_SUFFIX3
		// |                                 |      |
		// |                                 |      BOUNDARY_IN_SUFFIX2
		// |                                 |
		// |                                 BOUNDARY_IN_SUFFIX1
		// |
		// C_BOUNDARY_IN
		//
		// OR
		//
		// \r - \n - dash - dash - BOUNADRY - \r - \n
		// +------------------------------+   |    |
		// |                                  |    |
		// |                                  |    BOUNDARY_IN_SUFFIX6
		// |                                  |
		// |                                  BOUNDARY_IN_SUFFIX5
		// |
		// C_BOUNDARY_IN
		//
		if (iState == C_BOUNDARY_IN)
		{
BOUNDARY_IN:
			sStartPos = szString;

			bFoundEndOfBoundary = false;

			for(;;)
			{
				// Boundary not found, error
				if (*szBoundaryPos != *szString)
				{
					iState        = iPrevState;
					szBoundaryPos = sBoundary.data();
//oLogger.Error("NOT-A-BOUNDARY");
					break;
				}

				++szBoundaryPos; ++szString;

				// Found end of boundary, okay
				if (*szBoundaryPos == '\0') { bFoundEndOfBoundary = true; break; }

				// Found part of boundary
				if (szString == szStringEnd) { break; }
			}
			if (!bFoundEndOfBoundary) { sBuffer.append(sStartPos, szString - sStartPos); }

			// Store part of boundary in temp. buffer
			if (szString == szStringEnd) { return; }

			// Check next state
			if (bFoundEndOfBoundary)
			{
				// Check BOUNDARY_IN_SUFFIX1
				if (*szString == '-')
				{
					iState = C_BOUNDARY_IN_SUFFIX1;
					++szString;
BOUNDARY_IN_SUFFIX1:
					// Store part of boundary in temp. buffer
					if (szString == szStringEnd)
					{
						sBuffer.append(sStartPos, szString - sStartPos);
						return;
					}

					// Return to prev. state
					if (*szString != '-') { iState = iPrevState; }
					// Check BOUNDARY_IN_SUFFIX2
					else
					{
						iState = C_BOUNDARY_IN_SUFFIX2;
						++szString;
BOUNDARY_IN_SUFFIX2:
						// Store part of boundary in temp. buffer
						if (szString == szStringEnd)
						{
							sBuffer.append(sStartPos, szString - sStartPos);
							return;
						}

						// Return to prev. state
						if (*szString != '\r') { iState = iPrevState; }
						// Check BOUNDARY_IN_SUFFIX3
						else
						{
							iState = C_BOUNDARY_IN_SUFFIX3;
							++szString;
BOUNDARY_IN_SUFFIX3:
							// Store part of boundary in temp. buffer
							if (szString == szStringEnd)
							{
								sBuffer.append(sStartPos, szString - sStartPos);
								return;
							}

							// Return to prev. state
							if (*szString != '\n') { iState = iPrevState; }
							// Check BOUNDARY_IN_SUFFIX4
							else
							{
								iState = C_BOUNDARY_IN_SUFFIX4;
								++szString;
BOUNDARY_IN_SUFFIX4:
								// Commit section
								CommitSection();

								if (szString == szStringEnd)
								{
									iState = C_END_MIME;
//oLogger.Error("END-OF-MIME AND END-OF-BUFFER");
									return;
								}

//oLogger.Error("END-OF-MIME AND NOT END-OF-BUFFER");
								return;
							}
						}
					}
				}
				// Check BOUNDARY_IN_SUFFIX5
				if (*szString == '\r')
				{
					iState = C_BOUNDARY_IN_SUFFIX5;
					++szString;
BOUNDARY_IN_SUFFIX5:
					// Store part of boundary in temp. buffer
					if (szString == szStringEnd)
					{
						sBuffer.append(sStartPos, szString - sStartPos);
						return;
					}

					// Return to prev. state
					if (*szString != '\n') { iState = iPrevState; }
					// Check BOUNDARY_IN_SUFFIX6
					else
					{
						// End of MIME part of message reached
						iState = C_BOUNDARY_IN_SUFFIX6;
						++szString;
BOUNDARY_IN_SUFFIX6:
//oLogger.Error("END-OF-MIME-PART");

						// Commit section
						CommitSection();
					}
				}
				// Return to prev. state
				else { iState = iPrevState; }
			}
		}

		// C_HEADER
		// Header: [space] HeaderValue; optional="field" \r - \n [ \r - \n ]
		// |        |      |                              |    |    |    |
		// |        |      |                              |    |    |    C_HEADER_LF2
		// |        |      |                              |    |    |
		// |        |      |                              |    |    HEADER_CR2
		// |        |      |                              |    |
		// |        |      |                              |    HEADER_LF
		// |        |      |                              |
		// |        |      |                              HEADER_CR
		// |        |      |
		// |        |      C_KEY_VALUE
		// |        |
		// |        C_KEY_PRE_VALUE
		// |
		// C_KEY_NAME
		//
		// New MIME part
		if (iState == C_BOUNDARY_IN_SUFFIX6 ||
		    iState == C_HEADER ||
		    iState == C_KEY_NAME)
		{
			iState = C_HEADER;
HEADER:
//oLogger.Error("NEW MIME PART");

			if (szString == szStringEnd) { return; }
			sStartPos = szString;

			for (;;)
			{
				// Check \r\n
				if (*szString != '\r')
				{
					iState = C_KEY_NAME;
KEY_NAME:
					if (szString == szStringEnd) { return; }
					sStartPos = szString;

					for(;;)
					{
						// End of key
						if (*szString == ':') { bFoundEndOfKey = true; break; }
						// Invalid header
						if (*szString == '\r') { break; }
						++szString;

						// Found end of buffer
						if (szString == szStringEnd) { return; }
					}
					sBuffer.append(sStartPos, szString - sStartPos);

					if (!bFoundEndOfKey)
					{
						// Clear buffer
						sBuffer.erase();
//oLogger.Error("INVALID-HEADER-KEY");
					}
					else
					{
//oLogger.Error("HEADER-KEY `%s`", sBuffer.data());
						// Compare header name
						if (strncasecmp(C_CONTENT_DISPOSITION, sBuffer.data(), sBuffer.size()) == 0)
						{
//oLogger.Error("CONTENT-DISPOSITION");
							iHeaderKeyType = C_KEY_CONTENT_DISPOSITION;
						}
						else if (strncasecmp(C_CONTENT_TYPE, sBuffer.data(), sBuffer.size()) == 0)
						{
//oLogger.Error("CONTENT-TYPE");
							iHeaderKeyType = C_KEY_CONTENT_TYPE;
						}

						iState = C_KEY_SPACE;
						++szString;
						// Clear buffer
						sBuffer.erase();
KEY_SPACE:
						if (szString == szStringEnd) { break; }
						sStartPos = szString;

						for(;;)
						{
							// Invalid header
							if (*szString == '\r') { break; }
							// End of key
							if (*szString != ' ') { bFoundEndOfSpace = true; break; }
							++szString;

							// Found end of buffer
							if (szString == szStringEnd) { return; }
						}

						if (!bFoundEndOfSpace)
						{
							iState = C_KEY_NAME;
//oLogger.Error("INVALID-HEADER-SPACE");
						}
						else
						{
							iState = C_KEY_VALUE;
KEY_VALUE:
							if (szString == szStringEnd) { break; }
							sStartPos = szString;

							for(;;)
							{
								// Invalid header
								if (*szString == '\r') { break; }
								++szString;

								// Found end of buffer
								if (szString == szStringEnd) { return; }
							}
							sBuffer.append(sStartPos, szString - sStartPos);
//oLogger.Error("HEADER-VALUE `%s`", sBuffer.data());
							// Parse header
							if      (iHeaderKeyType == C_KEY_CONTENT_DISPOSITION) { ParseContentDisposition(); }
							else if (iHeaderKeyType == C_KEY_CONTENT_TYPE)        { oFile.content_type = sBuffer; }

							// Clear buffer
							sBuffer.erase();
						}
					}
				}

				// Check \r\n
				if (*szString == '\r')
				{
					iState = C_HEADER_CR;
					++szString;
HEADER_CR:
					// End of buffer
					if (szString == szStringEnd) { return; }

					// Check error
					if (*szString != '\n') { iState = C_ERROR; return; }
					// Line feed
					else
					{
						iState = C_HEADER_LF;
						++szString;
HEADER_LF:
						// End of buffer
						if (szString == szStringEnd) { return; }

						// Check \r\n, end of headers
						if (*szString == '\r')
						{
							iState = C_HEADER_CR2;
							++szString;
HEADER_CR2:
							// End of buffer
							if (szString == szStringEnd) { return; }

							// Check error
							if (*szString != '\n') { iState = C_ERROR; return; }
							{
								iState = C_HEADER_LF2;
								++szString;
HEADER_LF2:
								// End of buffer
								if (szString == szStringEnd) { return; }

								iState = oFile.full_filename.empty() ? C_BODY_VALUE : C_BODY_FILE;
								oFile.filesize = 0;
//oLogger.Error("END-OF-ALL-HEADERS-%s", oFile.full_filename.empty() ? "STORE-FIELD" : "WRITE-FILE");
								break;
							}
						}
						else
						{
							iState = C_KEY_NAME;
						}
//oLogger.Error("END-OF-HEADER");
					}
				}

				if (szString == szStringEnd) { return; }
			}
		}

		// This is file
		if (iState == C_BODY_FILE)
		{
BODY_FILE:
			// Write chunk
			WriteFile(sBuffer.data(), sBuffer.data() + sBuffer.size());
			sBuffer.erase();

			if (szString == szStringEnd) { break; }

			sStartPos = szString;
			for (;;)
			{
				if (*szString == '\r')
				{
					iPrevState = C_BODY_FILE;
					iState     = C_BOUNDARY_IN;
					szBoundaryPos = sBoundary.data();
//oLogger.Error("FILE-CHECK-BOUNDARY");
					break;
				}

				++szString;
				if (szString == szStringEnd) { break; }
			}

			// Write chunk
			WriteFile(sStartPos, szString);
		}

		// This is value
		if (iState == C_BODY_VALUE)
		{
BODY_VALUE:
			if (szString == szStringEnd) { break; }

			sStartPos = szString;
			for (;;)
			{
				if (*szString == '\r')
				{
					iPrevState = C_BODY_VALUE;
					iState     = C_BOUNDARY_IN;
					szBoundaryPos = sBoundary.data();
//oLogger.Error("VALUE-CHECK-BOUNDARY");
					break;
				}

				++szString;
				if (szString == szStringEnd) { break; }
			}

			// Store chunk
			sBuffer.append(sStartPos, szString - sStartPos);
		}
	}
}

//
// Get parser state
//
INT_32 MultipartParser::GetState() const
{
	switch (iState)
	{
		case C_ERROR:
			return -1;

		case C_END_MIME:
			return 1;

		default:
			return 0;
	}
}

//
// End of parsing process
//
void MultipartParser::ParseDone()
{
	;;
}

//
// Get error description
//
STLW::string MultipartParser::GetError() const { return ""; }

//
// A destructor
//
MultipartParser::~MultipartParser() throw()
{
	;;
}

} // namespace CAS
// End.
