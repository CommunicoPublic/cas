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
 *      mod_cas.c
 *
 * $CAS$
 */

/* Apache API Includes */
#include "httpd.h"
#include "http_config.h"
#include "http_protocol.h"
#include "http_log.h"
#include "util_script.h"
#include "http_main.h"
#include "http_request.h"
#include "util_md5.h"

#ifdef HAVE_SYS_RESOURCE_H
    #include <sys/types.h>
    #include <sys/time.h>
    #include <sys/resource.h>
#endif

/* Local Includes */
#include "ASApache13Worker.h"

#define DISPLAY_CAS_VERSION 1

/* Resources usage */
#ifdef HAVE_SYS_RESOURCE_H
static struct rusage sRUsagePrev;
#endif

module MODULE_VAR_EXPORT cas_module;

/*
 * Module configuration
 */
typedef struct
{
	/* Module enabled or not */
	int       enabled;
	/* Configuration file    */
	char    * config_file;
	/* Server object         */
	void    * opaque_server;

} mod_cas_config;

/* Opaque CAS data */
static void * vOpaqueServerManager = NULL;


/*
 * Apache cleanup
 */
static void mod_cas_server_request_shutdown(void * vOpaque)
{
	ASShutdownServer(((mod_cas_config *)vOpaque) -> opaque_server);
}

/*
 * Apache cleanup
 */
static void mod_cas_server_exec_shutdown(void * vOpaque)
{
	ASShutdownServer(((mod_cas_config *)vOpaque) -> opaque_server);
}

/*
 * Default module configuration
 */
static void * mod_cas_create_server_config(pool * pPool, server_rec * sServerRecord)
{
	/* Allocate memory */
	mod_cas_config * sConfig = (mod_cas_config *)ap_pcalloc(pPool, sizeof(mod_cas_config));

	/* By default module is disabled  */
	sConfig -> enabled       = 0;

	/* No config file present         */
	sConfig -> config_file   = NULL;

	/* No actine server               */
	sConfig -> opaque_server = NULL;

	ap_register_cleanup(pPool, sConfig, mod_cas_server_request_shutdown, mod_cas_server_exec_shutdown);

return (void *)sConfig;
}

/*
 * CASEnable flag
 */
static const char * mod_cas_enable(cmd_parms *cmd, void *dconf, int flag)
{
	mod_cas_config * sModuleConfig = (mod_cas_config*)ap_get_module_config(cmd -> server -> module_config, &cas_module);

	sModuleConfig -> enabled = flag;

return NULL;
}

/*
 * CASConfigFile directive
 */
static const char * mod_cas_config_file(cmd_parms * cmd, void * dconf, char * size)
{
	char            * szCommandPtr;
	mod_cas_config  * sModuleConfig = (mod_cas_config *)ap_get_module_config(cmd -> server -> module_config, &cas_module);

	szCommandPtr = size;

	sModuleConfig -> config_file = ap_getword_conf_nc(cmd -> pool, &szCommandPtr);

return NULL;
}

/*
 * Configuration table
 */
static const command_rec mod_cas_cmds[] =
{
	{"CASEnable",     mod_cas_enable,      NULL, RSRC_CONF, FLAG,  "Enable mod_cas module" },
	{"CASConfigFile", mod_cas_config_file, NULL, RSRC_CONF, TAKE1, "Set config file"       },
	{ NULL }
};

/*
 * Post-request cleanup
 */
static void mod_cas_request_shutdown(void * pOpaqueServer)
{
	/* Clear temp. files, etc */
	PostRequestCleanup(pOpaqueServer);
}

/*
 * Request handler
 */
static int mod_cas_handler(request_rec * r)
{
	struct timeval sTimeValLocBegin;
	struct timeval sTimeValLocEnd;

	/* Get module configuration */
	mod_cas_config      * sConfig = (mod_cas_config *)ap_get_module_config(r -> server -> module_config, &cas_module);

	/* Enabled? */
	if (sConfig -> enabled == 0) { return DECLINED; }

	gettimeofday(&sTimeValLocBegin, NULL);

	/* Re-init module if need */
	if (sConfig -> opaque_server == NULL)
	{
		if (sConfig -> config_file == NULL)
		{
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, r, "CAS: Fatal error: no CASConfigFile directive");
			return DECLINED;
		}

		sConfig -> opaque_server = ASInitHost(vOpaqueServerManager, r, sConfig -> config_file);
	}

	/* Error? */
	if (sConfig -> opaque_server == NULL)
	{
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, r, "CAS: Fatal error: Cannot initialize server \"%s\" (configuration file %s)", r -> server -> server_hostname, sConfig -> config_file);
		return DECLINED;
	}

	/* Register request cleanup */
	ap_register_cleanup(r -> pool, sConfig -> opaque_server, mod_cas_request_shutdown, ap_null_cleanup);

	/* Handle request */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "CAS: Handler");

	/* Check location */
	if (ASCheckLocation(sConfig -> opaque_server, r) == -1)
	{
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "CAS: request declined");
		return DECLINED;
	}

	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "CAS: handle request");

	/* Read request + Parse headers + Parse cookies + Handle request */
	ASProcessRequest(sConfig -> opaque_server, r);

	gettimeofday(&sTimeValLocEnd, NULL);

	/* Execution time */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "mod_cas_handler: Completed in %f seconds.", (1.0 * (sTimeValLocEnd.tv_sec - sTimeValLocBegin.tv_sec) + 1.0 * (sTimeValLocEnd.tv_usec - sTimeValLocBegin.tv_usec) / 1000000));

	/* Resources usage */
