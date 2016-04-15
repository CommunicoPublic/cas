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
 *      moc_cas2.c
 *
 * $CAS$
 */

/* Apache API Includes */
#include "httpd.h"
#include "http_config.h"
#include "http_request.h"
#include "http_log.h"

#ifdef HAVE_SYS_RESOURCE_H
    #include <sys/types.h>
    #include <sys/time.h>
    #include <sys/resource.h>
#endif

#include <unistd.h>
#include <time.h>

/* Local Includes */
#include "ASApache2XWorker.h"

module AP_MODULE_DECLARE_DATA cas_module;

#define DISPLAY_CAS_VERSION 1

/* Resources usage */
#ifdef HAVE_SYS_RESOURCE_H
static struct rusage sRUsagePrev;
#endif

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
 * Default module configuration
 */
static void * mod_cas_create_server_config(apr_pool_t * p, server_rec * s)
{
	/* Allocate memory */
	mod_cas_config * sConfig = (mod_cas_config *)apr_pcalloc(p, sizeof(mod_cas_config));

	/* By default module is disabled  */
	sConfig -> enabled       = 0;

	/* No config file present         */
	sConfig -> config_file   = NULL;

	/* No actine server               */
	sConfig -> opaque_server = NULL;

	/* ap_register_cleanup(pPool, sConfig, mod_cas_server_request_shutdown, mod_cas_server_exec_shutdown); */

return (void *)sConfig;
}

/*
 * CASEnable flag
 */
static const char * mod_cas_enable(cmd_parms *cmd, void *dummy, int arg)
{
	server_rec      * s    = cmd -> server;
	mod_cas_config  * conf = ap_get_module_config(s -> module_config, &cas_module);

	conf -> enabled = arg;

return NULL;
}

/*
 * CASConfigFile directive
 */
static const char * mod_cas_config_file(cmd_parms *cmd, void *dummy, const char *arg)
{
	server_rec      * s    = cmd -> server;
	mod_cas_config  * conf = ap_get_module_config(s -> module_config, &cas_module);

	conf -> config_file = (char *)arg;

return NULL;
}

/*
 * command apr_table_t
 */
static const command_rec mod_cas_cmds[] =
{
	AP_INIT_FLAG ("CASEnable",     mod_cas_enable,      NULL, RSRC_CONF, "CASEnable <flag> -- enable or disable CAS on virtual host." ),
	AP_INIT_TAKE1("CASConfigFile", mod_cas_config_file, NULL, RSRC_CONF, "CASConfigFile <string> -- set configuration file."          ),
	{NULL}
};

/*
 * Post-request cleanup
 */
static apr_status_t mod_cas_request_shutdown(void * pOpaqueServer)
{
	/* Clear temp. files, etc */
	PostRequestCleanup(pOpaqueServer);

return APR_SUCCESS;
}

/*
 * Child shutdown
 */
static apr_status_t mod_cas_child_shutdown(void * vOpaque)
{
	/* Schut down server */
	ASShutdownServerManager(vOpaque);

return APR_SUCCESS;
}

/*
 * Server shutdown
 */
static apr_status_t mod_cas_server_shutdown(void * vOpaque)
{
	/* Shut down server manager */
	ASShutdownServerManager(vOpaque);

return APR_SUCCESS;
}

/*
 * Request handler
 */
static int mod_cas_handler(request_rec * r)
{
	int iRetVal     = 0;

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
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, 0, r, "CAS: Fatal error: no CASConfigFile directive");
			return DECLINED;
		}

		sConfig -> opaque_server = ASInitHost(vOpaqueServerManager, r, sConfig -> config_file);
	}

	/* Error? */
	if (sConfig -> opaque_server == NULL)
	{
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, 0, r, "CAS: Fatal error: Cannot initialize server \"%s\" (configuration file %s)", r -> server -> server_hostname, sConfig -> config_file);
		return DECLINED;
	}

	/* Register request cleanup */
	apr_pool_cleanup_register(r -> pool, sConfig -> opaque_server, mod_cas_request_shutdown, apr_pool_cleanup_null);

	/* Handle request */
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, 0, r, "CAS: Handler");

	/* Check location */
	if (ASCheckLocation(sConfig -> opaque_server, r) == -1)
	{
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, 0, r, "CAS: request declined");
		return DECLINED;
	}

	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, 0, r, "CAS: handle request");

	/* Error? */
	if (sConfig -> opaque_server == NULL)
	{
		ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, 0, r, "CAS: Fatal error: Cannot initialize server \"%s\" (configuration file %s)", r -> server -> server_hostname, sConfig -> config_file);
		return DECLINED;
	}

	/* Read request + Parse headers + Parse cookies + Handle request */
	iRetVal = ASProcessRequest(sConfig -> opaque_server, r);

	/* Log execution time */
	gettimeofday(&sTimeValLocEnd, NULL);
	ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, 0, r, "mod_cas_handler: Completed in %f seconds.", (1.0 * (sTimeValLocEnd.tv_sec - sTimeValLocBegin.tv_sec) + 1.0 * (sTimeValLocEnd.tv_usec - sTimeValLocBegin.tv_usec) / 1000000));

	/* Resources usage */
