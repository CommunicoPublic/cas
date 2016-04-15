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
 *      ASApache2XWorker.h
 *
 * $CAS$
 */
#ifndef _AS_APACHE_2X_WORKER_H__
#define _AS_APACHE_2X_WORKER_H__ 1

/**
  @file ASApache2XWorker.h
  @brief Utilitues & helpers for Apache 2.X module
*/

#include "ASSysHeaders.h"

struct request_rec;
struct server_rec;

#ifdef __cplusplus
extern "C"
{
#endif

/**
  @fn void * ASInitServerManager(server_rec * sServerRec, char * szGlobalConfig, int * iRC)
  @brief Initialize CAS server manager
  @param sServerRec - Apache 2.X server record
  @param szGlobalConfig - global configuration file
  @param iRC - return code
  @return Pointer to opaque object
*/
void * ASInitServerManager(server_rec * sServerRec, char * szGlobalConfig, int * iRC);

/**
  @fn int ASInitHost(void * vOpaqueServerManager, request_rec * vRequest, char * szHostConfig)
  @brief Initialize CAS server manager
  @param vOpaqueServerManager - opaque data object
  @param vRequest - Apache 2.X request object
  @param szHostConfig - host configuration file
  @return pointer to opaque ASServer instance
*/
void * ASInitHost(void * vOpaqueServerManager, request_rec * vRequest, char * szHostConfig);

/**
  @fn int ASCheckLocation(void * vOpaqueServer, request_rec * vRequest)
  @brief Check location
  @param vOpaqueServer - opaque data object
  @param vRequest - Apache 2.X request object
  @return 0 - if success, -1 - if any erro occured
*/
int ASCheckLocation(void * vOpaqueServer, request_rec * vRequest);

/**
  @fn int ASProcessRequest(void * vOpaqueServer, request_rec * vRequest)
  @brief Process request
  @param vOpaqueServer - opaque data object
  @param vRequest - Apache 2.X request object
  @return 0 - if success, -1 - if any erro occured
*/
int ASProcessRequest(void * vOpaqueServer, request_rec * vRequest);

/**
  @fn void PostRequestCleanup(void * vOpaqueServer)
  @param vOpaqueServer - opaque data object
  @brief Post-request cleanup
*/
void PostRequestCleanup(void * vOpaqueServer);

/**
  @fn void ASShutdownServerManager(void * vOpaqueServerManager)
  @brief Shut down CAS server manager
  @param vOpaqueServerManager - opaque data object
*/
void ASShutdownServerManager(void * vOpaqueServerManager);

/**
  @fn void ASShutdownServer(void * vOpaqueServer)
  @brief Shut down CAS server
  @param vOpaqueServer - opaque data object
*/
void ASShutdownServer(void * vOpaqueServer);

#ifdef __cplusplus
}
#endif

#endif /* _AS_APACHE_2X_WORKER_H__ */
/* End. */
