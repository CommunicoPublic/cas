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
 *      ASNetworkRange.hpp
 *
 * $CAS$
 */
#ifndef _AS_NETWORK_RANGE_HPP__
#define _AS_NETWORK_RANGE_HPP__ 1

/**
  @file ASNetworkRange.hpp
  @brief Networks & IP ranges
*/

#include "ASTypes.hpp"
#include "STLVector.hpp"
#include "STLFunctional.hpp"

namespace CAS // C++ Application Server
{

/**
  @class ASNetworkRange ASNetworkRange.hpp <ASNetworkRange.hpp>
  @brief Search and store networks & IP ranges
*/
template <typename T>class NetworkRange
{
public:

	/**
	  @brief Add network
	  @param sNetwork - IP address or network
	  @param oData - Data to store
	  @return 0 - if success, -1 - if network is incorrect or if network intersects with prevoiusly stored
	*/
	INT_32 AddNetwork(const STLW::string & sNetwork, const T & oData)
	{
		UINT_32 iStartIP;
		UINT_32 iEndIP;
		if (DecodeNetwork(sNetwork, iStartIP, iEndIP) == -1) { return -1; }

		return AddRange(iStartIP, iEndIP, oData);
	}

	/**
	  @brief Add range of IP addresses
	  @param iStartIP - First IP address in range
	  @param iEndIP - Last IP address in range
	  @param oData - Data to store
	  @return 0 - if success, -1 - if network is incorrect or if network intersects with prevoiusly stored
	*/
	INT_32 AddRange(const UINT_32 iStartIP, const UINT_32 iEndIP, const T & oData)
	{
		T oTMP;
		// Check ranges
		if (SearchNetwork(iStartIP, iEndIP, oTMP) == 0) { return -1; }

		_NetworkRange oNetworkRange = {iStartIP, iEndIP, oData};

		vRanges.push_back(oNetworkRange);
		STLW::sort(vRanges.begin(), vRanges.end());

		return 0;
	}

	/**
	  @brief Search address in range
	  @param sIPAddress - IP address to search
	  @param oResult - Return value, if found
	  @return 0 - if success, -1 - if network is incorrect or if network intersects with prevoiusly stored
	*/
	INT_32 SearchAddress(const STLW::string & sIPAddress, T & oResult) const
	{
		UINT_32 iStartIP;
		UINT_32 iEndIP;
		if (DecodeNetwork(sIPAddress, iStartIP, iEndIP) == -1) { return -1; }

		return SearchAddress(iStartIP, oResult);
	}

	/**
	  @brief Search address in range
	  @param iIPAddress - IP address to search (int network byte order)
	  @param oResult - Result data, if address belongs to stored range
	  @return 0 - if found, -1 - otherwise
	*/
	INT_32 SearchAddress(const UINT_32 iIPAddress, T & oResult) const
	{
		return SearchNetwork(iIPAddress, iIPAddress, oResult);
	}

	/**
	  @brief Search first range intersection
	  @param iStartIP - First IP address in range
	  @param iEndIP - Last IP address in range
	  @param oResult - Result data, if address belongs to stored range
	  @return 0 - if found, -1 - otherwise
	*/
	INT_32 SearchNetwork(const UINT_32    iStartIP,
	                     const UINT_32    iEndIP,
	                     T              & oResult) const
	{
		INT_32 iStartPos = 0;
		INT_32 iEndPos   = vRanges.size();
		if (iEndPos == 0) { return -1; }

		// Recursive descending
		return SearchIntersection(iStartIP, iEndIP, iStartPos, iEndPos - 1, oResult);
	}
private:
	/**
	  @struct NetworkRange::_NetworkRange ASNetworkRange.hpp <ASNetworkRange.hpp>
	  @brief Network range storage
	*/
	struct _NetworkRange
	{
		/** First IP address in range */
		UINT_32    start_ip;
		/** Last IP address in range */
		UINT_32    end_ip;
		/** Stored data              */
		T          data;

		/**
		  @brief Comparator for std::sort
		  @param oRhs - object to compare
		  @return true if this range has smaller last IP
		*/
		bool operator < (const _NetworkRange & oRhs) const { return end_ip < oRhs.start_ip; }
	};

	/** Sorted list of all ranges */
	STLW::vector<_NetworkRange> vRanges;

	/**
	  @brief Get start & end IP addresses in network range
	  @param sIPAddress - IP address or network
	  @param iStartIP - First IP address in range
	  @param iEndIP - Last IP address in range
	  @return 0 - if success, -1 - if network is incorrect
	*/
	INT_32 DecodeNetwork(const STLW::string  & sIPAddress,
	                     UINT_32             & iStartIP,
	                     UINT_32             & iEndIP) const
	{
		iStartIP = iEndIP = 0;
		UINT_32 iNetmask = 0;
		STLW::string::const_iterator itsIPAddress = sIPAddress.begin();

		UINT_32 iDot = 0;
		bool bParseNetwork = false;
		for(;;)
		{
			if (itsIPAddress == sIPAddress.end()) { return -1; }

			{
				const UCHAR_8 ucTMP = *itsIPAddress;
				if (ucTMP < '0' || ucTMP > '9') { return -1; }
			}

			UINT_32 iOctet = 0;
			while (itsIPAddress != sIPAddress.end())
			{
				const UCHAR_8 ucTMP = *itsIPAddress;

				// Parse octet
				if (ucTMP >= '0' && ucTMP <= '9')
				{
					iOctet *= 10;
					iOctet += ucTMP - '0';
				}
				// Parse '.'
				else if (ucTMP == '.')
				{
					++itsIPAddress;
					break;
				}
				// Parse netmask
				else if (ucTMP == '/')
				{
					bParseNetwork = true;
					++itsIPAddress;
					break;
				}
				else { return -1; }

				++itsIPAddress;
			}

			++iDot;

			if (iOctet > 255) { return -1; }
			iStartIP <<= 8;
			iStartIP += iOctet;

			if (bParseNetwork)
			{
				while (itsIPAddress != sIPAddress.end())
				{
					const UCHAR_8 ucTMP = *itsIPAddress;
					if (ucTMP < '0' || ucTMP > '9') { return -1; }

					iNetmask *= 10;
					iNetmask += ucTMP - '0';

					++itsIPAddress;
				}
				if (iNetmask > 32) { return -1; }

				iStartIP <<= ((4 - iDot) * 8);

				iStartIP &= 0xFFFFFFFF ^ ((1 << (32 - iNetmask)) - 1);
				break;
			}

			// All done
			if (itsIPAddress == sIPAddress.end())
			{
				iStartIP <<= ((4 - iDot) * 8);
				break;
			}

			if (iDot > 4) { return -1; }
		}

		iEndIP = iStartIP + ((1 << (32 - iNetmask)) - 1);

		return 0;
	}

	/**
	  @brief Search first intersection between stored IUP range and given
	  @param iStartIP - First IP address in range
	  @param iEndIP - Last IP address in range
	  @param iStartPos - Starting position in sorted list
	  @param iEndPos - Ending position in sorted list
	  @param oResult - Result data, if found intersetion of ranges
	  @return 0 - if found, -1 - otherwise
	*/
	INT_32 SearchIntersection(const UINT_32    iStartIP,
	                          const UINT_32    iEndIP,
	                          const INT_32     iStartPos,
	                          const INT_32     iEndPos,
	                          T              & oResult) const
	{
		if (iStartPos > iEndPos) { return -1; }

		const INT_32 iMidPos = (iEndPos + iStartPos) / 2;
		const _NetworkRange & oRange = vRanges[iMidPos];

		/*
		 * start_ip       end_ip
		 * |--------------|
		 *
		 *      S            E
		 *      |------------|
		 */
		if (iStartIP >= oRange.start_ip)
		{
			if (iStartIP <= oRange.end_ip)
			{
				oResult = oRange.data;
				return 0;
			}

			// Search in next right
			return SearchIntersection(iStartIP, iEndIP, iMidPos + 1, iEndPos, oResult);
		}

		/*
		 *      start_ip       end_ip
		 *      |--------------|
		 *
		 * S   E
		 * |---|
		 */
		if (iEndIP < oRange.start_ip)
		{
			// Search in next left
			return SearchIntersection(iStartIP, iEndIP, iStartPos, iMidPos - 1, oResult);
		}

		oResult = oRange.data;
		return 0;
	}
};

} // namespace CAS
#endif // _AS_NETWORK_RANGE_HPP__
// End.
