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
 *      ASConfigErrorHandler.cpp
 *
 * $CAS$
 */
#include "ASConfigErrorHandler.hpp"

#include <stdio.h>

namespace CAS // C++ Application Server
{

//
// Invalid XML tag attribute
//
STLW::string ASConfigErrorHandler::InvalidAttribute(const STLW::string  & sTag,
                                                    const STLW::string  & sAttribute,
                                                    const UINT_32       & iLine,
                                                    const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("Invalid attribute \"") + sAttribute + "\" of tag \"" + sTag + "\" " + szBuffer;
}

//
// Invalid XML tag
//
STLW::string ASConfigErrorHandler::InvalidTag(const STLW::string  & sTag,
                                              const UINT_32       & iLine,
                                              const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("Invalid tag \"") + sTag + "\" " + szBuffer;
}

//
// Invalid XML tag attribute
//
STLW::string ASConfigErrorHandler::MissedAttribute(const STLW::string  & sTag,
                                                   const STLW::string  & sAttribute,
                                                   const UINT_32       & iLine,
                                                   const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("Missed or empty attribute \"") + sAttribute + "\" of tag \"" + sTag + "\" " + szBuffer;
}

//
// Missed XML tag
//
STLW::string ASConfigErrorHandler::MissedTag(const STLW::string  & sTag,
                                             const UINT_32       & iLine,
                                             const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("Missed or empty tag \"") + sTag + "\" " + szBuffer;
}

//
// Internal XML parser error
//
STLW::string ASConfigErrorHandler::InternalError(const STLW::string  & sReason,
                                                 const UINT_32       & iLine,
                                                 const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string(sReason) + "\" " + szBuffer;
}

//
// Extra XML tag attribute
//
STLW::string ASConfigErrorHandler::ExtraAttribute(const STLW::string  & sTag,
                                                  const STLW::string  & sAttribute,
                                                  const UINT_32       & iLine,
                                                  const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("Extra attribute \"") + sAttribute + "\" of tag \"" + sTag + "\" " + szBuffer;
}

//
// Extra XML tag
//
STLW::string ASConfigErrorHandler::ExtraTag(const STLW::string  & sTag,
                                             const UINT_32      & iLine,
                                             const UINT_32      & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("Extra tag \"") + sTag + "\" " + szBuffer;
}

//
// Invalid XML attribute value
//
STLW::string ASConfigErrorHandler::InvalidAttributeValue(const STLW::string  & sTag,
                                                         const STLW::string  & sAttribute,
                                                         const STLW::string  & sAttributeValue,
                                                         const UINT_32       & iLine,
                                                         const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("Invalid attribute value \"") + sAttributeValue + "\" of attribute \"" + sAttribute + "\" of tag \"" + sTag + "\" " + szBuffer;
}

//
// Invalid XML tag value
//
STLW::string ASConfigErrorHandler::InvalidTagValue(const STLW::string  & sTag,
                                                   const STLW::string  & sTagValue,
                                                   const UINT_32       & iLine,
                                                   const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("Invalid tag value \"") + sTagValue + "\" of tag \"" + sTag + "\" " + szBuffer;
}

//
// Tag processing error
//
STLW::string ASConfigErrorHandler::TagProcessingError(const STLW::string  & sTag,
                                                      const STLW::string  & sError,
                                                      const UINT_32       & iLine,
                                                      const UINT_32       & iPos)
{
	CHAR_8 szBuffer[1024 + 1];
	snprintf(szBuffer, 1024, "line %d, pos: %d", iLine, iPos);

return STLW::string("While processing tag \"") + sTag + "\" " + szBuffer + ": " + sError;
}

} // namespace CAS
// End.
