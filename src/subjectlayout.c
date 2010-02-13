/*
 * subjectlayout.c
 *
 * Copyright (c) 2009-2010 project bchan
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

#include    "subjectlayout.h"
#include    "subjectparser.h"
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

LOCAL TC dec[] = {TK_0,TK_1,TK_2,TK_3,TK_4,TK_5,TK_6,TK_7,TK_8,TK_9};

LOCAL W WtoTCS(W num, TC *dest)
{
	W digit,draw = 0,i = 0;

	digit = num / 1000000000 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num / 100000000 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num / 10000000 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num / 1000000 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num / 100000 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num / 10000 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num / 1000 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num / 100 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num / 10 % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	digit = num % 10;
	if ((digit != 0)||(draw != 0)) {
		dest[i++] = dec[digit];
		draw = 1;
	}
	dest[i] = TNULL;

	return i;
}

typedef struct sbjtlayout_thread_t_ sbjtlayout_thread_t;
struct sbjtlayout_thread_t_ {
	sbjtparser_thread_t *parser_thread;
	W index;
	W view_l,view_t,view_r,view_b;
	W i_titlesepareter;
	SIZE sz_title;
	W baseline;
	RECT vframe;
};

struct sbjtlayout_t_ {
	GID target;
	W draw_l,draw_t,draw_r,draw_b;
	sbjtlayout_thread_t **layout_thread; /* should be QUEUE? */
	W len;
	FSSPEC fspec;
	COLOR vobjbgcol;
};

LOCAL sbjtlayout_thread_t* sbjtlayout_thread_new(sbjtparser_thread_t *thread)
{
	sbjtlayout_thread_t *layout_thread;
	TC *str;

	layout_thread = (sbjtlayout_thread_t*)malloc(sizeof(sbjtlayout_thread_t));
	if (layout_thread == NULL) {
		return NULL;
	}
	layout_thread->parser_thread = thread;
	layout_thread->view_l = 0;
	layout_thread->view_t = 0;
	layout_thread->view_r = 0;
	layout_thread->view_b = 0;

	str = tc_strrchr(thread->title, TK_LPAR);
	if (str == NULL) {
		layout_thread->i_titlesepareter = thread->title_len;
	} else {
		layout_thread->i_titlesepareter = (str - thread->title) - 1;
	}

	return layout_thread;
}

LOCAL VOID sbjtlayout_thread_delete(sbjtlayout_thread_t *layout_thread)
{
	sbjtparser_thread_delete(layout_thread->parser_thread);
	free(layout_thread);
}

LOCAL W sbjtlayout_thread_calcindexdrawsize(sbjtlayout_thread_t *layout_thread, GID gid, SIZE *sz)
{
	TC str[12];
	W err, len;

	len = WtoTCS(layout_thread->index, str);
	str[len++] = TK_COLN;
	str[len] = TNULL;

	err = gget_stw(gid, str, len, NULL, NULL);
	if (err < 0) {
		return err;
	}
	sz->h = err;
	err = gget_sth(gid, str, len, NULL, NULL);
	if (err < 0) {
		return err;
	}
	sz->v = err;

	return 0;
}

LOCAL W sbjtlayout_thread_calctitledrawsize(sbjtlayout_thread_t *layout_thread, GID gid, SIZE *sz)
{
	return tadlib_calcdrawsize(layout_thread->parser_thread->title, layout_thread->i_titlesepareter, gid, sz);
}

LOCAL W sbjtlayout_thread_calcresnumdrawsize(sbjtlayout_thread_t *layout_thread, GID gid, SIZE *sz)
{
	return tadlib_calcdrawsize(layout_thread->parser_thread->title + layout_thread->i_titlesepareter, layout_thread->parser_thread->title_len - layout_thread->i_titlesepareter, gid, sz);
}

