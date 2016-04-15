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
 *      ASPoolTest.cpp
 *
 * $CAS$
 */

// C++ Includes
#include <ASObject.hpp>
#include <ASPool.hpp>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

using namespace CAS; // C++ Application Server
using namespace CTPP;

class TestPoolObject:
  public ASObject
{
public:
	TestPoolObject();

	INT_32 GetInt();

	void SetInt(const INT_32  & iValue);

	~TestPoolObject() throw() { ;; }
private:
	INT_32       iValue;

	CCHAR_P GetObjectType() const;

	CCHAR_P GetObjectName() const;
};

TestPoolObject::TestPoolObject(): iValue(0) { ;; }

CCHAR_P TestPoolObject::GetObjectType() const { return "TestPoolObject"; }

CCHAR_P TestPoolObject::GetObjectName() const { return "TestPoolObject"; }

INT_32 TestPoolObject::GetInt() { return iValue; }

void TestPoolObject::SetInt(const INT_32  & iIValue) { iValue = iIValue; }

// /////////////////////////////////////////////////////
class AnotherPoolObject:
  public ASObject
{
public:
	AnotherPoolObject();

	W_FLOAT GetFloat();

	void SetFloat(const W_FLOAT  & iValue);

	~AnotherPoolObject() throw() { ;; }
private:
	W_FLOAT       iValue;

	CCHAR_P GetObjectType() const;

	CCHAR_P GetObjectName() const;

};

AnotherPoolObject::AnotherPoolObject(): iValue(0) { ;; }

CCHAR_P AnotherPoolObject::GetObjectType() const { return "AnotherPoolObject"; }

CCHAR_P AnotherPoolObject::GetObjectName() const { return "AnotherPoolObject"; }

W_FLOAT AnotherPoolObject::GetFloat() { return iValue; }

void AnotherPoolObject::SetFloat(const W_FLOAT  & iIValue) { iValue = iIValue; }

int main(int argc, char ** argv)
{
	ASPool oPool;

	// Create new object
	TestPoolObject * pObject1 = new TestPoolObject;
	fprintf(stderr, "Object1: %p\n", (void *)pObject1);

	// Register resource in pool
	UINT_32 iResourceId1 = oPool.RegisterResource<TestPoolObject>(pObject1, "test_pool_object");
	fprintf(stderr, "Resource ID1: %d\n", iResourceId1);

	// Create another object
	AnotherPoolObject * pObject2 = new AnotherPoolObject;
	fprintf(stderr, "Object2: %p\n", (void *)pObject2);

	// Register resource in pool
	UINT_32 iResourceId2 = oPool.RegisterResource<AnotherPoolObject>(pObject2, "another_pool_object");
	fprintf(stderr, "Resource ID2: %d\n", iResourceId2);

	// Get first object
	pObject1 = oPool.GetResourceByName<TestPoolObject>("test_pool_object");
	// Error?
	if (pObject1 == NULL) { fprintf(stderr, "ERROR: Cannot get resource from pool.\n"); return EX_SOFTWARE; }

	// Store value
	pObject1 -> SetInt(123);

	// Try to get first object with wrong type
	pObject1 = oPool.GetResourceByName<TestPoolObject>("another_pool_object");
	// Error?
	if (pObject1 != NULL) { fprintf(stderr, "ERROR: Pointer should be NULL.\n"); return EX_SOFTWARE; }

	// Get first object
	pObject1 = oPool.GetResource<TestPoolObject>(iResourceId1);
	// Error?
	if (pObject1 == NULL) { fprintf(stderr, "ERROR: Cannot get resource from pool.\n"); return EX_SOFTWARE; }

	// Check value
	fprintf(stderr, "Integer value: %d\n", pObject1 -> GetInt());

	// Get second object
	pObject2 = oPool.GetResource<AnotherPoolObject>(iResourceId2);
	// Error?
	if (pObject2 == NULL) { fprintf(stderr, "ERROR: Cannot get resource from pool.\n"); return EX_SOFTWARE; }

	// Store value
	pObject2 -> SetFloat(123.456);

	// Try to register another object with same name
	pObject2 = NULL;
	oPool.RegisterResource<AnotherPoolObject>(pObject2, "another_pool_object");

	// Get second object
	pObject2 = oPool.GetResource<AnotherPoolObject>(iResourceId2);
	// Error?
	if (pObject2 == NULL) { fprintf(stderr, "ERROR: Cannot get resource from pool.\n"); return EX_SOFTWARE; }

	// Check value
	fprintf(stderr, "Float value: %f\n", pObject2 -> GetFloat());

	// Remove resource from pool
	oPool.RemoveResource<TestPoolObject>("test_pool_object");
	// Destroy object
	delete pObject1;

	// Try to get first object with wrong type
	pObject1 = oPool.GetResourceByName<TestPoolObject>("test_pool_object");
	// Error?
	if (pObject1 != NULL) { fprintf(stderr, "ERROR: Pointer should be NULL.\n"); return EX_SOFTWARE; }

	// Remove resource from pool
	oPool.RemoveResource<TestPoolObject>("another_pool_object");
	// Destroy object
	delete pObject2;

	fclose(stdin);
	fclose(stdout);
	fclose(stderr);

return EX_OK;
}
// End.
