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
 *      FastCGIConfigHandler.cpp
 *
 * $CAS$
 */

// C++ Includes
#include "FastCGIConfigHandler.hpp"

#include "ASConfigErrorHandler.hpp"
#include "FastCGIWorkerConfig.hpp"
#include "ASXMLParser.hpp"

namespace CAS // C++ Application Server
{
/*
<?xml version="1.0" ?>
<CASFCGIConfig version="3.8">
	<Listen ipv4="127.0.0.1" ipv6="[::]" port="8899" socket="/tmp/fastcgi.sock" mode="0644"/>

	<MaxClients>1000</MaxClients>
	<MinFreeWorkers>5</MinFreeWorkers>
	<MaxFreeWorkers>7</MaxFreeWorkers>

	<MaxRequests>1000</MaxRequests>

	<ListenQueue>128</ListenQueue>

	<HostName>example.com</HostName>
	<GlobalConfigFile>/etc/cas/global-config.xml</GlobalConfigFile>
	<HostConfigFile>/etc/cas/vhosts/vhost-config.xml</HostConfigFile>

	<StatusURI>/server-status</StatusURI>

	<PidFile>/var/run/cas-fcgi.pid</PidFile>
	<User>cas</User>
	<Group>cas</Group>

	<IOTimeout>10000</IOTimeout>

	<RealIPHeader>X-Real-Ip</RealIPHeader>

	<AllowFrom>
		<Net>127.0.0.1</Net>
	</AllowFrom>

	<AllowStatusFrom>
		<Net>127.0.0.1</Net>
	</AllowStatusFrom>
</CASFCGIConfig>
*/

//
// Constructor
//
ConfigHandler::ConfigHandler(WorkerConfig  & oIWorkerConfig): oWorkerConfig(oIWorkerConfig) { ;; }

//
// Get error description
//
STLW::string ConfigHandler::GetError() { return sError; }

//
// A destructor
//
ConfigHandler::~ConfigHandler() throw() { ;; }

//
// Start of XML Element
//
INT_32 ConfigHandler::StartElement(CCHAR_P          sElement,
                                   const UINT_32    iLevel,
                                   const XMLAttr  * aAttr,
                                   const UINT_32    iAttributes,
                                   const UINT_32    iLine,
                                   const UINT_32    iColumn)
{
	sCharacters.erase();
	switch (iLevel)
	{
		// Level 1:
		case 0:
			// CPPASConfig kept for backward compatibility
			if (strcasecmp("CASFCGIConfig", sElement) == 0) { return 0; }
			break;
		// Level 2:
		case 1:
			// <MaxClients>
			if      (strcasecmp("MaxClients",       sElement) == 0) { return 0; }
			// <MinFreeWorkers>
			else if (strcasecmp("MinFreeWorkers",   sElement) == 0) { return 0; }
			// <MaxFreeWorkers>
			else if (strcasecmp("MaxFreeWorkers",   sElement) == 0) { return 0; }
			// <MaxRequests>
			else if (strcasecmp("MaxRequests",      sElement) == 0) { return 0; }
			// <ListenQueue>
			else if (strcasecmp("ListenQueue",      sElement) == 0) { return 0; }
			// <IOTimeout>
			else if (strcasecmp("IOTimeout",        sElement) == 0) { return 0; }
			// <HostName>
			else if (strcasecmp("HostName",         sElement) == 0) { return 0; }
			// <HostConfigFile>
			else if (strcasecmp("HostConfigFile",   sElement) == 0) { return 0; }
			// <GlobalConfigFile>
			else if (strcasecmp("GlobalConfigFile", sElement) == 0) { return 0; }
			// <StatusURI>
			else if (strcasecmp("StatusURI",        sElement) == 0) { return 0; }
			// <RealIPHeader>
			else if (strcasecmp("RealIPHeader",     sElement) == 0) { return 0; }
			// <PidFile>
			else if (strcasecmp("PidFile",          sElement) == 0) { return 0; }
			// <User>
			else if (strcasecmp("User",             sElement) == 0) { return 0; }
			// <Group>
			else if (strcasecmp("Group",            sElement) == 0) { return 0; }
			// <AllowFrom>
			else if (strcasecmp("AllowFrom",        sElement) == 0)
			{
				oNets = NetworkRange<UINT_32>();
				return 0;
			}
			// <AllowStatusFrom>
			else if (strcasecmp("AllowStatusFrom", sElement) == 0)
			{
				oNets = NetworkRange<UINT_32>();
				return 0;
			}
			// <Listen>
			else if (strcasecmp("Listen",          sElement) == 0)
			{
				for (UINT_32 iAttr = 0; iAttr < iAttributes; ++iAttr)
				{
					if      (strcasecmp("ipv4", aAttr[iAttr].name) == 0) { oWorkerConfig.host = aAttr[iAttr].value;       }
					else if (strcasecmp("ipv6", aAttr[iAttr].name) == 0) { oWorkerConfig.host = aAttr[iAttr].value;       }
					else if (strcasecmp("port", aAttr[iAttr].name) == 0) { oWorkerConfig.port = atoi(aAttr[iAttr].value); }
					else if (strcasecmp("path", aAttr[iAttr].name) == 0) { oWorkerConfig.path = aAttr[iAttr].value;       }
					else if (strcasecmp("mode", aAttr[iAttr].name) == 0)
					{
						CHAR_P szDummy = NULL;
						oWorkerConfig.mode = strtol(aAttr[iAttr].value, &szDummy, 8);
					}
					else
					{
						sError = ASConfigErrorHandler::InvalidAttribute(sElement, aAttr[iAttr].name, iLine, iColumn);
						return -1;
					}
				}
				return 0;
			}

			break;
		// <Net>
		case 2:
			if (strcasecmp("Net", sElement) == 0) { return 0; }
			break;
	}

	sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);
return -1;
}



