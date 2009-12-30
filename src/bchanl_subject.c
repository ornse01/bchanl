/*
 * bchanl_subject.c
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
#include	<tcode.h>
#include	<tstring.h>
#include	<bsys/queue.h>
#include	<btron/btron.h>
#include	<btron/hmi.h>

#include    "bchanl_subject.h"

#include	"subjectcache.h"
#include	"subjectparser.h"
#include	"subjectlayout.h"

/* copy from bchan/src/cache.h */
#define DATCACHE_RECORDTYPE_MAIN 31
#define DATCACHE_RECORDTYPE_INFO 30
#define DATCACHE_RECORDSUBTYPE_RETRIEVE 0x0001
#define DATCACHE_RECORDSUBTYPE_HEADER 0x0002

struct bchanl_subject_t_ {
	GID gid;
	sbjtcache_t *cache;
	sbjtparser_t *parser;
	sbjtlayout_t *layout;
	sbjtdraw_t *draw;
	TC *title;
	W title_len;
};

typedef struct bchanl_subjecthashnode_t_ {
	QUEUE queue;
	bchanl_subject_t subject;
} bchanl_subjecthashnode_t;

struct bchanl_subjecthash_t_ {
	W base;
	GID gid;
	FSSPEC fspec;
	COLOR vobjbgcol;
	bchanl_subjecthashnode_t tbl[1];
};

EXPORT VOID bchanl_subject_gettitle(bchanl_subject_t *subject, TC **title, W *title_len)
{
	*title = subject->title;
	*title_len = subject->title_len;
}

EXPORT sbjtcache_t* bchanl_subject_getcache(bchanl_subject_t *subject)
{
	return subject->cache;
}

EXPORT sbjtlayout_t* bchanl_subject_getlayout(bchanl_subject_t *subject)
{
	return subject->layout;
}

EXPORT sbjtdraw_t* bchanl_subject_getdraw(bchanl_subject_t *subject)
{
	return subject->draw;
}

EXPORT W bchanl_subject_relayout(bchanl_subject_t *subject)
{
	sbjtparser_t *parser;
	sbjtlayout_t *layout;
	sbjtparser_thread_t *thread = NULL;
	W err;

	parser = subject->parser;
	layout = subject->layout;

	sbjtlayout_clear(layout);
	sbjtparser_clear(parser);

	for (;;) {
		err = sbjtparser_getnextthread(parser, &thread);
		if (err != 1) {
			break;
		}
		if (thread != NULL) {
			sbjtlayout_appendthread(layout, thread);
		} else {
			break;
		}
	}

	return 0;
}