#ifdef HAVE_SYS_RESOURCE_H
	{
		struct rusage sRUsage;
		if (getrusage(RUSAGE_SELF, &sRUsage) == 0)
		{
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "mod_cas_handler: rusage(utime %f(%f), stime %f(%f), rss: %lu(%d), text: %lu(%d), data: %lu(%d), stack: %lu(%d), pgrec: %lu(%d), faults: %lu(%d), swaps: %lu(%d), bklin: %lu(%d), blkout: %lu(%d), msgsnd: %lu(%d), msgrciv: %lu(%d), nsig: %lu(%d), volsw: %lu(%d), involsw: %lu(%d)",

			               (1.0 * sRUsage.ru_utime.tv_sec + 1.0 * sRUsage.ru_utime.tv_usec / 1000000),
			               (1.0 * (sRUsage.ru_utime.tv_sec - sRUsagePrev.ru_utime.tv_sec) + 1.0 * (sRUsage.ru_utime.tv_usec - sRUsagePrev.ru_utime.tv_usec) / 1000000),

			               (1.0 * sRUsage.ru_stime.tv_sec + 1.0 * sRUsage.ru_stime.tv_usec / 1000000),
			               (1.0 * (sRUsage.ru_stime.tv_sec - sRUsagePrev.ru_stime.tv_sec) + 1.0 * (sRUsage.ru_stime.tv_usec - sRUsagePrev.ru_stime.tv_usec) / 1000000),

			               sRUsage.ru_maxrss,   (int)(sRUsage.ru_maxrss   - sRUsagePrev.ru_maxrss),
			               sRUsage.ru_ixrss,    (int)(sRUsage.ru_ixrss    - sRUsagePrev.ru_ixrss),
			               sRUsage.ru_idrss,    (int)(sRUsage.ru_idrss    - sRUsagePrev.ru_idrss),
			               sRUsage.ru_isrss,    (int)(sRUsage.ru_isrss    - sRUsagePrev.ru_isrss),
			               sRUsage.ru_minflt,   (int)(sRUsage.ru_minflt   - sRUsagePrev.ru_minflt),
			               sRUsage.ru_majflt,   (int)(sRUsage.ru_majflt   - sRUsagePrev.ru_majflt),
			               sRUsage.ru_nswap,    (int)(sRUsage.ru_nswap    - sRUsagePrev.ru_nswap),
			               sRUsage.ru_inblock,  (int)(sRUsage.ru_inblock  - sRUsagePrev.ru_inblock),
			               sRUsage.ru_oublock,  (int)(sRUsage.ru_oublock  - sRUsagePrev.ru_oublock),
			               sRUsage.ru_msgsnd,   (int)(sRUsage.ru_msgsnd   - sRUsagePrev.ru_msgsnd),
			               sRUsage.ru_msgrcv,   (int)(sRUsage.ru_msgrcv   - sRUsagePrev.ru_msgrcv),
			               sRUsage.ru_nsignals, (int)(sRUsage.ru_nsignals - sRUsagePrev.ru_nsignals),
			               sRUsage.ru_nvcsw,    (int)(sRUsage.ru_nvcsw    - sRUsagePrev.ru_nvcsw),
			               sRUsage.ru_nivcsw,   (int)(sRUsage.ru_nivcsw   - sRUsagePrev.ru_nivcsw));

			// Copy data
			memcpy(&sRUsagePrev, &sRUsage, sizeof(struct rusage));
		}
	}
#endif

return OK;
}

/*
 * List of handlers
 */
static const handler_rec mod_cas_handlers[] =
{
	{ "mod_cas", mod_cas_handler },
	{NULL, NULL}
};

/*
 * URL translation
 */