LOCAL W sbjtlayout_thread_calcsize(sbjtlayout_thread_t *layout_res, GID gid, W top, W index)
{
	SIZE sz_index, sz_title, sz_resnum;
	W err;

	layout_res->index = index;

	err = sbjtlayout_thread_calcindexdrawsize(layout_res, gid, &sz_index);
	if (err < 0) {
		return err;
	}
	err = sbjtlayout_thread_calctitledrawsize(layout_res, gid, &sz_title);
	if (err < 0) {
		return err;
	}
	err = sbjtlayout_thread_calcresnumdrawsize(layout_res, gid, &sz_resnum);
	if (err < 0) {
		return err;
	}

	layout_res->sz_title = sz_title;

	layout_res->view_t = top + 2;
	layout_res->view_l = 0;
	layout_res->view_b = layout_res->view_t + sz_title.v + 16;
	layout_res->view_r = 16*6 + sz_title.h + sz_resnum.h;

	layout_res->baseline = 20;
	layout_res->vframe.c.left = sz_index.h + 16;
	layout_res->vframe.c.top = layout_res->baseline - sz_title.v - 1;
	layout_res->vframe.c.right = layout_res->vframe.c.left + sz_title.h + 21;
	layout_res->vframe.c.bottom = layout_res->baseline + 3;

	return err;
}

//#define sbjtlayout_fontconfig_class FTC_MINCHO
//#define sbjtlayout_fontconfig_class 0x000000c0 /* gothic */
#define sbjtlayout_fontconfig_class FTC_DEFAULT

LOCAL W sbjtlayout_setupgid(sbjtlayout_t *layout, GID gid)
{
	return gset_fon(gid, &layout->fspec);
}

EXPORT W sbjtlayout_appendthread(sbjtlayout_t *layout, sbjtparser_thread_t *parser_thread)
{
	sbjtlayout_thread_t *layout_thread;
	W len;

	layout_thread = sbjtlayout_thread_new(parser_thread);
	if (layout_thread == NULL) {
		return -1; /* TODO */
	}

	len = layout->len + 1;
	layout->layout_thread = (sbjtlayout_thread_t**)realloc(layout->layout_thread, sizeof(sbjtlayout_thread_t*)*len);
	layout->layout_thread[layout->len] = layout_thread;
	layout->len = len;

	sbjtlayout_setupgid(layout, layout->target);

	sbjtlayout_thread_calcsize(layout_thread, layout->target, layout->draw_b, layout->len);

	/* orrect */
	if (layout->draw_l > layout_thread->view_l) {
		layout->draw_l = layout_thread->view_l;
	}
	if (layout->draw_t > layout_thread->view_t) {
		layout->draw_t = layout_thread->view_t;
	}
	if (layout->draw_r < layout_thread->view_r) {
		layout->draw_r = layout_thread->view_r;
	}
	if (layout->draw_b < layout_thread->view_b) {
		layout->draw_b = layout_thread->view_b;
	}

	return 0;
}

EXPORT VOID sbjtlayout_getdrawrect(sbjtlayout_t *layout, W *l, W *t, W *r, W *b)
{
	*l = layout->draw_l;
	*t = layout->draw_t;
	*r = layout->draw_r;
	*b = layout->draw_b;
}

EXPORT VOID sbjtlayout_clear(sbjtlayout_t *layout)
{
	W i;

	if (layout->layout_thread != NULL) {
		for (i=0;i<layout->len;i++) {
			sbjtlayout_thread_delete(layout->layout_thread[i]);
		}
		free(layout->layout_thread);
	}
	layout->draw_l = 0;
	layout->draw_t = 0;
	layout->draw_r = 0;
	layout->draw_b = 0;
	layout->layout_thread = NULL;
	layout->len = 0;
}

EXPORT VOID sbjtlayout_setfsspec(sbjtlayout_t *layout, FSSPEC *fspec)
{
	memcpy(&layout->fspec, fspec, sizeof(FSSPEC));
}

EXPORT VOID sbjtlayout_setvobjbgcol(sbjtlayout_t *layout, COLOR color)
{
	layout->vobjbgcol = color;
}

EXPORT COLOR sbjtlayout_getvobjbgcol(sbjtlayout_t *layout)
{
	return layout->vobjbgcol;
}

