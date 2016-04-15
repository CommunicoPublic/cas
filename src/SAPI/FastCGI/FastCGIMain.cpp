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
 *      FastCGIMain.cpp
 *
 * $CAS$
 */
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>
#include <errno.h>

#include "ASGetOpt.hpp"
#include "ASXMLParser.hpp"
#include "ASLoggerFile.hpp"
#include "ASLogger.hpp"
#include "FastCGI.hpp"
#include "FastCGIConfigHandler.hpp"
#include "FastCGIMainProcess.hpp"
#include "FastCGIWorkerConfig.hpp"
#include "FastCGISetProcTitle.h"

#ifndef ENV_CONFIG_FILE
#define ENV_CONFIG_FILE "CASFCGI_CONFIG"
#endif // ENV_CONFIG_FILE

#ifndef DEFAULT_CONFIG_FILE
#define DEFAULT_CONFIG_FILE "/etc/cas-fcgid/cas-fcgid.xml"
#endif // DEFAULT_CONFIG_FILE

#ifndef C_PID_LENGTH
#define C_PID_LENGTH 1024
#endif // C_PID_LENGTH

using namespace CAS;

//
// Options
//
static const Options oOptions[] = {
                                    { 'f', "config",        C_HAS_ARG },  // Config file
                                    { 'g', "global-config", C_HAS_ARG  }, // Global configuration
                                    { 'd', "debug",         C_NO_ARG  },  // Print debug info at startup
                                    { 'h', "help",          C_NO_ARG  },  // Print usage info
                                    { 'v', "version",       C_NO_ARG  },  // Print version
                                    { 'V', "compile-info",  C_NO_ARG  },  // Show compile settings
                                    { 'F', "foreground",    C_NO_ARG  },  // Run process in foreground
                                    { 0 }
                                  };

//
// Get parameters
//
static INT_32 GetParams(const INT_32    iArgc,
                        CHAR_P        * aArgv,
                        STLW::string  & sGlobalConfigFile,
                        STLW::string  & sConfigFile,
                        STLW::string  & sModuleName,
                        bool          & bDebugInfo,
                        bool          & bPrintHelp,
                        bool          & bPrintVersion,
                        bool          & bPrintCompileInfo,
                        bool          & bDaemonize)
{
	bDebugInfo = bPrintHelp = bPrintVersion = bPrintCompileInfo = false;
	bDaemonize = true;
	sConfigFile.erase();

	UINT_32 iArgNum = 1;
	CCHAR_P szArgValue;
	for(;;)
	{
		INT_32 iOption = GetOpt(oOptions, iArgNum, iArgc, aArgv, szArgValue);
		if (iOption == -1) { break; }
		switch (iOption)
		{
			case 'g':
				sGlobalConfigFile = szArgValue;
				break;
			case 'f':
				sConfigFile = szArgValue;
				break;
			case 'm':
				sModuleName = szArgValue;
				break;
			case 'd':
				bDebugInfo = true;
				break;
			case 'h':
				bPrintHelp = true;
				break;
			case 'v':
				bPrintVersion = true;
				break;
			case 'V':
				bPrintCompileInfo = true;
				break;
			case 'F':
				bDaemonize = false;
				break;
			case '?':
				fprintf(stderr, "Unknown option `%s` or option with missed argument\n", aArgv[iArgNum]);
				return EX_CONFIG;
				break;
		}
	}

	if (sConfigFile.empty())
	{
		// Check environment
		CCHAR_P szConfigFile = getenv(ENV_CONFIG_FILE);
		if (szConfigFile != NULL && *szConfigFile != '\0') { sConfigFile = szConfigFile; }

		if (sConfigFile.empty()) { sConfigFile.assign(DEFAULT_CONFIG_FILE); }
	}

	if (sGlobalConfigFile.empty())
	{
		CCHAR_P szConfigFile = getenv("CAS_GLOBAL_CONFIG");
		if (szConfigFile != NULL)
		{
			fprintf(stderr, "Global config not given, using %s from ENVIRONMENT\n", szConfigFile);
			sGlobalConfigFile = szConfigFile;
		}
	}

return EX_OK;
}

