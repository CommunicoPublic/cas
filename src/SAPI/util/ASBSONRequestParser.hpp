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
 *      ASBSONRequestParser.hpp
 *
 * $CAS$
 */
#ifndef _AS_BSON_REQUEST_PARSER_HPP__
#define _AS_BSON_REQUEST_PARSER_HPP__ 1

// C++ Includes
#include "ASRequestParser.hpp"

#include <CDT.hpp>
//#include <CTPP2BSONParser.hpp>

#include "STLString.hpp"

#define C_PARSE_ERROR             -32700 // Parse error Invalid BSON was received by the server.
                                         // An error occurred on the server while parsing the BSON text.
#define C_INVALID_REQUEST         -32600 // Invalid Request The BSON sent is not a valid Request object.
#define C_METHOD_NOT_FOUND        -32601 // Method not found The method does not exist / is not available.
#define C_INVALID_PARAMS          -32602 // Invalid params Invalid method parameter(s).
#define C_INTERNAL_ERROR          -32603 // Internal error Internal BSON-RPC error.

#define C_IMPL_DEFINED_START_CODE -32099 // Server error Reserved for implementation-defined server-errors.
#define C_IMPL_DEFINED_END_CODE   -32000


namespace CAS // C++ Application Server
{
// FWD
class ASXMLParser;

/**
  @class ASBSONRequestParser ASBSONRequestParser.hpp <ASBSONRequestParser.hpp>
  @brief BSON-RPC request parser
*/
class ASBSONRequestParser:
  public ASRequestParser
{
public:
	/**
	  @brief Constructor
	  @param oResult - result data set
	*/
	ASBSONRequestParser(CTPP::CDT & oResult);

	/**
	  @brief Parse chunk of data
	  @param szString - start of buffer
	  @param szStringEnd - end of buffer
	*/
	void ParseChunk(CCHAR_P szString, CCHAR_P szStringEnd);

	/**
	  @brief Get parser state
	  @return 0 - if success, -1 - if any error occured, 1 - if parsing is finished
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
	~ASBSONRequestParser() throw();

private:
	/** BSON Parser       */
//	CTPP::CTPP2BSONParser  oCTPP2BSONParser;
	/** Return code       */
	INT_32                 iRC;
	/** Error description */
	STLW::string           sError;
	/** Raw request data  */
	STLW::string           sData;

};

#endif // _AS_BSON_REQUEST_PARSER_HPP__
} // namespace CAS
// End.
