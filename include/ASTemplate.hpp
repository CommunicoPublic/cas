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
 *      ASTemplate.hpp
 *
 * $CAS$
 */
#ifndef _AS_TEMPLATE_HPP__
#define _AS_TEMPLATE_HPP__ 1

/**
  @file ASTemplate.hpp
  @brief Application server HTML template
*/

#include "ASTypes.hpp"
#include "STLString.hpp"

namespace CTPP
{
	// FWD
	class VMMemoryCore;
	class VMFileLoader;
	class VMDumper;
}

namespace CAS // C++ Application Server
{

/**
  @class ASTemplate ASTemplate.hpp <ASTemplate.hpp>
  @brief Application server HTML template
*/
class ASTemplate
{
public:
	/**
	  @enum eTemplateType ASTemplate.hpp <ASTemplate.hpp>
	  @brief Describes type of template: compiled bytecode or source
	*/
	enum eTemplateType { UNDEF, BYTECODE, SOURCE };

	/**
	  @brief Constructor
	  @param sITemplate - Template name as plaintext or Perl-compatible regular expression
	  @param oITemplateType - Template type: "plain" or "regexp"
	  @param vIncludeDirs - list of include directories
	*/
	ASTemplate(const STLW::string                & sITemplate,
	           const eTemplateType               & oITemplateType,
	           const STLW::vector<STLW::string>  & vIncludeDirs);

	/**
	  @brief Get pointer to bytecode object
	  @return Ready-to-execute pointer to bytecode object
	*/
	const CTPP::VMMemoryCore * GetBytecode() const;

	/**
	  @brief A destructor
	*/
	~ASTemplate() throw();
private:
	/** Template name                            */
	STLW::string                 sTemplate;
	/** Template type                            */
	eTemplateType                oTemplateType;
	/** Bytecode loader object                   */
	CTPP::VMFileLoader         * pLoader;
	/** Bytecode dumper object                   */
	CTPP::VMDumper             * pDumper;
	/** Pointer to memory core                   */
	const CTPP::VMMemoryCore   * pVMMemoryCore;

	/**
	  @brief Default constructor
	*/
	ASTemplate();

	/**
	  @brief Copy constructor
	*/
	ASTemplate(const ASTemplate & oRhs);

	/**
	  @brief Copy operator =
	*/
	ASTemplate & operator=(const ASTemplate & oRhs);
};

} // namespace CAS
#endif // _AS_TEMPLATE_HPP__
// End.
