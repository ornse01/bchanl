/*
 * extbbslist.c
 *
 * Copyright (c) 2012 project bchan
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
#include	<tstring.h>
#include	<errcode.h>
#include	<btron/btron.h>
#include	<bsys/queue.h>

#include    "extbbslist.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct extbbslist_item_t_ {
	QUEUE que;
	TC *title;
	W title_len;
	struct {
		TC *tc;
		W tc_len;
		UB *asc;
		W asc_len;
	} url;
};
typedef struct extbbslist_item_t_ extbbslist_item_t;

LOCAL extbbslist_item_t* extbbslist_item_nextnode(extbbslist_item_t *item)
{
	return (extbbslist_item_t*)item->que.next;
}

LOCAL VOID extbbslist_item_QueInsert(extbbslist_item_t *entry, extbbslist_item_t *que)
{
	QueInsert(&entry->que, &que->que);
}

LOCAL Bool extbbslist_item_titlecheck(extbbslist_item_t *entry, TC *title, W title_len)
{
	W cmp;

	if (entry->title_len != title_len) {
		return False;
	}
	cmp = tc_strncmp(entry->title, title, title_len);
	if (cmp != 0) {
		return False;
	}
	return True;
}

LOCAL VOID extbbslist_item_replacetitle(extbbslist_item_t *item, TC *title, W title_len)
{
	if (item->title != NULL) {
		free(item->title);
	}
	item->title = title;
	item->title_len = title_len;
}

LOCAL VOID extbbslist_item_replaceTCurl(extbbslist_item_t *item, TC *url, W url_len)
{
	if (item->url.tc != NULL) {
		free(item->url.tc);
	}
	item->url.tc = url;
	item->url.tc_len = url_len;
}

LOCAL VOID extbbslist_item_replaceascurl(extbbslist_item_t *item, UB *url, W url_len)
{
	if (item->url.asc != NULL) {
		free(item->url.asc);
	}
	item->url.asc = url;
	item->url.asc_len = url_len;
}

LOCAL W extbbslist_item_initialize(extbbslist_item_t *item)
{
	QueInit(&item->que);
	item->title = NULL;
	item->title_len = 0;
	item->url.tc = NULL;
	item->url.tc_len = 0;
	item->url.asc = NULL;
	item->url.asc_len = 0;

	return 0;
}

LOCAL VOID extbbslist_item_finalize(extbbslist_item_t *item)
{
	if (item->url.asc != NULL) {
		free(item->url.asc);
	}
	if (item->url.tc != NULL) {
		free(item->url.tc);
	}
	if (item->title != NULL) {
		free(item->title);
	}
	QueRemove(&item->que);
}

LOCAL extbbslist_item_t* extbbslist_item_new()
{
	extbbslist_item_t *item;
	W err;

	item = malloc(sizeof(extbbslist_item_t));
	if (item == NULL) {
		return NULL;
	}
	err = extbbslist_item_initialize(item);
	if (err < 0) {
		free(item);
		return NULL;
	}
	return item;
}

LOCAL VOID extbbslist_item_delete(extbbslist_item_t *item)
{
	extbbslist_item_finalize(item);
	free(item);
}

struct extbbslist_t_ {
	QUEUE sentinel;
	extbbslist_readcontext_t *ctx;
	W num;
	LINK *lnk;
	W rectype;
	UH subtype;
};

LOCAL extbbslist_item_t* extbbslist_sentinelnode(extbbslist_t *list)
{
	return (extbbslist_item_t*)&list->sentinel;
}

LOCAL extbbslist_item_t* extbbslist_searchitem(extbbslist_t *list, TC *title, W title_len)
{
	extbbslist_item_t *entry, *senti;
	Bool ok;

	senti = extbbslist_sentinelnode(list);
	entry = extbbslist_item_nextnode(senti);
	for (;;) {
		if (entry == senti) {
			break;
		}
		ok = extbbslist_item_titlecheck(entry, title, title_len);
		if (ok != False) {
			return entry;
		}
		entry = extbbslist_item_nextnode(entry);
	}

	return NULL;
}

EXPORT W extbbslist_appenditem(extbbslist_t *list, TC *title, W title_len, UB *url, W url_len)
{
	extbbslist_item_t *item, *senti;
	W len;
	TC *str;
	UB *str_ac;

	if (list->ctx != NULL) {
		return -1; /* TODO */
	}

	item = extbbslist_searchitem(list, title, title_len);
	if (item != NULL) {
		return -1; /* TODO */
	}

	item = extbbslist_item_new();
	if (item == NULL) {
		return -1; /* TODO */
	}

	str = malloc(sizeof(TC)*(title_len+1));
	if (str == NULL) {
		extbbslist_item_delete(item);
		return -1; /* TODO */
	}
	memcpy(str, title, sizeof(TC)*title_len);
	str[title_len] = TNULL;
	extbbslist_item_replacetitle(item, str, title_len);

	str_ac = malloc(sizeof(UB)*(url_len+1));
	if (str_ac == NULL) {
		extbbslist_item_delete(item);
		return -1; /* TODO */
	}
	memcpy(str_ac, url, sizeof(UB)*url_len);
	str_ac[url_len] = '\0';
	extbbslist_item_replaceascurl(item, str_ac, url_len);

	len = sjstotcs(NULL, url);
	str = malloc(sizeof(TC)*(len+1));
	if (str == NULL) {
		extbbslist_item_delete(item);
		return -1;
	}
	sjstotcs(str, url);
	str[len] = TNULL;
	extbbslist_item_replaceTCurl(item, str, len);

	senti = extbbslist_sentinelnode(list);
	extbbslist_item_QueInsert(item, senti);
	list->num++;

	return 0;
}

