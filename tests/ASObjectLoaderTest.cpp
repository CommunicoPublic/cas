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
 *      ASObjectLoaderTest.hpp
 *
 * $CAS$
 */

#include <ASLoadableObject.hpp>

#include <stdio.h>

namespace TEST_NS
{

/**
  @class ASObject ASObject.hpp <ASObject.hpp>
  @brief Application server object
*/
class ASObjectLoaderTest:
  public CAS::ASObject
{
public:
	/**
	  @brief A constructor
	*/
	ASObjectLoaderTest();

	/**
	  @brief Get object type
	  @return human-readable type of object
	*/
	CCHAR_P GetObjectType() const;


	/**
	  @brief Get object name
	  @return human-readable name of object
	*/
	CCHAR_P GetObjectName() const;

	/**
	  @brief A destructor
	*/
	~ASObjectLoaderTest() throw();
};

EXPORT_HANDLER(ASObjectLoaderTest)

//
// A constructor
//
ASObjectLoaderTest::ASObjectLoaderTest()
{
	fprintf(stderr, "ASObjectLoaderTest::ASObjectLoaderTest\n");
}

//
// Get object type
//
CCHAR_P ASObjectLoaderTest::GetObjectType() const
{
	fprintf(stderr, "ASObjectLoaderTest::GetObjectType\n");

return "TestObject";
}

//
// Get object name
//
CCHAR_P ASObjectLoaderTest::GetObjectName() const
{
	fprintf(stderr, "ASObjectLoaderTest::GetObjectName\n");

return "TestObject";
}

//
// A destructor
//
ASObjectLoaderTest::~ASObjectLoaderTest() throw()
{
	fprintf(stderr, "ASObjectLoaderTest::~ASObjectLoaderTest\n");
}

} // namespace TEST_NS
// End.