//
// Print usage info
//
static void Usage(CHAR_P szProgname)
{
	fprintf(stdout, "Usage: %s [-g <global config file>] [-f <config file>] [-h] [-v] [-V] [-d] [-F]\n", szProgname);
}

//
// Print short help
//
static void Help(CHAR_P szProgname)
{
	Usage(szProgname);
	fprintf(stdout, "Options\n"
	                "  -f <config file>        : specify alternate configuration file\n"
	                "  -g <global config file> : specify alternate global configuration file\n"
	                "  -h                      : print short help\n"
	                "  -v                      : show version number\n"
	                "  -V                      : show compile settings\n"
	                "  -d                      : run in debug mode\n"
	                "  -F                      : run main process in foreground\n\n");
}

//
// Print version
//
static void Version()
{
	fprintf(stdout, "This is CAS FastCGI server. Version " AS_VERSION "(" AS_IDENT ")" "\n\n");
}

//
// Print compile info
//
static void CompileInfo()
{
	fprintf(stdout, "CAS FastCGI server compiled " __TIME__ " " __DATE__
#ifdef __GNUC__
	" with gcc " __VERSION__ "; C++ standard version is %llu \n", (unsigned long long)__cplusplus
#endif // __GNUC__
	);

	fprintf(stdout, "    Version....................: " AS_VERSION "(" AS_IDENT ")" "\n");
	fprintf(stdout, "    Default configuration file.: " DEFAULT_CONFIG_FILE "\n");
	fprintf(stdout, "\n");
}

//
// Daemonize process
//
static INT_32 Daemonize(CCHAR_P      szPidFile,
                        ASLogger   & oLogger)
{
	// Flush all streams before calling fork
	fflush(stdout);
	fflush(stderr);

	pid_t iPid = fork();
	switch (iPid)
	{
		// Error?
		case -1:
			oLogger.Emerg("Can't fork: %s, error code %d", strerror(errno), errno);
			return EX_SOFTWARE;

		case 0:
			setsid();
			iPid = fork();
			if      (iPid > 0) { exit(EX_OK); }
			else if (iPid == -1)
			{
				oLogger.Emerg("Can't fork: %s, error code %d", strerror(errno), errno);
				return EX_SOFTWARE;
			}
			break;

		default:
			exit(EX_OK);
	}

	// Only child here
	chdir("/");
	// Write PID to file
	FILE * F = fopen(szPidFile, "w");
	if (F == NULL)
	{
		oLogger.Emerg("Can't open PID file `%s`: %s, error code %d", szPidFile, strerror(errno), errno);
		return EX_SOFTWARE;
	}

	CHAR_8 szPID[C_PID_LENGTH + 1];
	snprintf(szPID, C_PID_LENGTH, "%d\n", INT_32(getpid()));

	oLogger.Info("Pid file is: `%s`", szPidFile);

	const UINT_32 iWriteBytes = strlen(szPID);
	const size_t iWroteBytes = fwrite(szPID, sizeof(char), iWriteBytes, F);
	fclose(F);

	if (iWriteBytes != iWroteBytes)
	{
		oLogger.Emerg("Can't write PID to file `%s`", szPidFile);
		return EX_SOFTWARE;
	}

	// Reopen streams
	if (freopen("/dev/zero", "r", stdin)  == NULL) { oLogger.Emerg("Can't reopen stdin to /dev/zero"); return EX_SOFTWARE; }
	if (freopen("/dev/null", "a", stdout) == NULL) { oLogger.Emerg("Can't reopen stdout to /dev/null"); return EX_SOFTWARE; }
	if (freopen("/dev/null", "a", stderr) == NULL) { oLogger.Emerg("Can't reopen stderr to /dev/null"); return EX_SOFTWARE; }

return EX_OK;
}

