/*
 * bbsmenulayout.c
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

#include    "bbsmenulayout.h"
#include    "bbsmenuparser.h"
#include    "tadlib.h"

#include	<bstdio.h>
#include	<bstdlib.h>
#include	<tstring.h>
#include	<tcode.h>
#include	<btron/btron.h>
#include	<btron/dp.h>
#include	<tad.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

typedef struct bbsmnlayout_item_t_ bbsmnlayout_item_t;
struct bbsmnlayout_item_t_ {
	bbsmnparser_item_t *parser_item;
	W index;
	W view_l,view_t,view_r,view_b;
};

struct bbsmnlayout_t_ {
	GID target;
	W draw_l,draw_t,draw_r,draw_b;
	bbsmnlayout_item_t **layout_item; /* should be QUEUE? */
	W len;
};

LOCAL bbsmnlayout_item_t* bbsmnlayout_item_new(bbsmnparser_item_t *item)
{
	bbsmnlayout_item_t *layout_item;

	layout_item = (bbsmnlayout_item_t*)malloc(sizeof(bbsmnlayout_item_t));
	if (layout_item == NULL) {
		return NULL;
	}
	layout_item->parser_item = item;
	layout_item->view_l = 0;
	layout_item->view_t = 0;
	layout_item->view_r = 0;
	layout_item->view_b = 0;

	return layout_item;
}

LOCAL VOID bbsmnlayout_item_delete(bbsmnlayout_item_t *layout_item)
{
	bbsmnparser_item_delete(layout_item->parser_item);
	free(layout_item);
}

LOCAL W bbsmnlayout_entrydraw_calctextdrawsize(TC *str, W len, GID gid, SIZE *sz)
{
	return tadlib_calcdrawsize(str, len, gid, sz);
}

LOCAL W bbsmnlayout_item_calctitledrawsize(bbsmnlayout_item_t *layout_item, GID gid, SIZE *sz)
{
	return bbsmnlayout_entrydraw_calctextdrawsize(layout_item->parser_item->title, layout_item->parser_item->title_len, gid, sz);
}

LOCAL W bbsmnlayout_item_calccategorydrawsize(bbsmnlayout_item_t *layout_item, GID gid, SIZE *sz)
{
	return bbsmnlayout_entrydraw_calctextdrawsize(layout_item->parser_item->category, layout_item->parser_item->category_len, gid, sz);
}

LOCAL W bbsmnlayout_item_calcsize(bbsmnlayout_item_t *layout_item, GID gid, W top)
{
	SIZE sz_body;
	W err;

	if (layout_item->parser_item->category != NULL) {
		err = bbsmnlayout_item_calccategorydrawsize(layout_item, gid, &sz_body);
		if (err < 0) {
			return err;
		}
	} else {
		err = bbsmnlayout_item_calctitledrawsize(layout_item, gid, &sz_body);
		if (err < 0) {
			return err;
		}
	}

	layout_item->view_t = top + 2;
	layout_item->view_l = 0;
	layout_item->view_b = layout_item->view_t + sz_body.v + 2;
	layout_item->view_r = sz_body.h + 2;

	return err;
}

//#define bbsmnlayout_fontconfig_class FTC_MINCHO
//#define bbsmnlayout_fontconfig_class 0x000000c0 /* gothic */
#define bbsmnlayout_fontconfig_class FTC_DEFAULT

LOCAL W bbsmnlayout_setupgid(bbsmnlayout_t *layout, GID gid)
{
	FSSPEC spec;

	gget_fon(gid, &spec, NULL);
	spec.attr |= FT_PROP;
	spec.attr |= FT_GRAYSCALE;
	spec.fclass = bbsmnlayout_fontconfig_class;
	spec.size.h = 16;
	spec.size.v = 16;
	gset_fon(gid, &spec);

	return 0;
}