EXPORT sbjtlayout_t* sbjtlayout_new(GID gid)
{
	sbjtlayout_t *layout;

	layout = (sbjtlayout_t*)malloc(sizeof(sbjtlayout_t));
	if (layout == NULL) {
		return NULL;
	}
	layout->target = gid;
	layout->draw_l = 0;
	layout->draw_t = 0;
	layout->draw_r = 0;
	layout->draw_b = 0;
	layout->layout_thread = NULL;
	layout->len = 0;
	layout->fspec.name[0] = TNULL;
	layout->fspec.attr = FT_PROP|FT_GRAYSCALE;
	layout->fspec.fclass = sbjtlayout_fontconfig_class;
	layout->fspec.size.h = 16;
	layout->fspec.size.v = 16;
	layout->vobjbgcol = 0x10000000;

	return layout;
}

EXPORT VOID sbjtlayout_delete(sbjtlayout_t *layout)
{
	W i;

	if (layout->layout_thread != NULL) {
		for (i=0;i<layout->len;i++) {
			sbjtlayout_thread_delete(layout->layout_thread[i]);
		}
		free(layout->layout_thread);
	}
	free(layout);
}

struct sbjtdraw_t_ {
	sbjtlayout_t *layout;
	W view_l, view_t, view_r, view_b;
};

LOCAL W sbjtdraw_entrydraw_drawtitle(sbjtlayout_thread_t *entry, GID gid, W dh, W dv)
{
	TC *str = entry->parser_thread->title;
	W len = entry->i_titlesepareter;
	return tadlib_drawtext(str, len, gid, dh, dv);
}

LOCAL W sbjtdraw_entrydraw_drawresnum(sbjtlayout_thread_t *entry, GID gid, W dh, W dv)
{
	TC *str = entry->parser_thread->title + entry->i_titlesepareter;
	W len = entry->parser_thread->title_len - entry->i_titlesepareter;
	return tadlib_drawtext(str, len, gid, dh, dv);
}

LOCAL W sbjtdraw_entrydraw_resnumber(sbjtlayout_thread_t *entry, W resnum, GID target)
{
	TC str[11];
	W len;

	len = WtoTCS(resnum, str);
	return gdra_str(target, str, len, G_STORE);
}

LOCAL int sectrect_tmp(RECT a, W left, W top, W right, W bottom)
{
	return (a.c.left<right && left<a.c.right && a.c.top<bottom && top<a.c.bottom);
}

LOCAL W sbjtdraw_drawthread(sbjtlayout_thread_t *entry, W index, GID target, RECT *r, W dh, W dv, COLOR vobjbgcol)
{
	W sect, err;
	RECT view, vframe, vframe_b;
	static	PAT	pat0 = {{
		0,
		16, 16,
		0x10000000,
		0x10000000,
		FILL100
	}};
	static	PAT	pat1 = {{
		0,
		16, 16,
		0x10000000,
		0x10000000,
		FILL100
	}};

	/* sectrect */
	sect = sectrect_tmp(*r, entry->view_l - dh, entry->view_t - dv, entry->view_r - dh, entry->view_b - dv);
	if (sect == 0) {
		return 0;
	}

	view.c.left = entry->view_l - dh;
	view.c.top = entry->view_t - dv;
	view.c.right = entry->view_r - dh;
	view.c.bottom = entry->view_b - dv;

	err = gset_chp(target, - dh, entry->view_t + entry->baseline - dv, 1);
	if (err < 0) {
		return err;
	}
	err = sbjtdraw_entrydraw_resnumber(entry, index+1, target);
	if (err < 0) {
		return err;
	}
	err = gdra_chr(target, TK_COLN, G_STORE);
	if (err < 0) {
		return err;
	}

	err = gset_chp(target, 16, 0, 0);
	if (err < 0) {
		return err;
	}

	vframe.c.left = entry->vframe.c.left + view.c.left;
	vframe.c.top = entry->vframe.c.top + view.c.top;
	vframe.c.right = entry->vframe.c.right + view.c.left;
	vframe.c.bottom = entry->vframe.c.bottom + view.c.top;
	vframe_b.c.left = vframe.c.left + 2;
	vframe_b.c.top = vframe.c.top + 2;
	vframe_b.c.right = vframe.c.right + 2;
	vframe_b.c.bottom = vframe.c.bottom + 2;
	pat1.spat.fgcol = vobjbgcol;
	gfra_rec(target, vframe_b, 1, &pat0, 0, G_STORE);
	gfil_rec(target, vframe, &pat1, 0, G_STORE);
	gfra_rec(target, vframe, 1, &pat0, 0, G_STORE);
	gset_chc(target, 0x10000000, vobjbgcol);

	err = gset_chp(target, entry->vframe.c.left+view.c.left+10, entry->baseline + view.c.top, 1);
	if (err < 0) {
		return err;
	}

	err = sbjtdraw_entrydraw_drawtitle(entry, target, dh, dv);
	if (err < 0) {
		return err;
	}

	gset_chc(target, 0x10000000, 0x10FFFFFF);
	err = gset_chp(target, 16, 0, 0);
	if (err < 0) {
		return err;
	}
	err = sbjtdraw_entrydraw_drawresnum(entry, target, dh, dv);
	if (err < 0) {
		return err;
	}

	return 0;
}

