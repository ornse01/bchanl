/*
 * bbsmenuretriever.h
 *
 * Copyright (c) 2009-2012 project bchan
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 *
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 *
 * 3. This notice may not be removed or altered from any source
 *    distribution.
 *
 */

#include	<basic.h>
#include	"bbsmenucache.h"
#include	<http/http_connector.h>

#ifndef __BBSMENURETREIEVER_H__
#define __BBSMENURETREIEVER_H__

typedef struct bbsmnretriever_t_ bbsmnretriever_t;

IMPORT bbsmnretriever_t* bbsmnretriever_new(http_connector_t *connector);
IMPORT VOID bbsmnretriever_delete(bbsmnretriever_t *retriever);
IMPORT W bbsmnretriever_sendrequest(bbsmnretriever_t *retriever, bbsmncache_t *cache);
IMPORT Bool bbsmnretriever_iswaitingendpoint(bbsmnretriever_t *retriever, ID endpoint);
IMPORT W bbsmnretriever_recievehttpevent(bbsmnretriever_t *retriever, bbsmncache_t *cache, http_connector_event *hevent);

#define BBSMNRETRIEVER_REQUEST_NOT_MODIFIED 0
#define BBSMNRETRIEVER_REQUEST_ALLRELOAD    1
#define BBSMNRETRIEVER_REQUEST_WAITNEXT     2

#endif
