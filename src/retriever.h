/*
 * retriever.h
 *
 * Copyright (c) 2009 project bchan
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

#ifndef __RETREIEVER_H__
#define __RETREIEVER_H__

typedef struct retriever_t_ retriever_t;

IMPORT retriever_t* retriever_new();
IMPORT VOID retriever_delete(retriever_t *retriever);
IMPORT W retriever_gethost(retriever_t *retriever, UB *host);
IMPORT W retriever_request(retriever_t *retriever);
IMPORT B* retriever_getheader(retriever_t *retriever);
IMPORT W retriever_getheaderlength(retriever_t *retriever);
IMPORT B* retriever_getbody(retriever_t *retriever);
IMPORT W retriever_getbodylength(retriever_t *retriever);

IMPORT W retriver_connectsocket(retriever_t *retriever);
IMPORT VOID retriever_clearbuffer(retriever_t *retriever);
IMPORT W retriever_recieve(retriever_t *retriever, W sock);
IMPORT W retriever_parsehttpresponse(retriever_t *retriever);
IMPORT W retriever_parse_response_status(retriever_t *retriever);
IMPORT W retriever_decompress(retriever_t *retriever);

#ifdef BCHANL_CONFIG_DEBUG
IMPORT VOID retriever_debugprint(retriever_t *retriever);
IMPORT VOID retriever_dumpheader(retriever_t *retriever);
#else
#define retriever_debugprint(retriever) /**/
#define retriever_dumpheader(retriever) /**/
#endif

#endif