EXPORT W bbsmnlayout_appenditem(bbsmnlayout_t *layout, bbsmnparser_item_t *parser_item)
{
	bbsmnlayout_item_t *layout_item;
	W len;

	layout_item = bbsmnlayout_item_new(parser_item);
	if (layout_item == NULL) {
		return -1; /* TODO */
	}

	len = layout->len + 1;
	layout->layout_item = (bbsmnlayout_item_t**)realloc(layout->layout_item, sizeof(bbsmnlayout_item_t*)*len);
	layout->layout_item[layout->len] = layout_item;
	layout->len = len;

	bbsmnlayout_setupgid(layout, layout->target);

	bbsmnlayout_item_calcsize(layout_item, layout->target, layout->draw_b);

	/* orrect */
	if (layout->draw_l > layout_item->view_l) {
		layout->draw_l = layout_item->view_l;
	}
	if (layout->draw_t > layout_item->view_t) {
		layout->draw_t = layout_item->view_t;
	}
	if (layout->draw_r < layout_item->view_r) {
		layout->draw_r = layout_item->view_r;
	}
	if (layout->draw_b < layout_item->view_b) {
		layout->draw_b = layout_item->view_b;
	}

	return 0;
}

EXPORT VOID bbsmnlayout_getdrawrect(bbsmnlayout_t *layout, W *l, W *t, W *r, W *b)
{
	*l = layout->draw_l;
	*t = layout->draw_t;
	*r = layout->draw_r;
	*b = layout->draw_b;
}

EXPORT VOID bbsmnlayout_clear(bbsmnlayout_t *layout)
{
	W i;

	if (layout->layout_item != NULL) {
		for (i=0;i<layout->len;i++) {
			bbsmnlayout_item_delete(layout->layout_item[i]);
		}
		free(layout->layout_item);
	}
	layout->draw_l = 0;
	layout->draw_t = 0;
	layout->draw_r = 0;
	layout->draw_b = 0;
	layout->layout_item = NULL;
	layout->len = 0;
}

EXPORT bbsmnlayout_t* bbsmnlayout_new(GID gid)
{
	bbsmnlayout_t *layout;

	layout = (bbsmnlayout_t*)malloc(sizeof(bbsmnlayout_t));
	if (layout == NULL) {
		return NULL;
	}
	layout->target = gid;
	layout->draw_l = 0;
	layout->draw_t = 0;
	layout->draw_r = 0;
	layout->draw_b = 0;
	layout->layout_item = NULL;
	layout->len = 0;

	return layout;
}

EXPORT VOID bbsmnlayout_delete(bbsmnlayout_t *layout)
{
	W i;

	if (layout->layout_item != NULL) {
		for (i=0;i<layout->len;i++) {
			bbsmnlayout_item_delete(layout->layout_item[i]);
		}
		free(layout->layout_item);
	}
	free(layout);
}

struct bbsmndraw_t_ {
	bbsmnlayout_t *layout;
	W view_l, view_t, view_r, view_b;
};

LOCAL W bbsmndraw_entrydraw_drawtext(TC *str, W len, GID gid, W dh, W dv)
{
	return tadlib_drawtext(str, len, gid, dh, dv);
}

LOCAL W bbsmndraw_entrydraw_drawtitle(bbsmnlayout_item_t *entry, GID gid, W dh, W dv)
{
	TC *str = entry->parser_item->title;
	W len = entry->parser_item->title_len;
	return bbsmndraw_entrydraw_drawtext(str, len, gid, dh, dv);
}

LOCAL W bbsmndraw_entrydraw_drawcategory(bbsmnlayout_item_t *entry, GID gid, W dh, W dv)
{
	TC *str = entry->parser_item->category;
	W len = entry->parser_item->category_len;
	return bbsmndraw_entrydraw_drawtext(str, len, gid, dh, dv);
}

LOCAL int sectrect_tmp(RECT a, W left, W top, W right, W bottom)
{
	return (a.c.left<right && left<a.c.right && a.c.top<bottom && top<a.c.bottom);
}

