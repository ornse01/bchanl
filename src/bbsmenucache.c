/*
 * bbsmenucache.c
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
#include	<bstdlib.h>
#include	<bstdio.h>
#include	<bstring.h>
#include	<errcode.h>

#include    "bbsmenucache.h"
#include    "cache.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%z)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct bbsmncache_t_ {
	chunkedcache_t bbsmenudata;
	UB *latestheader;
	W latestheader_len;
};

struct bbsmncache_datareadcontext_t_ {
	chunkedcache_readcontext_t contextbase;
};

EXPORT W bbsmncache_appenddata(bbsmncache_t *cache, UB *data, W len)
{
	return chunkedcache_appenddata(&cache->bbsmenudata, data, len);
}

EXPORT VOID bbsmncache_cleardata(bbsmncache_t *cache)
{
	chunkedcache_cleardata(&cache->bbsmenudata);
}

EXPORT Bool bbsmncache_datareadcontext_nextdata(bbsmncache_datareadcontext_t *context, UB **bin, W *len)
{
	return chunkedcache_readcontext_nextdata(&context->contextbase, bin, len);
}

EXPORT bbsmncache_datareadcontext_t* bbsmncache_startdataread(bbsmncache_t *cache, W start)
{
	return (bbsmncache_datareadcontext_t*)chunkedcache_startdataread(&cache->bbsmenudata, start);
}

EXPORT VOID bbsmncache_enddataread(bbsmncache_t *cache, bbsmncache_datareadcontext_t *context)
{
	chunkedcache_enddataread(&cache->bbsmenudata, (chunkedcache_readcontext_t*)context);
}

EXPORT VOID bbsmncache_getlatestheader(bbsmncache_t *cache, UB **header, W *len)
{
	*header = cache->latestheader;
	*len = cache->latestheader_len;
}

EXPORT W bbsmncache_updatelatestheader(bbsmncache_t *cache, UB *header, W len)
{
	UB *latestheader0;

	latestheader0 = realloc(cache->latestheader, len + 1);
	if (latestheader0 == NULL) {
		return -1;
	}

	cache->latestheader = latestheader0;
	memcpy(cache->latestheader, header, len);
	cache->latestheader_len = len;
	cache->latestheader[cache->latestheader_len] = '\0';

	return 0;
}

EXPORT W bbsmncache_datasize(bbsmncache_t *cache)
{
	return chunkedcache_datasize(&cache->bbsmenudata);
}

EXPORT bbsmncache_t* bbsmncache_new()
{
	bbsmncache_t *cache;

	cache = (bbsmncache_t*)malloc(sizeof(bbsmncache_t));
	if (cache == NULL) {
		return NULL;
	}
	chunkedcache_initialize(&cache->bbsmenudata);
	cache->latestheader = NULL;
	cache->latestheader_len = NULL;

	return cache;
}

EXPORT VOID bbsmncache_delete(bbsmncache_t *cache)
{
	if (cache->latestheader != NULL) {
		free(cache->latestheader);
	}
	chunkedcache_finalize(&cache->bbsmenudata);
}