static int mod_cas_translate(request_rec * r)
{
	struct timeval sTimeValLocBegin;
	struct timeval sTimeValLocEnd;

	/* Get module configuration */
	mod_cas_config  * sConfig = (mod_cas_config *)ap_get_module_config(r -> server -> module_config, &cas_module);

	/* Enabled? */
	if (sConfig -> enabled == 0) { return DECLINED; }

	gettimeofday(&sTimeValLocBegin, NULL);

	/* Re-init module if need */
	if (sConfig -> opaque_server == NULL)
	{
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "CAS: initialize virtual host \"%s\", config file \"%s\"", r -> server -> server_hostname, sConfig -> config_file);

		if (sConfig -> config_file == NULL)
		{
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, r, "CAS: Fatal error: no CASConfigFile directive");
			return DECLINED;
		}

		sConfig -> opaque_server = ASInitHost(vOpaqueServerManager, r, sConfig -> config_file);
	}

	/* Error? */
	if (sConfig -> opaque_server == NULL)
	{
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, r, "CAS: Fatal error: Cannot initialize server \"%s\" (configuration file %s)", r -> server -> server_hostname, sConfig -> config_file);
		return DECLINED;
	}

	/* Check location(s) */
	if (ASCheckLocation(sConfig -> opaque_server, r) == 0)
	{
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "CAS: set handler \"mod_cas\"");
		r -> handler = "mod_cas";

		gettimeofday(&sTimeValLocEnd, NULL);

		/* Execution time */
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "mod_cas_translate: Completed in %f seconds.", (1.0 * (sTimeValLocEnd.tv_sec - sTimeValLocBegin.tv_sec) + 1.0 * (sTimeValLocEnd.tv_usec - sTimeValLocBegin.tv_usec) / 1000000));

		return OK;
	}

	gettimeofday(&sTimeValLocEnd, NULL);

	/* Execution time */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, r, "mod_cas_translate: Completed in %f seconds.", (1.0 * (sTimeValLocEnd.tv_sec - sTimeValLocBegin.tv_sec) + 1.0 * (sTimeValLocEnd.tv_usec - sTimeValLocBegin.tv_usec) / 1000000));

/* All done */
return DECLINED;
}

/*
 * Apache cleanup
 */
static void mod_cas_server_manager_shutdown(void * vOpaque)
{
	ASShutdownServerManager(vOpaque);
}

/*
 * Apache cleanup
 */
static void mod_cas_server_manager_exec_shutdown(void * vOpaque)
{
	ASShutdownServerManager(vOpaque);
}

/*
 * Global initializer
 */
static void mod_cas_init(server_rec *s, pool *p)
{
	/* Return code */
	int iOK = -1;

	/* Initialize module at startup */
	char * szConfigFile = getenv("CAS_GLOBAL_CONFIG");
	if (szConfigFile == NULL) { szConfigFile = CAS_GLOBAL_CONFIG_FILE; }

#ifdef HAVE_SYS_RESOURCE_H
	/* Clear usage data */
	memset(&sRUsagePrev, 0, sizeof(struct rusage));
#endif

	vOpaqueServerManager = ASInitServerManager(s, szConfigFile, &iOK);

	/* Log errror and terminate process if any error occured */
	if (iOK == -1)
	{
		ap_log_error(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, s, "CAS: Fatal error, terminating instance");
		exit(1);
	}

	/* Register cleanup handlers */
	ap_register_cleanup(p, vOpaqueServerManager, mod_cas_server_manager_shutdown, mod_cas_server_manager_exec_shutdown);

	/* All done */
#ifdef DISPLAY_CAS_VERSION
	/* Add version component */
	ap_add_version_component("mod_cas/" AS_VERSION "(" AS_IDENT ")");
#endif
}

/*
 * Export table
 */
module MODULE_VAR_EXPORT cas_module =
{
	STANDARD_MODULE_STUFF,
	mod_cas_init,                   /* initializer                           */
	NULL,                           /* create per-directory config structure */
	NULL,                           /* merge per-directory config structures */
	mod_cas_create_server_config,   /* create per-server config structure    */
	NULL,                           /* merge per-server config structures    */
	mod_cas_cmds,                   /* command table                         */
	mod_cas_handlers,               /* handlers                              */
	mod_cas_translate,              /* translate_handler                     */
	NULL,                           /* check_user_id                         */
	NULL,                           /* check auth                            */
	NULL,                           /* check access                          */
	NULL,                           /* type_checker                          */
	NULL,                           /* pre-run fixups                        */
	NULL,                           /* logger                                */
	NULL,                           /* header parser                         */
	NULL,                           /* child_init                            */
	NULL,                           /* child_exit                            */
	NULL                            /* post read-request                     */
#ifdef EAPI
	,NULL,                          /* EAPI: add_module                      */
	NULL,                           /* EAPI: remove_module                   */
	NULL,                           /* EAPI: rewrite_command                 */
	NULL                            /* EAPI: new_connection                  */
#endif
};
/* End. */
