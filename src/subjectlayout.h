/*
 * subjectlayout.h
 *
 * Copyright (c) 2009-2011 project bchan
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
#include	<btron/dp.h>

#include    "subjectlist.h"

#ifndef __SUBJECTLAYOUT_H__
#define __SUBJECTLAYOUT_H__

typedef struct sbjtlayout_t_ sbjtlayout_t;

IMPORT sbjtlayout_t* sbjtlayout_new(GID gid);
IMPORT VOID sbjtlayout_delete(sbjtlayout_t *layout);
IMPORT W sbjtlayout_appendthread(sbjtlayout_t *layout, sbjtlist_tuple_t *tuple);
IMPORT VOID sbjtlayout_getdrawrect(sbjtlayout_t *layout, W *l, W *t, W *r, W *b);
IMPORT TC* sbjtlayout_gettitle(sbjtlayout_t *layout);
IMPORT VOID sbjtlayout_clear(sbjtlayout_t *layout);
IMPORT VOID sbjtlayout_setfsspec(sbjtlayout_t *layout, FSSPEC *fspec);
IMPORT VOID sbjtlayout_setvobjbgcol(sbjtlayout_t *layout, COLOR color);
IMPORT COLOR sbjtlayout_getvobjbgcol(sbjtlayout_t *layout);

typedef struct sbjtdraw_t_ sbjtdraw_t;

IMPORT sbjtdraw_t* sbjtdraw_new(sbjtlayout_t *layout);
IMPORT VOID sbjtdraw_delete(sbjtdraw_t *draw);
IMPORT W sbjtdraw_draw(sbjtdraw_t *draw, RECT *r);
IMPORT W sbjtdraw_findthread(sbjtdraw_t *draw, PNT rel_pos, sbjtlist_tuple_t **thread, RECT *vframe);
IMPORT VOID sbjtdraw_setviewrect(sbjtdraw_t *draw, W l, W t, W r, W b);
IMPORT VOID sbjtdraw_getviewrect(sbjtdraw_t *draw, W *l, W *t, W *r, W *b);
IMPORT VOID sbjtdraw_scrollviewrect(sbjtdraw_t *draw, W dh, W dv);

#endif
