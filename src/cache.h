/*
 * cache.h
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
#include	<bsys/queue.h>

#ifndef __CACHE_H__
#define __CACHE_H__

typedef struct chunkedcache_data_t_ chunkedcache_data_t;
typedef struct chunkedcache_t_ chunkedcache_t;
typedef struct chunkedcache_readcontext_t_ chunkedcache_readcontext_t;

struct chunkedcache_data_t_ {
	QUEUE queue;
	UB *data;
	W len;
};

struct chunkedcache_t_ {
	chunkedcache_data_t datalist;
	W s_datsize;
	chunkedcache_readcontext_t *context;
};

struct chunkedcache_readcontext_t_ {
	chunkedcache_t *cache;
	chunkedcache_data_t *current;
	W index;
};

IMPORT W chunkedcache_initialize(chunkedcache_t *cache);
IMPORT VOID chunkedcache_finalize(chunkedcache_t *cache);
IMPORT W chunkedcache_appenddata(chunkedcache_t *cache, UB *data, W len);
IMPORT VOID chunkedcache_cleardata(chunkedcache_t *cache);
IMPORT W chunkedcache_datasize(chunkedcache_t *cache);
IMPORT chunkedcache_readcontext_t* chunkedcache_startdataread(chunkedcache_t *cache, W start);
IMPORT VOID chunkedcache_enddataread(chunkedcache_t *cache, chunkedcache_readcontext_t *context);

IMPORT Bool chunkedcache_readcontext_nextdata(chunkedcache_readcontext_t *context, UB **bin, W *len);

#endif
