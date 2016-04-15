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
 *      ASFilePool.cpp
 *
 * $CAS$
 */

#include "ASFilePool.hpp"

namespace CAS // C++ Application Server
{

//
// Add file to pool
//
void ASFilePool::AddFile(const ASFile  & oFile) { oASFileStorage[oFile.name].push_back(oFile); }

//
// Constructor
//
ASFilePool::Iterator::Iterator(const ASFileStorage::iterator & oIStorageIteartor): oStorageIterator(oIStorageIteartor), iPos(0)
{
	;;
}

//
// Access operator
//
ASFile * ASFilePool::Iterator::operator->() { return &(oStorageIterator -> second[iPos]); }

//
// Pre-increment operator ++
//
ASFilePool::Iterator & ASFilePool::Iterator::operator++()
{
	++iPos;
	if (iPos == oStorageIterator -> second.size())
	{
		++oStorageIterator;
		iPos = 0;
	}

return *this;
}

//
// Post-increment operator ++
//
ASFilePool::Iterator ASFilePool::Iterator::operator++(int)
{
	Iterator oTMP = *this;

	++iPos;
	if (iPos == oStorageIterator -> second.size())
	{
		++oStorageIterator;
		iPos = 0;
	}

return oTMP;
}

//
// Comparison operator
//
bool ASFilePool::Iterator::operator==(const Iterator & oRhs) { return (oStorageIterator == oRhs.oStorageIterator && iPos == oRhs.iPos); }

//
// Comparison operator
//
bool ASFilePool::Iterator::operator!=(const Iterator & oRhs) { return (oStorageIterator != oRhs.oStorageIterator || iPos != oRhs.iPos); }

//
// Find file by name
//
STLW::pair<ASFilePool::Iterator, ASFilePool::Iterator> ASFilePool::Find(const STLW::string & sName)
{
	ASFileStorage::iterator itElements = oASFileStorage.find(sName);
	// Nothing found
	if (itElements == oASFileStorage.end())
	{
		// Return empty range
		return STLW::pair<ASFilePool::Iterator, ASFilePool::Iterator>(itElements, itElements);
	}

	// Something found
	ASFilePool::Iterator oBegin(itElements);
	ASFilePool::Iterator oEnd(itElements);
	oEnd.iPos = itElements -> second.size();

// Return range
return STLW::pair<ASFilePool::Iterator, ASFilePool::Iterator>(oBegin, oEnd);
}

//
// Get start of file list
//
ASFilePool::Iterator ASFilePool::Begin()         { return Iterator(oASFileStorage.begin()); }

//
// Get start of file list
//
ASFilePool::Iterator ASFilePool::End() { return Iterator(oASFileStorage.end()); }

//
// A destructor
//
ASFilePool::~ASFilePool() throw() { ;; }

} // namespace CAS
// End.
