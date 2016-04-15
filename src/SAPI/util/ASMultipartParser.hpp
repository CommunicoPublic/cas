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
 *      ASMultipartParser.hpp
 *
 * $CAS$
 */
#ifndef _AS_MULTIPART_PARSER_HPP__
#define _AS_MULTIPART_PARSER_HPP__ 1

// C++ Includes
#include "ASRequestParser.hpp"

#include "ASFilePool.hpp"
#include "ASServer.hpp"

#include <stdio.h>

namespace CAS // C++ Application Server
{
// FWD
class ASLogger;
class ASServer;
//struct ASServer::ASRequestContext;
class ASRequest;

/**
  @class MultipartParser ASMultipartParser.hpp <ASMultipartParser.hpp>
  @brief RFC1867 Multipart POST requeset parser
*/
class MultipartParser:
  public ASRequestParser
{
public:
	/**
	  @brief A constructor
	  @param oIData - Request variables collector
	  @param mIFiles - uploaded files collector
	  @param vIFilelist - list of uploaded files
	  @param sIUploadDir - upload directory
	  @param szIBoundary - boundary
	  @param iIMaxFileSize - max. file size
	  @param oILogger - logger object
	*/
	MultipartParser(CTPP::CDT                   & oIData,
	                ASFilePool                  & mIFiles,
	                STLW::vector<STLW::string>  & vIFilelist,
	                const STLW::string          & sIUploadDir,
	                const STLW::string          & sIBoundary,
	                const INT_64                & iIMaxFileSize,
	                ASServer                    & oServer,
	                ASServer::ASRequestContext  & oRequestContext,
	                ASRequest                   & oRequest,
	                ASLogger                    & oILogger);

	/**
	  @brief Parse chunk of data
	  @return 0 - if success, -1 - if any error occured, 1 - if parsing is finished
	*/
	void ParseChunk(CCHAR_P szString, CCHAR_P szStringEnd);

	/**
	  @brief Get parser state
	*/
	INT_32 GetState() const;

	/**
	  @brief End of parsing process
	*/
	void ParseDone();

	/**
	  @brief Get error description
	*/
	STLW::string GetError() const;

	/**
	  @brief A destructor
	*/
	~MultipartParser() throw();

private:
	/** POST data                          */
	CTPP::CDT                   & oData;
	/** Uploaded files                     */
	ASFilePool                  & mFiles;
	/** List of uploaded files             */
	STLW::vector<STLW::string>  & vFilelist;

	/** Upload directory                   */
	STLW::string                  sUploadDir;

	/** Current uploaded file              */
	ASFile                        oFile;
	/** File to save                       */
	FILE                        * F;

	/** Temp buffer                        */
	STLW::string                  sBuffer;

	/** Boundary                           */
	STLW::string                  sBoundary;
	/** Position in boundary               */
	CCHAR_P                       szBoundaryPos;

	/** Max. fiule size                    */
	INT_32                        iMaxFileSize;

	/** Current parser state               */
	UINT_32                       iState;
	/** Previous state                     */
	UINT_32                       iPrevState;

	/** Header id                          */
	UINT_32                       iHeaderKeyType;

	/** Application server                 */
	ASServer                    & oServer;
	/** Application server request context */
	ASServer::ASRequestContext  & oRequestContext;
	/** Request object                     */
	ASRequest                   & oRequest;
	/** Logger object                      */
	ASLogger                    & oLogger;

	/**
	  @brief Parse Content-Disposition header
	*/
	void ParseContentDisposition();

	/**
	  @brief Commit parsing operation
	*/
	void CommitSection();

	/**
	  @brief Write buffer to the file
	*/
	void WriteFile(CCHAR_P  szStart,
	               CCHAR_P  szEnd);
};

#endif // _AS_MULTIPART_PARSER_HPP__
} // namespace CAS
// End.
