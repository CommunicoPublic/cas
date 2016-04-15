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
 *      ASLogCodes.hpp
 *
 * $CAS$
 */
#ifndef _AS_LOG_CODES_HPP__
#define _AS_LOG_CODES_HPP__ 1

/**
  @file ASLogCodes.hpp
  @brief CAS priority codes for logger subsystem
*/

#define AS_LOG_EMERG       0       /* system is unusable */
#define AS_LOG_ALERT       1       /* action must be taken immediately */
#define AS_LOG_CRIT        2       /* critical conditions */
#define AS_LOG_ERR         3       /* error conditions */
#define AS_LOG_WARNING     4       /* warning conditions */
#define AS_LOG_NOTICE      5       /* normal but significant condition */
#define AS_LOG_INFO        6       /* informational */
#define AS_LOG_DEBUG       7       /* debug-level messages */
#define AS_LOG_PRIMASK     0x07    /* mask to extract priority part (internal) */

#define AS_LOG_WARN        AS_LOG_WARNING /* alias for AS_LOG_WARNING */
#define AS_LOG_ERROR       AS_LOG_ERR     /* alias for AS_LOG_ERR     */

#endif // _AS_LOG_CODES_HPP__
// End.
