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
 *      ASPool.hpp
 *
 * $CAS$
 */
#ifndef _AS_POOL_HPP__
#define _AS_POOL_HPP__ 1

/**
  @file ASPool.hpp
  @brief Application server generic pool implemenmtation
*/
#include "ASObject.hpp"

#include "STLMap.hpp"
#include "STLString.hpp"
#include "STLVector.hpp"

namespace CAS // C++ Application Server
{

/**
  @class ASPool ASPool.hpp <ASPool.hpp>
  @brief Application server object
*/
class ASPool
{
public:
	/**
	  @brief Constructor
	*/
	inline ASPool() { ;; }

	/**
	  @brief Remove resource from pool
	  @param sResourceName - resource to remove
	  @return 0 - if success, -1 - otherwise
	*/
	template <typename T> INT_32 RemoveResource(const STLW::string & sResourceName)
	{
		INT_32 iReturnCode = -1;

		STLW::map<STLW::string, UINT_64>::iterator itmIndex = mObjectIndex.find(sResourceName);
		if (itmIndex != mObjectIndex.end())
		{
			// Notify object
			vObjects.at(itmIndex -> second) -> HandleEvent(C_REMOVE_RESOURCE_FROM_POOL);
			// Remove object
			vObjects.at(itmIndex -> second) = NULL;

			mObjectIndex.erase(itmIndex);
			iReturnCode = 0;
		}

	return iReturnCode;
	}

	/**
	  @brief Register resource in pool
	  @param pResource - resource to register
	  @param sResourceName - unique resource name
	  @return Resource ID or -1 if any error occured (if any resource with same name already exists)
	*/
	template <typename T> INT_32 RegisterResource(T * pResource, const STLW::string & sResourceName)
	{
		INT_32 iResourceID = -1;

		STLW::map<STLW::string, UINT_64>::const_iterator itmIndex = mObjectIndex.find(sResourceName);
		if (itmIndex == mObjectIndex.end())
		{
			vObjects.push_back(pResource);

			// Notify object
			static_cast<ASObject * >(pResource) -> HandleEvent(C_REGISTER_RESOURCE_IN_POOL);

			iResourceID = vObjects.size() - 1;
			mObjectIndex[sResourceName] = iResourceID;
		}

	return iResourceID;
	}

	/**
	  @brief Get resource by name
	  @param sResourceName - resource name
	  @return pointer to resource, or NULL if any error occured
	*/
	template <typename T> T * GetResourceByName(const STLW::string & sResourceName) const
	{
		T * pObject = NULL;

		STLW::map<STLW::string, UINT_64>::const_iterator itmIndex = mObjectIndex.find(sResourceName);
		if (itmIndex != mObjectIndex.end())
		{
			ASObject * pTMP = vObjects.at(itmIndex -> second);
			pObject = dynamic_cast<T *>(pTMP);

			// Notify object, ONLY if cast is successfull
			if (pObject != NULL) { pTMP  -> HandleEvent(C_GET_RESOURCE_FROM_POOL); }
		}

	return pObject;
	}

	/**
	  @brief Check resource
	  @param sResourceName - resource name
	  @return 0 - if resource present, -1 - otherwise
	*/
	template <typename T> INT_32 CheckResourceByName(const STLW::string & sResourceName) const
	{
		STLW::map<STLW::string, UINT_64>::const_iterator itmIndex = mObjectIndex.find(sResourceName);

		if (itmIndex != mObjectIndex.end()) { return 0; }

	return -1;
	}

	/**
	  @brief Get resource by ID
	  @param iResourceId - resource ID
	  @return pointer to resource, or NULL if any error occured
	*/
	template <typename T> T * GetResource(const UINT_64 & iResourceId) const
	{
		ASObject * pTMP = vObjects.at(iResourceId);
		T * pObject = dynamic_cast<T *>(pTMP);

		// Notify object, ONLY if cast is successfull
		if (pObject != NULL) { pTMP -> HandleEvent(C_GET_RESOURCE_FROM_POOL); }

	return pObject;
	}

	/**
	  @brief Get number of objhects in pool
	  @return number of objhects in pool
	*/
	inline UINT_64 GetPoolSize() const { return vObjects.size(); }

	/**
	  @brief Clear pool
	*/
	template<typename T> void ClearPool()
	{
		STLW::map<STLW::string, UINT_64> mTMP; mObjectIndex.swap(mTMP);
		STLW::vector<ASObject *>         vTMP; vObjects.swap(vTMP);
	}

	/**
	  @brief Notify objects in pool about event
	  @param iEventId - event Id
	*/
	template<typename T> void NotifyObjects(const UINT_32 iEventId)
	{
		STLW::vector<ASObject *>::iterator itvObjects = vObjects.begin();
		while (itvObjects != vObjects.end())
		{
			(*itvObjects) -> HandleEvent(iEventId);
			++itvObjects;
		}
	}

	/**
	  @brief A destructor
	*/
	inline ~ASPool() throw() { ;; }
public:
	// Does not exist
	ASPool(const ASPool  & oRhs);
	ASPool& operator=(const ASPool  & oRhs);

	/** Object index                   */
	STLW::map<STLW::string, UINT_64>  mObjectIndex;
	/** Array of objects               */
	STLW::vector<ASObject *>          vObjects;
};

} // namespace CAS
#endif // _AS_POOL_HPP__
// End.
