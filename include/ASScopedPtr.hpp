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
 *      ASScopedPtr.hpp
 *
 * $CAS$
 */
#ifndef _AS_SCOPED_PTR_HPP__
#define _AS_SCOPED_PTR_HPP__ 1

#include <ASTypes.hpp>

namespace CAS
{
/**
  @class ScopedPtr ASScopedPtr.hpp <ASScopedPtr.hpp>
  @brief Scoped pointer
*/
template <typename T> class ScopedPtr
{
public:
	/**
	  @brief Constructor
	  @param pIObj - pointer to object
	*/
	ScopedPtr(T  * pIObj = NULL): pObj(pIObj) { ;; }

	/**
	  @brief Type cast constructor
	*/
	ScopedPtr & operator =(T  * pIObj)
	{
		if (pObj != NULL) { delete pObj; }
		pObj = pIObj;
	return *this;
	}

	/**
	  @brief Compare two pointers
	*/
	bool operator ==(const void * vPtr) { return pObj == vPtr; }

	/**
	  @brief Get pointer to the object
	*/
	T * operator->() { return pObj; }

	/**
	  @brief Get reference to the object
	*/
	T & operator*() { return *pObj; }

	/**
	  @brief Type cast operator
	*/
	operator T*() { return pObj; }

	/**
	  @brief Check object is NULL
	*/
	operator bool() { return pObj != NULL; }

	/**
	  @brief A destructor
	*/
	~ScopedPtr() throw() { delete pObj; }
private:
	// Does not exist
	ScopedPtr(const ScopedPtr  & oRhs);
	ScopedPtr& operator =(const ScopedPtr  & oRhs);

	/** Pointer to the object */
	T  * pObj;
};

} // namespace CAS
#endif // _AS_SCOPED_PTR_HPP__
// End.