EXPORT W extbbslist_deleteitem(extbbslist_t *list, TC *title, W title_len)
{
	extbbslist_item_t *item;

	if (list->ctx != NULL) {
		return -1; /* TODO */
	}

	item = extbbslist_searchitem(list, title, title_len);
	if (item == NULL) {
		return -1; /* TODO */
	}

	extbbslist_item_delete(item);
	list->num--;

	return 0; /* TODO */
}

EXPORT W extbbslist_number(extbbslist_t *list)
{
	return list->num;
}

EXPORT W extbbslist_writefile(extbbslist_t *list)
{
}

EXPORT W extbbslist_readfile(extbbslist_t *list)
{
}

struct extbbslist_readcontext_t_ {
	extbbslist_item_t *sentinel;
	extbbslist_item_t *curr;
};

EXPORT Bool extbbslist_readcontext_getnext(extbbslist_readcontext_t *ctx, TC **title, W *title_len, UB **url, W *url_len)
{
	if (ctx->curr == ctx->sentinel) {
		return False;
	}
	*title = ctx->curr->title;
	*title_len = ctx->curr->title_len;
	*url = ctx->curr->url.asc;
	*url_len = ctx->curr->url.asc_len;
	ctx->curr = extbbslist_item_nextnode(ctx->curr);
	return True;
}

LOCAL extbbslist_readcontext_t* extbbslist_readcontext_new(extbbslist_item_t *senti, extbbslist_item_t *curr)
{
	extbbslist_readcontext_t *ctx;

	ctx = (extbbslist_readcontext_t*)malloc(sizeof(extbbslist_readcontext_t));
	if (ctx == NULL) {
		return NULL;
	}
	ctx->sentinel = senti;
	ctx->curr = curr;

	return ctx;
}

LOCAL VOID extbbslist_readcontext_delete(extbbslist_readcontext_t *ctx)
{
	free(ctx);
}

EXPORT extbbslist_readcontext_t* extbbslist_startread(extbbslist_t *list)
{
	extbbslist_readcontext_t *ctx;
	extbbslist_item_t *senti, *curr;

	senti = extbbslist_sentinelnode(list);
	curr = extbbslist_item_nextnode(senti);
	ctx = extbbslist_readcontext_new(senti, curr);
	if (ctx == NULL) {
		return NULL;
	}
	list->ctx = ctx;
	return ctx;
}

EXPORT VOID extbbslist_endread(extbbslist_t *list, extbbslist_readcontext_t *ctx)
{
	list->ctx = NULL;
	extbbslist_readcontext_delete(ctx);
}

LOCAL VOID extbbslist_initialize(extbbslist_t *list, LINK *db_link, W rectype, UH subtype)
{
	QueInit(&list->sentinel);
	list->ctx = NULL;
	list->num = 0;
	list->lnk = db_link;
	list->rectype = rectype;
	list->subtype = subtype;
}

LOCAL VOID extbbslist_finalize(extbbslist_t *list)
{
	extbbslist_item_t *item;
	Bool empty;

	for (;;) {
		empty = isQueEmpty(&list->sentinel);
		if (empty == True) {
			break;
		}
		item = (extbbslist_item_t*)list->sentinel.prev;
		extbbslist_item_delete(item);
	}
}

EXPORT extbbslist_t* extbbslist_new(LINK *db_link, W rectype, UH subtype)
{
	extbbslist_t *list;

	list = malloc(sizeof(extbbslist_t));
	if (list == NULL) {
		return NULL;
	}
	extbbslist_initialize(list, db_link, rectype, subtype);
	return list;
}

EXPORT VOID extbbslist_delete(extbbslist_t *list)
{
	extbbslist_finalize(list);
	free(list);
}
