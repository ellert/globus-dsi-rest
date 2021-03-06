/*
 * Copyright 1999-2016 University of Chicago
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "globus_i_dsi_rest.h"
#include "version.h"

const char *                            globus_i_dsi_rest_debug_level_names[] =
{
    [GLOBUS_DSI_REST_TRACE]           = "TRACE",
    [GLOBUS_DSI_REST_INFO]            = "INFO",
    [GLOBUS_DSI_REST_DEBUG]           = "DEBUG",
    [GLOBUS_DSI_REST_WARN]            = "WARN",
    [GLOBUS_DSI_REST_ERROR]           = "ERROR"
};

globus_mutex_t                          globus_i_dsi_rest_handle_cache_mutex;
size_t                                  globus_i_dsi_rest_handle_cache_index;
CURL                                   *globus_i_dsi_rest_handle_cache[GLOBUS_I_DSI_REST_HANDLE_CACHE_SIZE];

GlobusDebugDefine(GLOBUS_DSI_REST);

static
int
globus_l_dsi_rest_activate(void)
{
    int rc;
    rc = curl_global_init(CURL_GLOBAL_ALL);
    if (rc != CURLE_OK)
    {
        goto fail;
    }
    rc = globus_module_activate(GLOBUS_COMMON_MODULE);
    if (rc != GLOBUS_SUCCESS)
    {
        goto activate_fail;
    }
    rc = globus_mutex_init(&globus_i_dsi_rest_handle_cache_mutex, NULL);
    if (rc != GLOBUS_SUCCESS)
    {
        goto mutex_init_fail;
    }
    globus_i_dsi_rest_handle_cache_index = 0;

    GlobusDebugInit(GLOBUS_DSI_REST, TRACE INFO DEBUG WARN ERROR);

mutex_init_fail:
    if (rc != 0)
    {
        globus_module_deactivate(GLOBUS_COMMON_MODULE);
    }
activate_fail:
fail:
    return rc;
}
/* globus_l_dsi_rest_activate() */

static
int
globus_l_dsi_rest_deactivate(void)
{
    globus_mutex_lock(&globus_i_dsi_rest_handle_cache_mutex);
    while (globus_i_dsi_rest_handle_cache_index > 0)
    {
        curl_easy_cleanup(globus_i_dsi_rest_handle_cache[--globus_i_dsi_rest_handle_cache_index]);
    }
    globus_mutex_unlock(&globus_i_dsi_rest_handle_cache_mutex);

    globus_module_deactivate(GLOBUS_COMMON_MODULE);
    curl_global_cleanup();

    GlobusDebugDestroy(GLOBUS_DSI_REST);
    return 0;
}
/* globus_l_dsi_rest_deactivate() */

globus_module_descriptor_t
globus_i_dsi_rest_module =
{
    .module_name = "globus_dsi_rest",
    .activation_func = globus_l_dsi_rest_activate,
    .deactivation_func = globus_l_dsi_rest_deactivate,
    .version = &local_version
};
