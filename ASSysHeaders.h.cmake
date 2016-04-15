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
 *      ASSysHeaders.h
 *
 * $CAS$
 */
#ifndef _AS_SYS_HEADERS_H__
#define _AS_SYS_HEADERS_H__ 1

/**
  @file ASSysHeaders.h
  @brief Application server system headers
*/

#cmakedefine HAVE_SYS_TYPES_H    1

#cmakedefine HAVE_SYS_TIME_H     1

#cmakedefine HAVE_SYS_UIO_H      1

#cmakedefine HAVE_FCNTL_H        1

#cmakedefine HAVE_MATH_H         1

#cmakedefine HAVE_STDIO_H        1

#cmakedefine HAVE_STDLIB_H       1

#cmakedefine HAVE_STRING_H       1

#cmakedefine HAVE_TIME_H         1

#cmakedefine HAVE_UNISTD_H       1

#cmakedefine HAVE_SYSEXITS_H     1

#cmakedefine DEBUG_MODE          1

#cmakedefine NO_STL_STD_PREFIX   1

#cmakedefine HAVE_DLFCN_H        1

#cmakedefine HAVE_SYS_RESOURCE_H 1

#cmakedefine AS_XMLRPCCLIENT     1

#cmakedefine AS_MD5_SUPPORT         1
#cmakedefine AS_MD5_WITHOUT_OPENSSL 1

#define AS_VERSION         "${AS_VERSION}"
#define AS_IDENT           "${AS_IDENT}"
#define AS_MASTER_SITE_URL "${AS_MASTER_SITE_URL}"

#cmakedefine CAS_GLOBAL_CONFIG_FILE "${CAS_GLOBAL_CONFIG_FILE}"
#cmakedefine CAS_SHAREDIR           "${CAS_SHAREDIR}"

#endif /* _AS_SYS_HEADERS_H__ */
/* End. */