LOCAL W bbsmndraw_entrydraw(bbsmnlayout_item_t *entry, W index, GID target, RECT *r, W dh, W dv, W width)
{
	W sect, err;
	RECT view, frame;
	static	PAT	pat0 = {{
		0,
		16, 16,
		0x10000000,
		0x10000000,
		FILL100
	}};

	/* sectrect */
	sect = sectrect_tmp(*r, entry->view_l - dh, entry->view_t - dv, entry->view_r - dh + width + 1, entry->view_b - dv);
	if (sect == 0) {
		return 0;
	}

	view.c.left = entry->view_l - dh;
	view.c.top = entry->view_t - dv;
	view.c.right = entry->view_r - dh;
	view.c.bottom = entry->view_b - dv;
	frame.c.left = view.c.left;
	frame.c.top = view.c.top;
	frame.c.right = frame.c.left + width + 1;
	frame.c.bottom = view.c.bottom;

	err = gfra_rec(target, frame, 1, &pat0, 0, G_STORE);
	if (err < 0) {
		return err;
	}

	if (entry->parser_item->category != NULL) {
		err = gset_chp(target, dh, entry->view_t + 16 - dv, 1);
		if (err < 0) {
			return err;
		}
		gset_chc(target, 0x10CC3300, 0x10FFFFFF);
		err = bbsmndraw_entrydraw_drawcategory(entry, target, dh, dv);
		if (err < 0) {
			return err;
		}
	} else {
		err = gset_chp(target, 16 - dh, entry->view_t + 16 - dv, 1);
		if (err < 0) {
			return err;
		}
		gset_chc(target, 0x100000FF, 0x10FFFFFF);
		err = bbsmndraw_entrydraw_drawtitle(entry, target, dh, dv);
		if (err < 0) {
			return err;
		}
	}

	return 0;
}

EXPORT W bbsmndraw_draw(bbsmndraw_t *draw, RECT *r)
{
	W i,err;
	GID target;
	bbsmnlayout_t *layout;

	layout = draw->layout;
	target = layout->target;

	for (i=0;i < layout->len;i++) {
		bbsmnlayout_setupgid(layout, layout->target);
		err = bbsmndraw_entrydraw(layout->layout_item[i], i, target, r, draw->view_l, draw->view_t, draw->view_r - draw->view_l);
		if (err < 0) {
			return err;
		}
	}

	return 0;
}

EXPORT W bbsmndraw_findboard(bbsmndraw_t *draw, PNT rel_pos, bbsmnparser_item_t **item)
{
	W i,abs_x,abs_y;
	bbsmnlayout_t *layout;
	bbsmnlayout_item_t *bbsmn_item;

	layout = draw->layout;
	abs_x = rel_pos.x + draw->view_l;
	abs_y = rel_pos.y + draw->view_t;

	for (i=0;i < layout->len;i++) {
		bbsmn_item = layout->layout_item[i];
		if ((bbsmn_item->view_l <= abs_x)
			&&(abs_x < bbsmn_item->view_l + draw->view_r - draw->view_l)
			&&(bbsmn_item->view_t <= abs_y)
			&&(abs_y < bbsmn_item->view_b)) {
			*item = bbsmn_item->parser_item;
			return 1;
		}
	}

	return 0;
}

EXPORT VOID bbsmndraw_setviewrect(bbsmndraw_t *draw, W l, W t, W r, W b)
{
	draw->view_l = l;
	draw->view_t = t;
	draw->view_r = r;
	draw->view_b = b;
}

EXPORT VOID bbsmndraw_getviewrect(bbsmndraw_t *draw, W *l, W *t, W *r, W *b)
{
	*l = draw->view_l;
	*t = draw->view_t;
	*r = draw->view_r;
	*b = draw->view_b;
}

EXPORT VOID bbsmndraw_scrollviewrect(bbsmndraw_t *draw, W dh, W dv)
{
	draw->view_l += dh;
	draw->view_t += dv;
	draw->view_r += dh;
	draw->view_b += dv;
}

EXPORT bbsmndraw_t* bbsmndraw_new(bbsmnlayout_t *layout)
{
	bbsmndraw_t *draw;

	draw = (bbsmndraw_t*)malloc(sizeof(bbsmndraw_t));
	if (draw == NULL) {
		return NULL;
	}
	draw->layout = layout;
	draw->view_l = 0;
	draw->view_t = 0;
	draw->view_r = 0;
	draw->view_b = 0;

	return draw;
}

EXPORT VOID bbsmndraw_delete(bbsmndraw_t *draw)
{
	free(draw);
}
