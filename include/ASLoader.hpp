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
 *      ASLoader.hpp
 *
 * $CAS$
 */
#ifndef _AS_LOADER_HPP__
#define _AS_LOADER_HPP__ 1

/**
  @file ASLoader.hpp
  @brief Application server generic object loader
*/
//#include <ASLoadableObject.hpp>
#include "ASTypes.hpp"
#include "ASException.hpp"
#include "STLMap.hpp"
#include "STLString.hpp"

#include <dlfcn.h>
#include <errno.h>

namespace CAS // C++ Application Server
{

/**
  @class ASLoader ASLoader.hpp <ASLoader.hpp>
  @brief Application server object loader
*/
template<typename T>class ASLoader
{
public:
	/**
	  @brief Constructor
	  @param sILibraryPrefix - Library loader prefix
	*/
	ASLoader(const STLW::string & sILibraryPrefix);

	/**
	  @brief Load library from disk
	  @param szLibraryName - path to the library
	*/
	INT_32 LoadLibrary(CCHAR_P szLibraryName);

	/**
	  @brief Get pointer to the object
	  @param szLibraryName - dynamic library file name
	  @param szClassName - class name
	  @return pointer to the object
	*/
	T * GetObject(CCHAR_P szLibraryName, CCHAR_P szClassName);

	/**
	  @brief Get object with type check
	  @param szLibraryName - dynamic library file name
	  @param szClassName - class name
	  @return pointer to the object
	*/
	template <typename TT> TT * GetCastedObject(CCHAR_P szLibraryName, CCHAR_P szClassName)
	{
		return dynamic_cast<TT *>(GetObject(szLibraryName, szClassName));
	}

	/**
	  @brief A destructor
	*/
	~ASLoader() throw();

private:
	// Does not exist
	ASLoader(const ASLoader  & oRhs);
	ASLoader& operator=(const ASLoader  & oRhs);

	typedef T * ((*InitPtr)());

	/**
	  @struct HandleInfo
	  @brief Dynamic-loaded library hadnle storage
	*/
	struct HandleInfo
	{
		/** DLL handle          */
		void        * handle;
		/** Number of references */
		INT_32        refcount;
	};

	/** Library loader prefix               */
	const STLW::string                            sLibraryPrefix;
	/** Library name-to-dl handle map       */
	STLW::map<STLW::string, HandleInfo>           mLibraryMap;

	/**
	  @brief Check library handle in cache
	  @param szLibraryName - library file name
	  @return 0 - if found, -1 - otherwise
	*/
	HandleInfo * CheckLibraryHandle(CCHAR_P szLibraryName);

	/**
	  @brief Load library
	  @param szLibraryName - library file name
	*/
	void * DoLoadLibrary(CCHAR_P szLibraryName);
};

//
// Constructor
//
template <typename T>ASLoader<T>::ASLoader(const STLW::string & sILibraryPrefix): sLibraryPrefix(sILibraryPrefix)
{
	;;
}

//
// Load library from disk
//
template <typename T>INT_32 ASLoader<T>::LoadLibrary(CCHAR_P szLibraryName)
{
	// Check library in cache
	HandleInfo * pHandleInfo = CheckLibraryHandle(szLibraryName);

	// Found library
	if (pHandleInfo != NULL) { return -1; }

	// Try to load library
	if (DoLoadLibrary(szLibraryName) == NULL) { return -1; }

	pHandleInfo = CheckLibraryHandle(szLibraryName);
	++(pHandleInfo -> refcount);

return 0;
}

//
// Get pointer to the object
//
template <typename T> T * ASLoader<T>::GetObject(CCHAR_P szLibraryName, CCHAR_P szClassName)
{
	// Check cache
	HandleInfo * pHandleInfo = CheckLibraryHandle(szLibraryName);
	// Locad library
	if (pHandleInfo == NULL)
	{
		DoLoadLibrary(szLibraryName);
		pHandleInfo = CheckLibraryHandle(szLibraryName);
	}

	STLW::string sInitHandler(szClassName);
	sInitHandler.append(sLibraryPrefix);

	// Initialize class
#if defined(linux) || defined(sun)
	// Linux brain-damaged stupidity
	void * vPtr = dlsym(pHandleInfo -> handle, sInitHandler.c_str());
	InitPtr vVInitPtr;
	memcpy((void *)(&vVInitPtr), (void *)(&vPtr), sizeof(void *));
#else
	InitPtr vVInitPtr = (InitPtr)dlfunc(pHandleInfo -> handle, sInitHandler.c_str());
#endif

	if (vVInitPtr == NULL)
	{
		STLW::string sTMP("Cannot get magic symbol \"");
		sTMP.append(sInitHandler);
		sTMP.append("\" from library \"");
		sTMP.append(szLibraryName);
		sTMP.append("\"");
		throw UnixException(sTMP.c_str(), errno);
	}
	// Number of references
	++(pHandleInfo -> refcount);

	// Create object
	T * pASObject = ((*vVInitPtr)());

	// All done
	if (pASObject != NULL) { return pASObject; }

	STLW::string sTMP("Internal error in module \"");
	sTMP.append(szClassName);
	sTMP.append("\", library \"");
	sTMP.append(szLibraryName);
	sTMP.append("\"");
	throw UnixException(sTMP.c_str(), errno);

// Make compiler happy; this should *not* happened
return NULL;
}

//
// Check libraru handle in cache
//
template <typename T> typename ASLoader<T>::HandleInfo * ASLoader<T>::CheckLibraryHandle(CCHAR_P szLibraryName)
{
	typename STLW::map<STLW::string, HandleInfo>::iterator itmLibraryMap = mLibraryMap.find(szLibraryName);
	if (itmLibraryMap != mLibraryMap.end()) { return &(itmLibraryMap -> second); }

return NULL;
}

//
// Load library
//
template <typename T>void * ASLoader<T>::DoLoadLibrary(CCHAR_P szLibraryName)
{
	HandleInfo oHandleInfo;
	// Okay, try to load function
	oHandleInfo.handle   = dlopen(szLibraryName, RTLD_NOW | RTLD_GLOBAL);
	oHandleInfo.refcount = 0;
	// Error?
	if (oHandleInfo.handle == NULL)
	{
		STLW::string sTMP("Cannot open library \"");
		sTMP.append(szLibraryName);
		sTMP.append("\": ");
		sTMP.append(dlerror());
		throw UnixException(sTMP.c_str(), errno);
	}

	mLibraryMap[szLibraryName] = oHandleInfo;

return oHandleInfo.handle;
}

//
// A destructor
//
template <typename T>ASLoader<T>::~ASLoader() throw()
{
#ifdef linux
	typename STLW::map<STLW::string, HandleInfo>::iterator itmLibraryMap = mLibraryMap.begin();
	while (itmLibraryMap != mLibraryMap.end())
	{
		for(INT_32 iI = 0; iI < itmLibraryMap -> second.refcount; ++iI)
		{
			dlclose(itmLibraryMap -> second.handle);
		}

		++itmLibraryMap;
	}
#endif
}

} // namespace CAS
#endif // _AS_LOADER_HPP__
// End.
