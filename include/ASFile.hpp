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
 *      ASFile.hpp
 *
 * $CAS$
 */
#ifndef _AS_FILE_HPP__
#define _AS_FILE_HPP__ 1

/**
  @file ASFile.hpp
  @brief RFC 1867 uploaded file
*/

#include "ASTypes.hpp"
#include "STLString.hpp"

namespace CAS // C++ Application Server
{

/**
  @struct ASFile ASFile.hpp <ASFile.hpp>
  @brief RFC 1867 uploaded file
*/
struct ASFile
{
	// Input name
	STLW::string     name;
	// Full filename with path
	STLW::string     full_filename;
	// File name
	STLW::string     filename;
	// Content-type
	STLW::string     content_type;
	// Content encoding
	STLW::string     content_transfer_encoding;
	// Temporary file name
	STLW::string     tmp_name;
	// File size (in bytes)
	UINT_64          filesize;
};

} // namespace CAS
#endif // _AS_FILE_HPP__
// End.
