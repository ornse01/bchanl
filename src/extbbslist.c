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

LOCAL VOID extbbslist_item_QueRemove(extbbslist_item_t *entry)
{
	QueRemove(&entry->que);
	QueInit(&entry->que);
}

LOCAL Bool extbbslist_item_isQueEmpty(extbbslist_item_t *entry)
{
	return isQueEmpty(&entry->que);
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

LOCAL W extbbslist_item_assigntitle(extbbslist_item_t *item, CONST TC *title, W title_len)
{
	TC *str;
	str = malloc(sizeof(TC)*(title_len+1));
	if (str == NULL) {
		return -1; /* TODO */
	}
	memcpy(str, title, sizeof(TC)*title_len);
	str[title_len] = TNULL;
	extbbslist_item_replacetitle(item, str, title_len);
	return 0;
}

LOCAL W extbbslist_item_assignurl(extbbslist_item_t *item, CONST TC *url, W url_len)
{
	TC *str;
	UB *str_ac;
	W len;

	str = malloc(sizeof(TC)*(url_len+1));
	if (str == NULL) {
		return -1; /* TODO */
	}
	memcpy(str, url, sizeof(TC)*url_len);
	str[url_len] = TNULL;

	len = tcstosjs(NULL, url);
	str_ac = malloc(sizeof(UB)*(len+1));
	if (str_ac == NULL) {
		free(str);
		return -1; /* TODO */
	}
	tcstosjs(str_ac, url);
	str_ac[len] = '\0';

	extbbslist_item_replaceTCurl(item, str, url_len);
	extbbslist_item_replaceascurl(item, str_ac, len);

	return 0;
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
	extbbslist_editcontext_t *edit;
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

LOCAL VOID extbbslist_clear(extbbslist_t *list)
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

struct extbbslist_editcontext_t_ {
	QUEUE sentinel;
	W view_l, view_t, view_r, view_b;
	W num;
	struct {
		extbbslist_item_t *item;
		W index;
	} selected;
};

#define EXTBBSLIST_ENTRY_HEIGHT 20
#define EXTBBSLIST_ENTRY_PADDING_TOP 1

#define EXTBBSLIST_TITLE_WIDTH 128

LOCAL extbbslist_item_t* extbbslist_editcontext_sentinelnode(extbbslist_editcontext_t *ctx)
{
	return (extbbslist_item_t*)&ctx->sentinel;
}

LOCAL extbbslist_item_t *extbbslist_editcontext_searchitembyindex(extbbslist_editcontext_t *ctx, W index)
{
	W i;
	extbbslist_item_t *senti, *item;

	senti = extbbslist_editcontext_sentinelnode(ctx);
	item = extbbslist_item_nextnode(senti);
	for (i = 0; item != senti; i++) {
		if (i == index) {
			return item;
		}
		item = extbbslist_item_nextnode(item);
	}
	return NULL;
}

EXPORT W extbbslist_editcontext_append(extbbslist_editcontext_t *ctx, CONST TC *title, W title_len, CONST TC *url, W url_len)
{
	extbbslist_item_t *item, *senti;
	W err;

	item = extbbslist_item_new();
	if (item == NULL) {
		return -1; /* TODO */
	}

	err = extbbslist_item_assigntitle(item, title, title_len);
	if (err < 0) {
		extbbslist_item_delete(item);
		return err;
	}

	err = extbbslist_item_assignurl(item, url, url_len);
	if (err < 0) {
		extbbslist_item_delete(item);
		return err;
	}

	senti = extbbslist_editcontext_sentinelnode(ctx);
	extbbslist_item_QueInsert(item, senti);
	ctx->num++;

	return 0;
}

EXPORT W extbbslist_editcontext_update(extbbslist_editcontext_t *ctx, W i, CONST TC *title, W title_len, CONST TC *url, W url_len)
{
	extbbslist_item_t *item;
	W err;

	item = extbbslist_editcontext_searchitembyindex(ctx, i);
	if (item == NULL) {
		return -1; /* TODO */
	}

	err = extbbslist_item_assigntitle(item, title, title_len);
	if (err < 0) {
		return err;
	}

	err = extbbslist_item_assignurl(item, url, url_len);
	if (err < 0) {
		return err;
	}

	return 0;
}

LOCAL W extbbslist_editcontext_drawcolseparater(extbbslist_editcontext_t *ctx, GID target, W x_start, W y, W height, PAT *lnpat)
{
	PNT p1, p2;

	p1.y = y;
	p2.y = p1.y + height - 1;

	p1.x = x_start + EXTBBSLIST_TITLE_WIDTH + 4;
	p2.x = p1.x;
	gdra_lin(target, p1, p2, 1, lnpat, G_STORE);

	return 0;
}

LOCAL W extbbslist_editcontext_drawitemtext(extbbslist_editcontext_t *ctx, GID target, W x_start, W y, PAT *lnpat, extbbslist_item_t *item)
{
	SIZE sz;
	FSSPEC fspec;
	W y2;

	gset_scr(target, 0x21);

	gget_fon(target, &fspec, NULL);
	sz = fspec.size;
	fspec.size.h = fspec.size.v;
	gset_fon(target, &fspec);

	y2 = y + sz.v + EXTBBSLIST_ENTRY_PADDING_TOP;
	gdra_stp(target, x_start, y2, item->title, item->title_len, G_STORE);

	gset_scr(target, 0x21);

	fspec.size.h = fspec.size.v / 2;
	gset_fon(target, &fspec);

	gdra_stp(target, x_start + EXTBBSLIST_TITLE_WIDTH + 8 , y2, item->url.tc, item->url.tc_len, G_STORE);

	extbbslist_editcontext_drawcolseparater(ctx, target, x_start, y, EXTBBSLIST_ENTRY_HEIGHT, lnpat);

	return 0;
}

EXPORT W extbbslist_editcontext_draw(extbbslist_editcontext_t *ctx, GID target, RECT *r)
{
	W i, x, y;
	extbbslist_item_t *senti, *item;
	PAT bgpat = {{
		0,
		16, 16,
		0x10efefef,
		0x10efefef,
		FILL100
	}};
	PAT lnpat = {{
		0,
		16, 16,
		0x10000000,
		0x10efefef,
		FILL100
	}};
	RECT bgr;

	senti = extbbslist_editcontext_sentinelnode(ctx);
	item = extbbslist_item_nextnode(senti);
	for (i = 0; ; i++) {
		if (item == senti) {
			break;
		}
		if ((ctx->view_t <= (i + 1) * EXTBBSLIST_ENTRY_HEIGHT)&&(i * EXTBBSLIST_ENTRY_HEIGHT <= ctx->view_b)) {
			if (ctx->selected.item == item) {
				bgpat.spat.fgcol = 0x10000000;
				lnpat.spat.fgcol = 0x10FFFFFF;
				gset_chc(target, 0x10FFFFFF, 0x10000000);
			} else {
				bgpat.spat.fgcol = 0x10FFFFFF;
				lnpat.spat.fgcol = 0x10000000;
				gset_chc(target, 0x10000000, 0x10FFFFFF);
			}
			bgr.c.left = ctx->view_l;
			bgr.c.top = i * EXTBBSLIST_ENTRY_HEIGHT - ctx->view_t;
			bgr.c.right = ctx->view_r;
			bgr.c.bottom = (i+1) * EXTBBSLIST_ENTRY_HEIGHT - ctx->view_t;
			gfil_rec(target, bgr, &bgpat, 0, G_STORE);

			x = 0 - ctx->view_l;
			y = i * EXTBBSLIST_ENTRY_HEIGHT - ctx->view_t;
			extbbslist_editcontext_drawitemtext(ctx, target, x, y, &lnpat, item);
		}

		item = extbbslist_item_nextnode(item);
	}

	return 0;
}

EXPORT Bool extbbslist_editcontext_finditem(extbbslist_editcontext_t *ctx, PNT rel_pos, W *index)
{
	W n;
	n = (rel_pos.y + ctx->view_t) / EXTBBSLIST_ENTRY_HEIGHT;
	if (n < ctx->num) {
		*index = n;
		return True;
	}
	return False;
}

IMPORT W extbbslist_editcontext_swapitem(extbbslist_editcontext_t *ctx, W i0, W i1)
{
	extbbslist_item_t *item0, *item1, *item0_next, *item1_next;
	W buf;

	if (i0 == i1) {
		return 0;
	}
	if (i1 > i0) {
		buf = i1;
		i1 = i0;
		i0 = buf;
	}

	item0 = extbbslist_editcontext_searchitembyindex(ctx, i0);
	if (item0 == NULL) {
		return -1;
	}
	item0_next = extbbslist_item_nextnode(item0);
	item1 = extbbslist_editcontext_searchitembyindex(ctx, i1);
	if (item1 == NULL) {
		return -1;
	}
	item1_next = extbbslist_item_nextnode(item1);

	if (i0 + 1 == i1) {
		extbbslist_item_QueRemove(item1);
		extbbslist_item_QueInsert(item1, item0);
		return 0;
	}

	extbbslist_item_QueRemove(item1);
	extbbslist_item_QueInsert(item1, item0_next);
	extbbslist_item_QueRemove(item0);
	extbbslist_item_QueInsert(item0, item1_next);

	return 0;
}

IMPORT W extbbslist_editcontext_deleteitem(extbbslist_editcontext_t *ctx, W i)
{
	extbbslist_item_t *item;

	item = extbbslist_editcontext_searchitembyindex(ctx, i);
	if (item == NULL) {
		return -1; /* TODO */
	}
	ctx->num--;
	if (ctx->selected.index == i) {
		ctx->selected.item = NULL;
		ctx->selected.index = -1;
	}
	ctx->selected.item = item;
	ctx->selected.index = i;
	return 0;
}

IMPORT VOID extbbslist_editcontext_setselect(extbbslist_editcontext_t *ctx, W i)
{
	extbbslist_item_t *item;

	item = extbbslist_editcontext_searchitembyindex(ctx, i);
	if (item == NULL) {
		ctx->selected.item = NULL;
		ctx->selected.index = -1;
		return;
	}
	ctx->selected.item = item;
	ctx->selected.index = i;
}

IMPORT W extbbslist_editcontext_getselect(extbbslist_editcontext_t *ctx)
{
	return ctx->selected.index;
}

EXPORT VOID extbbslist_editcontext_setviewrect(extbbslist_editcontext_t *ctx, W l, W t, W r, W b)
{
	ctx->view_l = l;
	ctx->view_t = t;
	ctx->view_r = r;
	ctx->view_b = b;
}

EXPORT VOID extbbslist_editcontext_getviewrect(extbbslist_editcontext_t *ctx, W *l, W *t, W *r, W *b)
{
	*l = ctx->view_l;
	*t = ctx->view_t;
	*r = ctx->view_r;
	*b = ctx->view_b;
}

EXPORT VOID extbbslist_editcontext_scrollviewrect(extbbslist_editcontext_t *ctx, W dh, W dv)
{
	ctx->view_l += dh;
	ctx->view_t += dv;
	ctx->view_r += dh;
	ctx->view_b += dv;
}

EXPORT VOID extbbslist_editcontext_getdrawrect(extbbslist_editcontext_t *ctx, W *l, W *t, W *r, W *b)
{
	*l = 0;
	*t = 0;
	*r = 200;
	*b = EXTBBSLIST_ENTRY_HEIGHT * ctx->num;
}

LOCAL extbbslist_editcontext_t* extbbslist_editcontext_new()
{
	extbbslist_editcontext_t *ctx;

	ctx = (extbbslist_editcontext_t*)malloc(sizeof(extbbslist_editcontext_t));
	if (ctx == NULL) {
		return NULL;
	}
	QueInit(&ctx->sentinel);
	ctx->view_l = 0;
	ctx->view_t = 0;
	ctx->view_r = 0;
	ctx->view_b = 0;
	ctx->num = 0;
	ctx->selected.item = NULL;
	ctx->selected.index = -1;

	return ctx;
}

LOCAL VOID extbbslist_editcontext_delete(extbbslist_editcontext_t *ctx)
{
	extbbslist_item_t *item;
	Bool empty;

	for (;;) {
		empty = isQueEmpty(&ctx->sentinel);
		if (empty == True) {
			break;
		}
		item = (extbbslist_item_t*)ctx->sentinel.prev;
		extbbslist_item_delete(item);
	}
	free(ctx);
}

EXPORT extbbslist_editcontext_t* extbbslist_startedit(extbbslist_t *list)
{
	extbbslist_editcontext_t *ctx;
	extbbslist_item_t *senti, *item;
	W err;

	if (list->ctx != NULL) {
		return NULL;
	}
	if (list->edit != NULL) {
		return NULL;
	}

	ctx = extbbslist_editcontext_new();
	if (ctx == NULL) {
		return NULL;
	}

	senti = extbbslist_sentinelnode(list);
	item = extbbslist_item_nextnode(senti);
	for (; item != senti;) {
		err = extbbslist_editcontext_append(ctx, item->title, item->title_len, item->url.tc, item->url.tc_len);
		if (err < 0) {
			extbbslist_editcontext_delete(ctx);
			return NULL;
		}
		item = extbbslist_item_nextnode(item);
	}

	list->edit = ctx;

	return ctx;
}

EXPORT VOID extbbslist_endedit(extbbslist_t *list, extbbslist_editcontext_t *ctx, Bool update)
{
	extbbslist_item_t *sentinel, *next;
	Bool empty;

	if (update != False) {
		extbbslist_clear(list);

		sentinel = extbbslist_editcontext_sentinelnode(ctx);

		empty = extbbslist_item_isQueEmpty(sentinel);
		if (empty == False) {
			next = extbbslist_item_nextnode(sentinel);
			extbbslist_item_QueRemove(sentinel);

			sentinel = extbbslist_sentinelnode(list);
			extbbslist_item_QueInsert(sentinel, next);
		}
	}

	list->edit = NULL;
	extbbslist_editcontext_delete(ctx);
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
	list->edit = NULL;
	list->num = 0;
	list->lnk = db_link;
	list->rectype = rectype;
	list->subtype = subtype;
}

LOCAL VOID extbbslist_finalize(extbbslist_t *list)
{
	extbbslist_clear(list);
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
