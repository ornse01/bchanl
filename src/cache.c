/*
 * cache.c
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

#include    "cache.h"
#include	<bstdlib.h>
#include	<bsys/queue.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

LOCAL chunkedcache_data_t* chunkedcache_data_next(chunkedcache_data_t *data)
{
	return (chunkedcache_data_t*)data->queue.next;
}

LOCAL chunkedcache_data_t* chunkedcache_data_new(UB *data, W len)
{
	chunkedcache_data_t *cache_data;

	cache_data = malloc(sizeof(chunkedcache_data_t));
	if (cache_data == NULL) {
		return NULL;
	}
	cache_data->data = malloc(sizeof(UB)*len);
	if (cache_data->data == NULL) {
		free(cache_data);
		return NULL;
	}
	memcpy(cache_data->data, data, len);
	cache_data->len = len;

	return cache_data;
}

LOCAL VOID chunkedcache_data_delete(chunkedcache_data_t *cache_data)
{
	QueRemove(&(cache_data->queue));
	if (cache_data->data != NULL) {
		free(cache_data->data);
	}
	free(cache_data);
}

EXPORT W chunkedcache_appenddata(chunkedcache_t *cache, UB *data, W len)
{
	chunkedcache_data_t *cache_data;

	if (cache->context != NULL) { /* TODO wai_sem */
		return -1;
	}

	cache_data = chunkedcache_data_new(data, len);
	if (cache_data == NULL) {
		return -1; /* TODO */
	}
	QueInsert(&(cache_data->queue), &(cache->datalist.queue));
	cache->s_datsize += len;
	return 0; /* TODO */
}

EXPORT VOID chunkedcache_cleardata(chunkedcache_t *cache)
{
	chunkedcache_data_t *cache_data;
	Bool ok;

	if (cache->context != NULL) { /* TODO wai_sem */
		return;
	}

	for (;;) {
		ok = isQueEmpty(&(cache->datalist.queue));
		if (ok == True) {
			break;
		}
		cache_data = (chunkedcache_data_t*)cache->datalist.queue.next;
		chunkedcache_data_delete(cache_data);
	}
	free(cache->datalist.data);

	cache->datalist.data = NULL;
	cache->datalist.len = 0;
	cache->s_datsize = 0;
}

EXPORT Bool chunkedcache_readcontext_nextdata(chunkedcache_readcontext_t *context, UB **bin, W *len)
{
	chunkedcache_data_t *next;

	if (context->current == NULL) {
		return False;
	}

	*bin = context->current->data + context->index;
	*len = context->current->len - context->index;

	next = chunkedcache_data_next(context->current);
	if (next == &(context->cache->datalist)) {
		next = NULL;
	}
	context->current = next;
	context->index = 0;

	return True;
}

LOCAL chunkedcache_readcontext_t* chunkedcache_readcontext_new(chunkedcache_t *cache)
{
	chunkedcache_readcontext_t *context;

	context = malloc(sizeof(chunkedcache_readcontext_t*));
	if (context == NULL) {
		return NULL;
	}
	context->cache = cache;

	return context;
}

LOCAL VOID chunkedcache_readcontext_delete(chunkedcache_readcontext_t *context)
{
	free(context);
}

EXPORT chunkedcache_readcontext_t* chunkedcache_startdataread(chunkedcache_t *cache, W start)
{
	chunkedcache_readcontext_t *context;
	chunkedcache_data_t *cache_data;
	W dest;

	if (cache->context != NULL) {
		return NULL;
	}

	context = chunkedcache_readcontext_new(cache);
	if (context == NULL) {
		return NULL;
	}
	cache->context = context;

	if (start >= cache->s_datsize) {
		context->current = NULL;
		context->index = 0;
		return context;
	}

	cache_data = &(cache->datalist);
	dest = start;
	for (;;) {
		if (dest < cache_data->len) {
			break;
		}
		dest -= cache_data->len;
		cache_data = chunkedcache_data_next(cache_data);
	}

	context->current = cache_data;
	context->index = dest;

	return context;

}

EXPORT VOID chunkedcache_enddataread(chunkedcache_t *cache, chunkedcache_readcontext_t *context)
{
	cache->context = NULL;
	chunkedcache_readcontext_delete(context);
}

EXPORT W chunkedcache_datasize(chunkedcache_t *cache)
{
	return cache->s_datsize;
}

EXPORT W chunkedcache_initialize(chunkedcache_t *cache)
{
	QueInit(&(cache->datalist.queue));
	cache->datalist.data = NULL;
	cache->datalist.len = 0;
	cache->s_datsize = 0;
	cache->context = NULL;

	return 0; /* TODO */
}

EXPORT VOID chunkedcache_finalize(chunkedcache_t *cache)
{
	chunkedcache_data_t *cache_data;
	Bool ok;

	for (;;) {
		ok = isQueEmpty(&(cache->datalist.queue));
		if (ok == True) {
			break;
		}
		cache_data = chunkedcache_data_next(&cache->datalist);
		chunkedcache_data_delete(cache_data);
	}
	if (cache->datalist.data != NULL) {
		free(cache->datalist.data);
	}
}
