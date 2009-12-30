/*
 * bbsmenucache.h
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

#include    <basic.h>

#ifndef __BBSMENUCACHE_H__
#define __BBSMENUCACHE_H__

typedef struct bbsmncache_t_ bbsmncache_t;
typedef struct bbsmncache_datareadcontext_t_ bbsmncache_datareadcontext_t;

IMPORT bbsmncache_t* bbsmncache_new();
IMPORT VOID bbsmncache_delete(bbsmncache_t *cache);
IMPORT W bbsmncache_appenddata(bbsmncache_t *cache, UB *data, W len);
IMPORT VOID bbsmncache_cleardata(bbsmncache_t *cache);
IMPORT VOID bbsmncache_getlatestheader(bbsmncache_t *cache, UB **header, W *len);
IMPORT W bbsmncache_updatelatestheader(bbsmncache_t *cache, UB *header, W len);
IMPORT W bbsmncache_datasize(bbsmncache_t *cache);
IMPORT bbsmncache_datareadcontext_t* bbsmncache_startdataread(bbsmncache_t *cache, W start);
IMPORT VOID bbsmncache_enddataread(bbsmncache_t *cache, bbsmncache_datareadcontext_t *context);

IMPORT Bool bbsmncache_datareadcontext_nextdata(bbsmncache_datareadcontext_t *context, UB **bin, W *len);

#endif
