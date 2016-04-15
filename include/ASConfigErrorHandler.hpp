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
 *      ASConfigErrorHandler.hpp
 *
 * $CAS$
 */

#ifndef _AS_CONFIG_ERROR_HANDLER_HPP__
#define _AS_CONFIG_ERROR_HANDLER_HPP__ 1

/**
  @file ASConfigErrorHandler.hpp
  @brief Application server XML parser errors
*/

#include "ASTypes.hpp"
#include "STLString.hpp"

namespace CAS // C++ Application Server
{

/**
  @class ASConfigErrorHandler ASConfigErrorHandler.hpp <ASConfigErrorHandler.hpp>
  @brief Generic error handler for XML configuration parser
*/
class ASConfigErrorHandler
{
public:
	/**
	  @brief Invalid XML tag attribute
	  @param sTag - tag name
	  @param sAttribute - attribute name
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string InvalidAttribute(const STLW::string  & sTag,
	                                     const STLW::string  & sAttribute,
	                                     const UINT_32       & iLine,
	                                     const UINT_32       & iPos);

	/**
	  @brief Invalid XML tag
	  @param sTag - tag name
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string InvalidTag(const STLW::string  & sTag,
	                               const UINT_32       & iLine,
	                               const UINT_32       & iPos);

	/**
	  @brief Attribute that need, not found
	  @param sTag - tag name
	  @param sAttribute - attribute name
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string MissedAttribute(const STLW::string  & sTag,
	                                    const STLW::string  & sAttribute,
	                                    const UINT_32       & iLine,
	                                    const UINT_32       & iPos);

	/**
	  @brief Tag that need, not found
	  @param sTag - tag name
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string MissedTag(const STLW::string  & sTag,
	                              const UINT_32       & iLine,
	                              const UINT_32       & iPos);

	/**
	  @brief Internal XML parser error
	  @param sReason - error description
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string InternalError(const STLW::string  & sReason,
	                                  const UINT_32       & iLine,
	                                  const UINT_32       & iPos);

	/**
	  @brief Extra XML attribute
	  @param sTag - tag name
	  @param sAttribute - attribute name
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string ExtraAttribute(const STLW::string  & sTag,
	                                   const STLW::string  & sAttribute,
	                                   const UINT_32       & iLine,
	                                   const UINT_32       & iPos);

	/**
	  @brief Extra XML tag
	  @param sTag - tag name
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string ExtraTag(const STLW::string  & sTag,
	                             const UINT_32       & iLine,
	                             const UINT_32       & iPos);

	/**
	  @brief Invalid attribute balue
	  @param sTag - tag name
	  @param sAttribute - attribute name
	  @param sAttributeValue - proper attribute value
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string InvalidAttributeValue(const STLW::string  & sTag,
	                                          const STLW::string  & sAttribute,
	                                          const STLW::string  & sAttributeValue,
	                                          const UINT_32       & iLine,
	                                          const UINT_32       & iPos);

	/**
	  @brief Invalid tag value
	  @param sTag - tag name
	  @param sTagValue - proper tag value
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string InvalidTagValue(const STLW::string  & sTag,
	                                    const STLW::string  & sTagValue,
	                                    const UINT_32       & iLine,
	                                    const UINT_32       & iPos);

	/**
	  @brief Error in processing tag contents
	  @param sTag - tag name
	  @param sError - error description
	  @param iLine - line of XML document
	  @param iPos - position in line
	  @return human-readable error description
	*/
	static STLW::string TagProcessingError(const STLW::string  & sTag,
	                                       const STLW::string  & sError,
	                                       const UINT_32       & iLine,
	                                       const UINT_32       & iPos);
private:
	// Nothing
};

} // namespace CAS
#endif // _AS_CONFIG_ERROR_HANDLER_HPP__
// End.
