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
 *      ASDataFilter.hpp
 *
 * $CAS$
 */
#ifndef _AS_DATA_FILTER_HPP__
#define _AS_DATA_FILTER_HPP__ 1

/**
  @file ASDataFilter.hpp
  @brief Various data filters
*/

#include "ASTypes.hpp"
#include "STLString.hpp"
#include "STLVector.hpp"

namespace CAS // C++ Application Server
{

/**
  @class ASDataFilter ASDataFilter.hpp <ASDataFilter.hpp>
  @brief Various data filters
*/
class ASDataFilter
{
public:
	/**
	  @brief Check subnet with mask
	  @param szSubnet - NULL-terminated string with subnet in form (network/netmask)
	  @return 0 - if success, -1 - otherwise
	*/
	static INT_32 IsSubnet(CCHAR_P szSubnet);

	/**
	  @brief Check IP address
	  @param szIP - NULL-terminated string with ip address in form (1.2.3.4)
	  @return 0 - if success, -1 - otherwise
	*/
	static INT_32 IsIPAddress(CCHAR_P szIP);

	/**
	  @brief Check IP belongs to specified subnet
	  @param szIP - NULL-terminated string with ip address in form (1.2.3.4)
	  @param szSubnet - NULL-terminated string with subnet in form (network/netmask)
	  @return 0 - if success, -1 - if IP is out of range, -2 - if subnet is invalid
	*/
	static INT_32 InSubnetRange(CCHAR_P szIP, CCHAR_P szSubnet);

	/**
	  @brief Check integer value
	  @param szValue - value to check
	  @return 0 - if success, -1 - otherwise
	*/
	static INT_32 IsInteger(CCHAR_P szValue);

	/**
	  @brief Check floating point value
	  @param szValue - value to check
	  @return 0 - if success, -1 - otherwise
	*/
	static INT_32 IsFloat(CCHAR_P szValue);

	/**
	  @brief Check regular expression
	  @param szRE - regular expression
	  @param szVal - value to check
	  @return 0 - if success, -1 - otherwise
	*/
	static INT_32 MatchRegexp(CCHAR_P szRE, CCHAR_P szVal);

private:
};

} // namespace CAS
#endif // _AS_DATA_FILTER_HPP__
// End.