#ifdef HAVE_SYS_RESOURCE_H
	{
		struct rusage sRUsage;
		if (getrusage(RUSAGE_SELF, &sRUsage) == 0)
		{
			ap_log_rerror(APLOG_MARK, APLOG_NOERRNO | APLOG_INFO, 0, r, "mod_cas_handler: rusage(utime %f(%f), stime %f(%f), rss: %lu(%d), text: %lu(%d), data: %lu(%d), stack: %lu(%d), pgrec: %lu(%d), faults: %lu(%d), swaps: %lu(%d), bklin: %lu(%d), blkout: %lu(%d), msgsnd: %lu(%d), msgrciv: %lu(%d), nsig: %lu(%d), volsw: %lu(%d), involsw: %lu(%d)",

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

	/* Return value */
	if (iRetVal != OK) { return iRetVal; }

return OK;
}

/*
 * Global initializer
 */
static int mod_cas_init(apr_pool_t * pconf, apr_pool_t * plog, apr_pool_t * ptemp, server_rec * s)
{
	/* Return code */
	int iOK     = -1;
	int iRetVal = 0;

 	srandom(time(NULL) + getpid());

	/* Initialize module at startup */
	char * szConfigFile = getenv("CAS_GLOBAL_CONFIG");
	if (szConfigFile == NULL) { szConfigFile = CAS_GLOBAL_CONFIG_FILE; }

	vOpaqueServerManager = ASInitServerManager(s, szConfigFile, &iOK);

#ifdef HAVE_SYS_RESOURCE_H
	/* Clear usage data */
	memset(&sRUsagePrev, 0, sizeof(struct rusage));
#endif

	/* Log errror and terminate process if any error occured */
	if (iOK == -1)
	{
		ap_log_error(APLOG_MARK, APLOG_NOERRNO | APLOG_EMERG, iRetVal, s, "CAS: Fatal error, terminating instance");
		exit(1);
	}

	/* Register cleanup handlers */
/*	ap_register_cleanup(p, vOpaqueServerManager, mod_cas_request_shutdown, mod_cas_exec_shutdown); */

	/* Register shutdown handler */
	apr_pool_cleanup_register(pconf, vOpaqueServerManager, mod_cas_server_shutdown, apr_pool_cleanup_null);

	/* All done */
#ifdef DISPLAY_CAS_VERSION
	/* Add version component */
	ap_add_version_component(pconf, "mod_cas/" AS_VERSION "(" AS_IDENT ")");
#endif

return OK;
}

/*
 * Child initialization
 */
static void mod_cas_child_init(apr_pool_t * pchild, server_rec * s)
{
	apr_pool_cleanup_register(pchild, vOpaqueServerManager, mod_cas_child_shutdown, apr_pool_cleanup_null);
}

/*
 * Register hooks
 */
static void register_hooks(apr_pool_t * p)
{
	/* Global initialization */
	ap_hook_post_config(mod_cas_init, NULL, NULL, APR_HOOK_MIDDLE);
	/* Handler               */
	ap_hook_handler(mod_cas_handler, NULL, NULL, APR_HOOK_MIDDLE);
	/* Child initialization  */
	ap_hook_child_init(mod_cas_child_init, NULL, NULL, APR_HOOK_MIDDLE);
}

/*
 * Export table
 */
module AP_MODULE_DECLARE_DATA cas_module =
{
	STANDARD20_MODULE_STUFF,
	NULL,                         /* create per-directory configuration structures */
	NULL,                         /* merge per-directory configuration structures  */
	mod_cas_create_server_config, /* create per-server configuration structures    */
	NULL,                         /* merge per-server config                       */
	mod_cas_cmds,                 /* command apr_table_t                           */
	register_hooks                /* register hooks                                */
};
/* End. */
