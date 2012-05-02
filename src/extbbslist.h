/*
 * extbbslist.h
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
#include	<btron/dp.h>

#ifndef __EXTBBSLIST_H__
#define __EXTBBSLIST_H__

typedef struct extbbslist_t_ extbbslist_t;
typedef struct extbbslist_readcontext_t_ extbbslist_readcontext_t;
typedef struct extbbslist_editcontext_t_ extbbslist_editcontext_t;

IMPORT extbbslist_t* extbbslist_new(LINK *db_link, W rectype, UH subtype);
IMPORT VOID extbbslist_delete(extbbslist_t *list);
IMPORT W extbbslist_appenditem(extbbslist_t *list, TC *title, W title_len, UB *url, W url_len);
IMPORT W extbbslist_deleteitem(extbbslist_t *list, TC *title, W title_len);
IMPORT W extbbslist_number(extbbslist_t *list);
IMPORT W extbbslist_writefile(extbbslist_t *list);
IMPORT W extbbslist_readfile(extbbslist_t *list);

IMPORT extbbslist_readcontext_t* extbbslist_startread(extbbslist_t *list);
IMPORT VOID extbbslist_endread(extbbslist_t *list, extbbslist_readcontext_t *ctx);
IMPORT Bool extbbslist_readcontext_getnext(extbbslist_readcontext_t *ctx, TC **title, W *title_len, UB **url, W *url_len);

IMPORT extbbslist_editcontext_t* extbbslist_startedit(extbbslist_t *list);
IMPORT VOID extbbslist_endedit(extbbslist_t *list, extbbslist_editcontext_t *ctx, Bool update);
IMPORT W extbbslist_editcontext_append(extbbslist_editcontext_t *ctx, CONST TC *title, W title_len, CONST TC *url, W url_len);
IMPORT W extbbslist_editcontext_update(extbbslist_editcontext_t *ctx, W i, CONST TC *title, W title_len, CONST TC *url, W url_len);
IMPORT W extbbslist_editcontext_draw(extbbslist_editcontext_t *ctx, GID target, RECT *r);
IMPORT Bool extbbslist_editcontext_finditem(extbbslist_editcontext_t *ctx, PNT rel_pos, W *index);
IMPORT W extbbslist_editcontext_swapitem(extbbslist_editcontext_t *ctx, W i0, W i1);
IMPORT W extbbslist_editcontext_deleteitem(extbbslist_editcontext_t *ctx, W i);
IMPORT VOID extbbslist_editcontext_setselect(extbbslist_editcontext_t *ctx, W i);
IMPORT W extbbslist_editcontext_getselect(extbbslist_editcontext_t *ctx);
IMPORT Bool extbbslist_editcontext_ischanged(extbbslist_editcontext_t *ctx);
IMPORT VOID extbbslist_editcontext_setviewrect(extbbslist_editcontext_t *ctx, W l, W t, W r, W b);
IMPORT VOID extbbslist_editcontext_getviewrect(extbbslist_editcontext_t *ctx, W *l, W *t, W *r, W *b);
IMPORT VOID extbbslist_editcontext_scrollviewrect(extbbslist_editcontext_t *ctx, W dh, W dv);
IMPORT VOID extbbslist_editcontext_getdrawrect(extbbslist_editcontext_t *ctx, W *l, W *t, W *r, W *b);

#endif
