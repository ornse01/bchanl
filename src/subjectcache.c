/*
 * subjectcache.c
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
#include	<btron/btron.h>
#include	<bsys/queue.h>

#include    "subjectcache.h"
#include    "cache.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct sbjtcache_t_ {
	chunkedcache_t subjectdata;
	UB *host;
	W host_len;
	UB *board;
	W board_len;
	UB *latestheader;
	W latestheader_len;
};

struct sbjtcache_datareadcontext_t_ {
	chunkedcache_readcontext_t contextbase;
};

EXPORT W sbjtcache_appenddata(sbjtcache_t *cache, UB *data, W len)
{
	return chunkedcache_appenddata(&cache->subjectdata, data, len);
}

EXPORT VOID sbjtcache_cleardata(sbjtcache_t *cache)
{
	chunkedcache_cleardata(&cache->subjectdata);
}

EXPORT Bool sbjtcache_datareadcontext_nextdata(sbjtcache_datareadcontext_t *context, UB **bin, W *len)
{
	return chunkedcache_readcontext_nextdata(&context->contextbase, bin, len);
}

EXPORT sbjtcache_datareadcontext_t* sbjtcache_startdataread(sbjtcache_t *cache, W start)
{
	return (sbjtcache_datareadcontext_t*)chunkedcache_startdataread(&cache->subjectdata, start);
}

EXPORT VOID sbjtcache_enddataread(sbjtcache_t *cache, sbjtcache_datareadcontext_t *context)
{
	chunkedcache_enddataread(&cache->subjectdata, (chunkedcache_readcontext_t*)context);
}

EXPORT VOID sbjtcache_getlatestheader(sbjtcache_t *cache, UB **header, W *len)
{
	*header = cache->latestheader;
	*len = cache->latestheader_len;
}

EXPORT W sbjtcache_updatelatestheader(sbjtcache_t *cache, UB *header, W len)
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

EXPORT VOID sbjtcache_gethost(sbjtcache_t *cache, UB **host, W *len)
{
	*host = cache->host;
	*len = cache->host_len;
}

EXPORT W sbjtcache_updatehost(sbjtcache_t *cache, UB *host, W len)
{
	UB *host0;

	host0 = realloc(cache->host, len + 1);
	if (host0 == NULL) {
		return -1;
	}

	cache->host = host0;
	memcpy(cache->host, host, len);
	cache->host_len = len;
	cache->host[cache->host_len] = '\0';

	return 0;
}

EXPORT VOID sbjtcache_getboard(sbjtcache_t *cache, UB **borad, W *len)
{
	*borad = cache->board;
	*len = cache->board_len;
}

EXPORT W sbjtcache_updateboard(sbjtcache_t *cache, UB *board, W len)
{
	UB *board0;

	board0 = realloc(cache->board, len + 1);
	if (board0 == NULL) {
		return -1;
	}

	cache->board = board0;
	memcpy(cache->board, board, len);
	cache->board_len = len;
	cache->board[cache->board_len] = '\0';

	return 0;
}

EXPORT W sbjtcache_datasize(sbjtcache_t *cache)
{
	return chunkedcache_datasize(&cache->subjectdata);
}

EXPORT sbjtcache_t* sbjtcache_new()
{
	sbjtcache_t *cache;

	cache = (sbjtcache_t*)malloc(sizeof(sbjtcache_t));
	if (cache == NULL) {
		return NULL;
	}
	chunkedcache_initialize(&cache->subjectdata);
	cache->latestheader = NULL;
	cache->latestheader_len = NULL;
	cache->host = NULL;
	cache->host_len = NULL;
	cache->board = NULL;
	cache->board_len = NULL;

	return cache;
}

EXPORT VOID sbjtcache_delete(sbjtcache_t *cache)
{
	if (cache->board != NULL) {
		free(cache->board);
	}
	if (cache->host != NULL) {
		free(cache->host);
	}
	if (cache->latestheader != NULL) {
		free(cache->latestheader);
	}
	chunkedcache_finalize(&cache->subjectdata);
}