//
// Get system user data
//
static INT_32 GetSystemUserData(WorkerConfig  & oWorkerConfig)
{
	if (oWorkerConfig.user.empty())
	{
		return EX_OK;
	}

	struct passwd    oUserRec;
	struct passwd  * pUser = NULL;
	INT_32          iBufferLen = 1024;
	CHAR_P          sBuffer = (CHAR_P)malloc(iBufferLen);
	for (;;)
	{
		INT_32 iRC = getpwnam_r(oWorkerConfig.user.c_str(), &oUserRec, sBuffer, iBufferLen, &pUser);
		if (iRC == 0 && pUser != NULL) { break; }
		if (iRC != ERANGE)
		{
			free(sBuffer);
			return EX_OK;
		}

		iBufferLen <<= 1;
		CHAR_P sTMP = (CHAR_P)realloc(sBuffer, iBufferLen);
		if (sTMP != NULL) { sBuffer = sTMP; }
		else
		{
			free(sBuffer);
			return EX_OK;
		}
	}

	oWorkerConfig.uid = pUser -> pw_uid;
	oWorkerConfig.gid = pUser -> pw_gid;

	free(sBuffer);

	INT_32    iGroupLen = 16;
	gid_t   * aGroups   = (gid_t *)malloc(iGroupLen * sizeof(gid_t));
	INT_32    iGroupNum = 0;
	for(;;)
	{
		iGroupNum = getgrouplist(oWorkerConfig.user.c_str(), oWorkerConfig.gid, aGroups, &iGroupLen);
		if (iGroupNum != -1) { break; }

		iGroupLen <<= 1;
		gid_t * aTMP = (gid_t *)realloc(aGroups, iGroupLen * sizeof(gid_t));
		if (aTMP != NULL) { aGroups = aTMP; }
		else
		{
			free(aGroups);
			return EX_OK;
		}
	}

	for (INT_32 iPos = 0; iPos < iGroupNum; ++iPos) { oWorkerConfig.gids.push_back(aGroups[iPos]); }
	free(aGroups);

return EX_OK;
}

//
// Switch to unprivileged user
//
static INT_32 UnixSetup(const WorkerConfig  & oWorkerConfig,
                        ASLogger            & oLogger)
{
	// Set proper UID/GID
	if (getuid() == 0)
	{
		oLogger.Info("Switching to user/group %d:%d", oWorkerConfig.gid, oWorkerConfig.uid);
		if (oWorkerConfig.uid == 0)
		{
			oLogger.Emerg("CAS FastCGI server won't work from superuser account (root).");
			return EX_SOFTWARE;
		}
		else
		{
			// Set additional groups
			if (!oWorkerConfig.gids.empty())
			{
				const size_t iGroups = oWorkerConfig.gids.size();
				gid_t * aGids        = new gid_t[iGroups];
				for(UINT_32 iPos = 0; iPos < iGroups; ++iPos) { aGids[iPos] = oWorkerConfig.gids[iPos]; }

				if (setgroups(oWorkerConfig.gids.size(), aGids) == -1)
				{
					STLW::string sGroups;

					for(UINT_32 iPos = 0; iPos < iGroups; ++iPos)
					{
						CHAR_8 szGroup[64];
						snprintf(szGroup, 63, "%llu ", (long long unsigned)aGids[iPos]);
						sGroups.append(szGroup);
					}
					delete [] aGids;

					const INT_32 iErrNo = errno;
					oLogger.Emerg("Can't set additional groups %s: %s error code %d", sGroups.c_str(), strerror(iErrNo), iErrNo);
					return EX_SOFTWARE;
				}
				delete [] aGids;
			}

			if (setgid(oWorkerConfig.gid) == -1)
			{
				const INT_32 iErrNo = errno;
				oLogger.Emerg("Can't set group id to %d: %s error code %d", INT_32(oWorkerConfig.gid), strerror(iErrNo), iErrNo);
				return EX_SOFTWARE;
			}

			if (setuid(oWorkerConfig.uid)  == -1)
			{
				const INT_32 iErrNo = errno;
				oLogger.Emerg("Can't set user id to %d: %s, error code %d", INT_32(oWorkerConfig.uid), strerror(iErrNo), iErrNo);
				return EX_SOFTWARE;
			}
		}
	}
	oLogger.Info("Switching to unprivileged user completed");

return EX_OK;
}

