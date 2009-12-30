/*
 * bbsmenulayout.h
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
#include	<btron/dp.h>

#include    "bbsmenuparser.h"

#ifndef __BBSMENULAYOUT_H__
#define __BBSMENULAYOUT_H__

typedef struct bbsmnlayout_t_ bbsmnlayout_t;

IMPORT bbsmnlayout_t* bbsmnlayout_new(GID gid);
IMPORT VOID bbsmnlayout_delete(bbsmnlayout_t *layout);
IMPORT W bbsmnlayout_appenditem(bbsmnlayout_t *layout, bbsmnparser_item_t *parser_item);
IMPORT VOID bbsmnlayout_getdrawrect(bbsmnlayout_t *layout, W *l, W *t, W *r, W *b);
IMPORT TC* bbsmnlayout_gettitle(bbsmnlayout_t *layout);
IMPORT VOID bbsmnlayout_clear(bbsmnlayout_t *layout);

typedef struct bbsmndraw_t_ bbsmndraw_t;

IMPORT bbsmndraw_t* bbsmndraw_new(bbsmnlayout_t *layout);
IMPORT VOID bbsmndraw_delete(bbsmndraw_t *draw);
IMPORT W bbsmndraw_draw(bbsmndraw_t *draw, RECT *r);
IMPORT W bbsmndraw_findboard(bbsmndraw_t *draw, PNT rel_pos, bbsmnparser_item_t **item);
IMPORT VOID bbsmndraw_setviewrect(bbsmndraw_t *draw, W l, W t, W r, W b);
IMPORT VOID bbsmndraw_getviewrect(bbsmndraw_t *draw, W *l, W *t, W *r, W *b);
IMPORT VOID bbsmndraw_scrollviewrect(bbsmndraw_t *draw, W dh, W dv);

#endif