//
// End of XML Element
//
INT_32 ConfigHandler::EndElement(CCHAR_P        sElement,
                                 const UINT_32  iLevel,
                                 const UINT_32  iLine,
                                 const UINT_32  iColumn)
{
	switch (iLevel)
	{
		// Level 1:
		case 0:
			if (strcasecmp("CASFCGIConfig", sElement) == 0) { return 0; }
			break;
		// Level 2:
		case 1:
			// <Listen>
			if (strcasecmp("Listen", sElement) == 0) { return 0; }
			// <IOTimeout>
			else if (strcasecmp("IOTimeout", sElement) == 0)
			{
				oWorkerConfig.io_timeout = atoi(sCharacters.data());
				return 0;
			}
			// <ListenQueue>
			else if (strcasecmp("ListenQueue", sElement) == 0)
			{
				oWorkerConfig.listen = atoi(sCharacters.data());
				return 0;
			}
			// <MaxClients>
			else if (strcasecmp("MaxClients", sElement) == 0)
			{
				oWorkerConfig.max_clients = atoi(sCharacters.data());
				return 0;
			}
			// <MinFreeWorkers>
			else if (strcasecmp("MinFreeWorkers", sElement) == 0)
			{
				oWorkerConfig.min_free_workers = atoi(sCharacters.data());
				return 0;
			}
			// <MaxFreeWorkers>
			else if (strcasecmp("MaxFreeWorkers", sElement) == 0)
			{
				oWorkerConfig.max_free_workers = atoi(sCharacters.data());
				return 0;
			}
			// <MaxRequests>
			else if (strcasecmp("MaxRequests", sElement) == 0)
			{
				oWorkerConfig.max_requests = atoi(sCharacters.data());
				return 0;
			}
			// <HostName>
			else if (strcasecmp("HostName", sElement) == 0)
			{
				oWorkerConfig.hostname = sCharacters.data();
				return 0;
			}
			// <HostConfigFile>
			else if (strcasecmp("HostConfigFile", sElement) == 0)
			{
				oWorkerConfig.host_config_file = sCharacters.data();
				return 0;
			}
			// <GlobalConfigFile>
			else if (strcasecmp("GlobalConfigFile", sElement) == 0)
			{
				if (oWorkerConfig.global_config_file.empty()) { oWorkerConfig.global_config_file = sCharacters.data(); }
				return 0;
			}
			// <StatusURI>
			else if (strcasecmp("StatusURI", sElement) == 0)
			{
				oWorkerConfig.status_uri = sCharacters.data();
				return 0;
			}
			// <RealIPHeader>
			else if (strcasecmp("RealIPHeader", sElement) == 0)
			{
				oWorkerConfig.realip_header = sCharacters.data();
				return 0;
			}
			// <AllowFrom>
			else if (strcasecmp("AllowFrom", sElement) == 0)
			{
				oWorkerConfig.allowed_nets = oNets;
				return 0;
			}
			// <AllowStatusFrom>
			else if (strcasecmp("AllowStatusFrom", sElement) == 0)
			{
				oWorkerConfig.allowed_status_nets = oNets;
				return 0;
			}
			// <PidFile>
			else if (strcasecmp("PidFile", sElement) == 0)
			{
				oWorkerConfig.pid_file = sCharacters.data();
				return 0;
			}
			// <User>
			else if (strcasecmp("User", sElement) == 0)
			{
				oWorkerConfig.user = sCharacters.data();
				return 0;
			}
			// <Group>
			else if (strcasecmp("Group", sElement) == 0)
			{
				oWorkerConfig.group = sCharacters.data();
				return 0;
			}
			break;
		// Level 3:
		case 2:
			if (strcasecmp("Net", sElement) == 0) { oNets.AddNetwork(sCharacters, 1); }
			break;
	}

	sError = ASConfigErrorHandler::InvalidTag(sElement, iLine, iColumn);

return -1;
}

//
// Characters insige a element
//
INT_32 ConfigHandler::Characters(CCHAR_P        sData,
                                 const UINT_32  iLength,
                                 const UINT_32  iLevel,
                                 const UINT_32  iLine,
                                 const UINT_32  iColumn)
{
	sCharacters.append(sData, iLength);

return 0;
}

//
// XML Parse Error Handler
//
void ConfigHandler::ParseError(CCHAR_P        sReason,
                               const UINT_32  iLevel,
                               const UINT_32  iLine,
                               const UINT_32  iColumn)
{
	sError = ASConfigErrorHandler::InternalError(sReason, iLine, iColumn);
}

} // namespace CAS
// End.