int main(int argc, char ** argv)
{
	STLW::string  sGlobalConfigFile;
	STLW::string  sConfigFile;
	STLW::string  sModuleName;
	bool  bPrintHelp        = false;
	bool  bPrintVersion     = false;
	bool  bPrintCompileInfo = false;
	bool  bDaemonize        = true;
	bool  bDebugInfo        = false;

	initproctitle(argc, argv);

	// Read and parse command-line options
	INT_32 iRC = GetParams(argc, argv, sGlobalConfigFile, sConfigFile, sModuleName, bDebugInfo, bPrintHelp, bPrintVersion, bPrintCompileInfo, bDaemonize);
	if (iRC != EX_OK) { Usage(argv[0]); return EX_USAGE; }

	// Just print help and exit
	if (bPrintHelp) { Help(argv[0]); return EX_OK; }

	// Print version
	if (bPrintVersion) { Version(); return EX_OK; }

	// Print compiler settings
	if (bPrintCompileInfo) { CompileInfo(); return EX_OK; }

	FILE * F = fopen(sConfigFile.c_str(), "rb");
	if (F == NULL) { fprintf(stderr, "ERROR: Cannot open `%s` for reading: %s\n", sConfigFile.c_str(), strerror(errno)); return EX_SOFTWARE; }

	WorkerConfig oWorkerConfig;
	oWorkerConfig.global_config_file = sGlobalConfigFile;
	oWorkerConfig.procname           = argv[0];
	oWorkerConfig.debug              = bDebugInfo;
	oWorkerConfig.foreground         = !bDaemonize;
	try
	{
		ConfigHandler oConfigHandler(oWorkerConfig);
		ASXMLParser oParser(&oConfigHandler);
		if (oParser.ParseFile(F) == -1)
		{
			fprintf(stderr, "ERROR: In file %s: %s\n", sConfigFile.c_str(), oConfigHandler.GetError().c_str());
			return EX_CONFIG;
		}
		fclose(F);
	}
	catch(STLW::exception &e) { fprintf(stderr, "ERROR: %s\n", e.what()); return EX_SOFTWARE; }
	catch(...)                { fprintf(stderr, "ERROR: Ouch!\n"); return EX_SOFTWARE; }

	if (GetSystemUserData(oWorkerConfig) != EX_OK) { return EX_SOFTWARE; }

	if (oWorkerConfig.global_config_file.empty())
	{
		oWorkerConfig.global_config_file = CAS_GLOBAL_CONFIG_FILE;
		fprintf(stderr, "Global config not given, using %s as DEFAULT\n", CAS_GLOBAL_CONFIG_FILE);
	}

	MainProcess oMainProcess(oWorkerConfig);

	ASLoggerFile oLogger(stderr);
	if (bDaemonize)
	{
		if (Daemonize(oWorkerConfig.pid_file.c_str(), oLogger) != EX_OK) { return EX_SOFTWARE; }
	}
	if (UnixSetup(oWorkerConfig, oLogger) != EX_OK) { return EX_SOFTWARE; }

	try
	{
		if (oMainProcess.Setup() == -1)
		{
			fprintf(stderr, "Can't start FastCGI server: %s; config file %s\n", strerror(errno), sConfigFile.c_str());
			return EX_SOFTWARE;
		}
	}
	catch(STLW::exception &e) { fprintf(stderr, "ERROR: %s\n", e.what()); return EX_SOFTWARE; }
	catch(...)                { fprintf(stderr, "ERROR: Ouch!\n"); return EX_SOFTWARE; }

return oMainProcess.Run();
}
// End.

