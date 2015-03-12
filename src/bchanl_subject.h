/*
 * bchanl_subject.h
 *
 * Copyright (c) 2009-2015 project bchan
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
#include    <tad.h>
#include    <btron/dp.h>

#include    "subjectcache.h"
#include    "subjectparser.h"
#include    "subjectlist.h"
#include    "subjectlayout.h"

#ifndef __BCHANL_SUBJECT_H__
#define __BCHANL_SUBJECT_H__

typedef struct bchanl_subject_t_ bchanl_subject_t;
typedef struct bchanl_subjecthash_t_ bchanl_subjecthash_t;

IMPORT VOID bchanl_subject_gettitle(bchanl_subject_t *subject, TC **title, W *title_len);
IMPORT sbjtcache_t* bchanl_subject_getcache(bchanl_subject_t *subject);
IMPORT sbjtlayout_t* bchanl_subject_getlayout(bchanl_subject_t *subject);
IMPORT sbjtdraw_t* bchanl_subject_getdraw(bchanl_subject_t *subject);
#define BCHANL_SUBJECT_SORTBY_NUMBER SBJTLIST_SORTBY_NUMBER
#define BCHANL_SUBJECT_SORTBY_RES SBJTLIST_SORTBY_RES
#define BCHANL_SUBJECT_SORTBY_SINCE SBJTLIST_SORTBY_SINCE
#define BCHANL_SUBJECT_SORTBY_VIGOR SBJTLIST_SORTBY_VIGOR
IMPORT W bchanl_subject_relayout(bchanl_subject_t *subject);
IMPORT W bchanl_subject_reorder(bchanl_subject_t *subject, TC *filterword, W filterword_len, W sortby, Bool descending);
#define BCHANL_SUBJECT_CREATEVIEWERVOBJ_CANCELED 0
#define BCHANL_SUBJECT_CREATEVIEWERVOBJ_CREATED  1
IMPORT W bchanl_subject_createviewervobj(bchanl_subject_t *subject, sbjtlist_tuple_t *tuple, UB *fsnrec, W fsnrec_len, VOBJSEG *seg, LINK *lnk);
IMPORT VOID bchanl_subject_setresnumberdisplay(bchanl_subject_t *subject, Bool display);
IMPORT VOID bchanl_subject_setsincedisplay(bchanl_subject_t *subject, Bool display);
IMPORT VOID bchanl_subject_setvigordisplay(bchanl_subject_t *subject, Bool display);
IMPORT Bool bchanl_subject_getresnumverdisplay(bchanl_subject_t *subject);
IMPORT Bool bchanl_subject_getsincedisplay(bchanl_subject_t *subject);
IMPORT Bool bchanl_subject_getvigordisplay(bchanl_subject_t *subject);

IMPORT bchanl_subjecthash_t* bchanl_subjecthash_new(GID gid, W base);
IMPORT VOID bchanl_subjecthash_delete(bchanl_subjecthash_t *subjecthash);
IMPORT bchanl_subject_t* bchanl_subjecthash_search(bchanl_subjecthash_t *subjecthash, UB *host, W host_len, UH port, UB *board, W board_len);
IMPORT W bchanl_subjecthash_append(bchanl_subjecthash_t *subjecthash, UB *host, W host_len, UH port, UB *board, W board_len, TC *title, W title_len);

#endif
