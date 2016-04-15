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
 *      ASTemplatePathExpander.cpp
 *
 * $CAS$
 */

#include "ASTemplatePathExpander.hpp"
#include "ASException.hpp"
#include "ASPCRE.hpp"

#include <sys/stat.h>
#include <errno.h>

namespace CAS // C++ Application Server
{

//
// Constructor
//
ASTemplatePathExpander::ASTemplatePathExpander(const STLW::string  & sITemplate,
                                               const ePathType     & oIPathType): sTemplate(sITemplate),
                                                                                  oPathType(oIPathType)
{
	if (oPathType == REGEXP_PATH)
	{
		throw LogicError("ASTemplatePathExpander, oPathType == REGEXP_PATH: not developed yet");

		PCRE oPCRE(sTemplate.c_str());
		// Iterate through result
	}
	else
	{
		// Store template name
		vTemplateList.push_back(sTemplate);
	}
}

//
// Check URI
//
STLW::vector<STLW::string> ASTemplatePathExpander::GetTemplates() const { return vTemplateList; }

//
// Get original URI
//
STLW::string ASTemplatePathExpander::GetTemplateName() const { return sTemplate; }

//
// Get URI type
//
ASTemplatePathExpander::ePathType ASTemplatePathExpander::GetTemplateType() const { return oPathType; }

//
// A destructor
//
ASTemplatePathExpander::~ASTemplatePathExpander() throw()
{
	;;
}

} // namespace CAS
// End.
