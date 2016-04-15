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
 *      ASFilePool.hpp
 *
 * $CAS$
 */
#ifndef _AS_FILE_POOL_HPP__
#define _AS_FILE_POOL_HPP__ 1

/**
  @file ASFilePool.hpp
  @brief Application uploaded files pool
*/
#include "ASFile.hpp"

#include "STLVector.hpp"
#include "STLMap.hpp"

namespace CAS // C++ Application Server
{

// TBD
//typedef STLW::map<STLW::string, ASFile> ASFilePool;

/**
  @class ASFilePool ASFilePool.hpp <ASFilePool.hpp>
  @brief Pool with uploaded files
*/
class ASFilePool
{
private:
	typedef STLW::map<STLW::string, STLW::vector<ASFile> > ASFileStorage;

public:

	/**
	  @brief Add file to pool
	  @param oFile - file to add
	*/
	void AddFile(const ASFile  & oFile);

	/**
	  @class Iterator ASFilePool.hpp <ASFilePool.hpp>
	  @brief Pool iterator
	*/
	class Iterator
	{
	private:
		friend class ASFilePool;
		/** File storage iterator */
		ASFileStorage::iterator  oStorageIterator;
		/** Position in vector    */
		UINT_32                  iPos;
		/**
		  @brief Constructor
		  @param oIStorageIteartor - storage iterator
		*/
		Iterator(const ASFileStorage::iterator & oIStorageIteartor);
	public:
		/**
		  @brief Access operator
		  @return pointer to object
		*/
		ASFile * operator->();

		/**
		  @brief Pre-increment operator ++
		*/
		Iterator & operator++();

		/**
		  @brief Post-increment operator ++
		*/
		Iterator   operator++(int);

		/**
		  @brief Comparison operator
		  @brief oRhs - value to compare
		*/
		bool operator==(const Iterator & oRhs);

		/**
		  @brief Comparison operator
		  @brief oRhs - value to compare
		*/
		bool operator!=(const Iterator & oRhs);
	};

	/**
	  @brief Find file by name
	  @param sName - file name
	  @return List iterator
	*/
	STLW::pair<Iterator, Iterator> Find(const STLW::string & sName);

	/**
	  @brief Get start of file list
	  @return List iterator
	*/
	Iterator Begin();

	/**
	  @brief Get start of file list
	  @return List iterator
	*/
	Iterator End();

	/**
	  @brief A destructor
	*/
	~ASFilePool() throw();

private:
	friend class ASFilePool::Iterator;
	/** File storage */
	ASFileStorage    oASFileStorage;
};

} // namespace CAS
#endif // _AS_FILE_POOL_HPP__
// End.