EXPORT W sbjtdraw_draw(sbjtdraw_t *draw, RECT *r)
{
	W i,err;
	GID target;
	sbjtlayout_t *layout;

	layout = draw->layout;
	target = layout->target;

	for (i=0;i < layout->len;i++) {
		sbjtlayout_setupgid(layout, layout->target);
		err = sbjtdraw_drawthread(layout->layout_thread[i], i, target, r, draw->view_l, draw->view_t, layout->vobjbgcol);
		if (err < 0) {
			return err;
		}
	}

	return 0;
}

EXPORT W sbjtdraw_findthread(sbjtdraw_t *draw, PNT rel_pos, sbjtparser_thread_t **thread, RECT *vframe)
{
	W i,abs_x,abs_y,l,t,r,b;
	sbjtlayout_t *layout;
	sbjtlayout_thread_t *sbjt_thread;

	layout = draw->layout;
	abs_x = rel_pos.x + draw->view_l;
	abs_y = rel_pos.y + draw->view_t;

	for (i=0;i < layout->len;i++) {
		sbjt_thread = layout->layout_thread[i];
		l = sbjt_thread->view_l + sbjt_thread->vframe.c.left;
		t = sbjt_thread->view_t + sbjt_thread->vframe.c.top;
		r = sbjt_thread->view_l + sbjt_thread->vframe.c.right;
		b = sbjt_thread->view_t + sbjt_thread->vframe.c.bottom;
		if ((l <= abs_x)&&(abs_x < r)
			&&(t <= abs_y)&&(abs_y < b)) {
			*thread = sbjt_thread->parser_thread;
			if (vframe != NULL) {
				vframe->c.left = l - draw->view_l;
				vframe->c.top = t - draw->view_t;
				vframe->c.right = r - draw->view_l;
				vframe->c.bottom = b - draw->view_t;
			}
			return 1;
		}
	}

	return 0;
}

EXPORT VOID sbjtdraw_setviewrect(sbjtdraw_t *draw, W l, W t, W r, W b)
{
	draw->view_l = l;
	draw->view_t = t;
	draw->view_r = r;
	draw->view_b = b;
}

EXPORT VOID sbjtdraw_getviewrect(sbjtdraw_t *draw, W *l, W *t, W *r, W *b)
{
	*l = draw->view_l;
	*t = draw->view_t;
	*r = draw->view_r;
	*b = draw->view_b;
}

EXPORT VOID sbjtdraw_scrollviewrect(sbjtdraw_t *draw, W dh, W dv)
{
	draw->view_l += dh;
	draw->view_t += dv;
	draw->view_r += dh;
	draw->view_b += dv;
}

EXPORT sbjtdraw_t* sbjtdraw_new(sbjtlayout_t *layout)
{
	sbjtdraw_t *draw;

	draw = (sbjtdraw_t*)malloc(sizeof(sbjtdraw_t));
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

EXPORT VOID sbjtdraw_delete(sbjtdraw_t *draw)
{
	free(draw);
}