EXPORT W bchanl_subject_createviewervobj(bchanl_subject_t *subject, sbjtparser_thread_t *thread, UB *fsnrec, W fsnrec_len, VOBJSEG *seg, LINK *lnk)
{
	W fd, len, err;
	UB *bin;
	TC *str, title[21];

	seg->view = (RECT){{0,0,300,20}};
	seg->height = 100;
	seg->chsz = 16;
	seg->frcol = 0x10000000;
	seg->chcol = 0x10000000;
	seg->tbcol = sbjtlayout_getvobjbgcol(subject->layout);
	seg->bgcol = 0x10ffffff;
	seg->dlen = 0;

	/* should move to parser? */
	str = tc_strrchr(thread->title, TK_LPAR);
	if (str == NULL) {
		len = thread->title_len;
	} else {
		len = (str - thread->title) - 1;
	}
	if (len > 20) {
		len = 20;
	}
	tc_strncpy(title, thread->title, len);
	title[len] = TNULL;

	fd = cre_fil(lnk, title, NULL, 1, F_FLOAT);
	if (fd < 0) {
		printf("cre_fil error\n");
		return fd;
	}

	err = apd_rec(fd, fsnrec, fsnrec_len, 8, 0, 0);
	if (err < 0) {
		printf("apd_rec:fusen rec error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}

	err = apd_rec(fd, NULL, NULL, DATCACHE_RECORDTYPE_INFO, DATCACHE_RECORDSUBTYPE_RETRIEVE, 0);
	if (err < 0) {
		printf("apd_rec:retrieve info error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}
	err = see_rec(fd, -1, -1, NULL);
	if (err < 0) {
		printf("see_rec error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}

	sbjtcache_gethost(subject->cache, &bin, &len);
	err = wri_rec(fd, -1, bin, len, NULL, NULL, 0);
	if (err < 0) {
		printf("wri_rec:host error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}
	err = wri_rec(fd, -1, "\n", 1, NULL, NULL, 0);
	if (err < 0) {
		printf("wri_rec:host error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}
	sbjtcache_getboard(subject->cache, &bin, &len);
	err = wri_rec(fd, -1, bin, len, NULL, NULL, 0);
	if (err < 0) {
		printf("wri_rec:board error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}
	err = wri_rec(fd, -1, "\n", 1, NULL, NULL, 0);
	if (err < 0) {
		printf("wri_rec:board error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}
	bin = thread->number;
	len = thread->number_len;
	err = wri_rec(fd, -1, bin, len, NULL, NULL, 0);
	if (err < 0) {
		printf("wri_rec:thread error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}
	err = wri_rec(fd, -1, "\n", 1, NULL, NULL, 0);
	if (err < 0) {
		printf("wri_rec:thread error\n");
		cls_fil(fd);
		del_fil(NULL, lnk, 0);
		return fd;
	}

	cls_fil(fd);

	return 0;
}

LOCAL W bchanl_subject_initialize(bchanl_subject_t *subject, GID gid, UB *host, W host_len, UB *board, W board_len, TC *title, W title_len, FSSPEC *fspec, COLOR vobjbgcol)
{
	sbjtcache_t *cache;
	sbjtparser_t *parser;
	sbjtlayout_t *layout;
	sbjtdraw_t *draw;
	TC *title_buf;
	W err;

	cache = sbjtcache_new();
	if (cache == NULL) {
		goto error_cache;
	}
	parser = sbjtparser_new(cache);
	if (parser == NULL) {
		goto error_parser;
	}
	layout = sbjtlayout_new(gid);
	if (layout == NULL) {
		goto error_layout;
	}
	draw = sbjtdraw_new(layout);
	if (draw == NULL) {
		goto error_draw;
	}
	title_buf = malloc(sizeof(TC)*(title_len+1));
	if (title_buf == NULL) {
		goto error_title;
	}
	tc_strncpy(title_buf, title, title_len);
	title_buf[title_len] = TNULL;
	err = sbjtcache_updatehost(cache, host, host_len);
	if (err < 0) {
		goto error_info;
	}
	err = sbjtcache_updateboard(cache, board, board_len);
	if (err < 0) {
		goto error_info;
	}
	sbjtlayout_setfsspec(layout, fspec);
	sbjtlayout_setvobjbgcol(layout, vobjbgcol);

	subject->gid = gid;
	subject->cache = cache;
	subject->parser = parser;
	subject->layout = layout;
	subject->draw = draw;
	subject->title = title_buf;

	return 0;

error_info:
	free(title_buf);
error_title:
	sbjtdraw_delete(draw);
error_draw:
	sbjtlayout_delete(layout);
error_layout:
	sbjtparser_delete(parser);
error_parser:
	sbjtcache_delete(cache);
error_cache:
	return -1; /* TODO */
}

LOCAL VOID bchanl_subject_finalize(bchanl_subject_t *subject)
{
	free(subject->title);
	sbjtdraw_delete(subject->draw);
	sbjtlayout_delete(subject->layout);
	sbjtparser_delete(subject->parser);
	sbjtcache_delete(subject->cache);
}

LOCAL bchanl_subjecthashnode_t* bchanl_subjecthashnode_new(GID gid, UB *host, W host_len, UB *board, W board_len, TC *title, W title_len, FSSPEC *fspec, COLOR vobjbgcol)
{
	bchanl_subjecthashnode_t *node;
	W err;

	node = (bchanl_subjecthashnode_t*)malloc(sizeof(bchanl_subjecthashnode_t));
	if (node == NULL) {
		return NULL;
	}

	QueInit(&(node->queue));
	err = bchanl_subject_initialize(&(node->subject), gid, host, host_len, board, board_len, title, title_len, fspec, vobjbgcol);
	if (err < 0) {
		free(node);
		return NULL;
	}

	return node;
}

LOCAL VOID bchanl_subjecthashnode_delete(bchanl_subjecthashnode_t *hashnode)
{
	bchanl_subject_finalize(&(hashnode->subject));
	QueRemove(&(hashnode->queue));
	return;
}

LOCAL Bool bchanl_subjecthashnode_issameboard(bchanl_subjecthashnode_t *node, UB *host, W host_len, UB *board, W board_len)
{
	sbjtcache_t *cache;
	UB *host0, *board0;
	W host_len0, board_len0, cmp;

	cache = node->subject.cache;

	sbjtcache_gethost(cache, &host0, &host_len0);
	if (host_len != host_len0) {
		return False;
	}
	cmp = strncmp(host, host0, host_len);
	if (cmp != 0) {
		return False;
	}

	sbjtcache_getboard(cache, &board0, &board_len0);
	if (board_len != board_len0) {
		return False;
	}

	cmp = strncmp(board, board0, board_len);
	if (cmp != 0) {
		return False;
	}

	return True;
}

LOCAL W bchanl_subjecthash_calchashvalue(bchanl_subjecthash_t *subjecthash, UB *host, W host_len, UB *board, W board_len)
{
	W i,num = 0;

	for (i = 0; i < host_len; i++) {
		num += host[i];
	}
	for (i = 0; i < board_len; i++) {
		num += board[i];
	}

	return num % subjecthash->base;
}

EXPORT bchanl_subject_t* bchanl_subjecthash_search(bchanl_subjecthash_t *subjecthash, UB *host, W host_len, UB *board, W board_len)
{
	bchanl_subjecthashnode_t *node, *buf;
	W hashval;
	Bool same;

	hashval = bchanl_subjecthash_calchashvalue(subjecthash, host, host_len, board, board_len);
	buf = subjecthash->tbl + hashval;

	for (node = (bchanl_subjecthashnode_t *)buf->queue.next; node != buf; node = (bchanl_subjecthashnode_t *)node->queue.next) {
		same = bchanl_subjecthashnode_issameboard(node, host, host_len, board, board_len);
		if (same == True) {
			return &(node->subject);
		}
	}

	return NULL;
}

EXPORT W bchanl_subjecthash_append(bchanl_subjecthash_t *subjecthash, UB *host, W host_len, UB *board, W board_len, TC *title, W title_len)
{
	bchanl_subjecthashnode_t *hashnode, *buf;
	bchanl_subject_t *subject;
	W hashval;

	subject = bchanl_subjecthash_search(subjecthash, host, host_len, board, board_len);
	if (subject != NULL) {
		return 0;
	}

	hashval = bchanl_subjecthash_calchashvalue(subjecthash, host, host_len, board, board_len);
	buf = subjecthash->tbl + hashval;

	hashnode = bchanl_subjecthashnode_new(subjecthash->gid, host, host_len, board, board_len, title, title_len, &subjecthash->fspec, subjecthash->vobjbgcol);
	if (hashnode == NULL) {
		return -1; /* TODO*/
	}

	QueInsert(&(hashnode->queue), &(buf->queue));

	return 0;
}

EXPORT bchanl_subjecthash_t* bchanl_subjecthash_new(GID gid, W base)
{
	W i, err;
	bchanl_subjecthash_t *ret;

	if (base <= 0) {
		return NULL;
	}

	ret = (bchanl_subjecthash_t*)malloc(sizeof(W)+sizeof(GID)+sizeof(FSSPEC)+sizeof(COLOR)+sizeof(bchanl_subjecthashnode_t)*base);
	if (ret == NULL) {
		return NULL;
	}

	ret->gid = gid;
	ret->base = base;
	for (i=0;i<base;i++) {
		QueInit(&(ret->tbl[i].queue));
	}

	err = wget_inf(WI_FSVOBJ, &ret->fspec, sizeof(FSSPEC));
	if (err < 0) {
		ret->fspec.name[0] = TNULL;
		ret->fspec.attr = FT_PROP|FT_GRAYSCALE;
		ret->fspec.fclass = FTC_DEFAULT;
		ret->fspec.size.h = 16;
		ret->fspec.size.v = 16;
	}
	err = wget_inf(WI_VOBJBGCOL, &ret->vobjbgcol, sizeof(COLOR));
	if (err < 0) {
		ret->vobjbgcol = 0x10000000;
	}

	return ret;
}

EXPORT VOID bchanl_subjecthash_delete(bchanl_subjecthash_t *subjecthash)
{
	W i;
	bchanl_subjecthashnode_t *buf,*buf_next;
	Bool empty;

	for(i = 0;i < subjecthash->base; i++){
		buf = subjecthash->tbl + i;
		for(;;){
			empty = isQueEmpty(&(buf->queue));
			if (empty == True) {
				break;
			}
			buf_next = (bchanl_subjecthashnode_t*)buf->queue.next;
			QueRemove(&buf_next->queue);
			bchanl_subjecthashnode_delete(buf_next);
		}
	}

	free(subjecthash);
}
