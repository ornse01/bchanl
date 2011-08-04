/*
 * bchanl_hmi.h
 *
 * Copyright (c) 2011 project bchan
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
#include	<btron/hmi.h>

#ifndef __BCHANL_HMI_H__
#define __BCHANL_HMI_H__

typedef struct subjectwindow_t_ subjectwindow_t;
typedef VOID (*subjectwindow_scrollcalback)(VP arg, W dh, W dv);

IMPORT VOID subjectwindow_scrollbyvalue(subjectwindow_t *window, W dh, W dv);
IMPORT W subjectwindow_setdrawrect(subjectwindow_t *window, W l, W t, W r, W b);
IMPORT W subjectwindow_setworkrect(subjectwindow_t *window, W l, W t, W r, W b);
IMPORT VOID subjectwindow_responsepasterequest(subjectwindow_t *window, W nak, PNT *pos);
IMPORT W subjectwindow_startredisp(subjectwindow_t *window, RECT *r);
IMPORT W subjectwindow_endredisp(subjectwindow_t *window);
IMPORT W subjectwindow_eraseworkarea(subjectwindow_t *window, RECT *r);
IMPORT W subjectwindow_scrollworkarea(subjectwindow_t *window, W dh, W dv);
IMPORT W subjectwindow_getworkrect(subjectwindow_t *window, RECT *r);
IMPORT W subjectwindow_requestredisp(subjectwindow_t *window);
IMPORT GID subjectwindow_startdrag(subjectwindow_t *window);
IMPORT W subjectwindow_getdrag(subjectwindow_t *window, PNT *pos, WID *wid, PNT *pos_butup);
IMPORT VOID subjectwindow_enddrag(subjectwindow_t *window);
IMPORT GID subjectwindow_getGID(subjectwindow_t *window);
IMPORT WID subjectwindow_getWID(subjectwindow_t *window);
IMPORT W subjectwindow_settitle(subjectwindow_t *window, TC *title);
IMPORT Bool subjectwindow_isactive(subjectwindow_t *window);

typedef struct bbsmenuwindow_t_ bbsmenuwindow_t;
typedef VOID (*bbsmenuwindow_scrollcalback)(VP arg, W dh, W dv);

IMPORT VOID bbsmenuwindow_scrollbyvalue(bbsmenuwindow_t *wscr, W dh, W dv);
IMPORT W bbsmenuwindow_setdrawrect(bbsmenuwindow_t *window, W l, W t, W r, W b);
IMPORT W bbsmenuwindow_setworkrect(bbsmenuwindow_t *window, W l, W t, W r, W b);
IMPORT VOID bbsmenuwindow_responsepasterequest(bbsmenuwindow_t *window, W nak, PNT *pos);
IMPORT W bbsmenuwindow_startredisp(bbsmenuwindow_t *window, RECT *r);
IMPORT W bbsmenuwindow_endredisp(bbsmenuwindow_t *window);
IMPORT W bbsmenuwindow_eraseworkarea(bbsmenuwindow_t *window, RECT *r);
IMPORT W bbsmenuwindow_scrollworkarea(bbsmenuwindow_t *window, W dh, W dv);
IMPORT W bbsmenuwindow_getworkrect(bbsmenuwindow_t *window, RECT *r);
IMPORT W bbsmenuwindow_requestredisp(bbsmenuwindow_t *window);
IMPORT GID bbsmenuwindow_startdrag(bbsmenuwindow_t *window);
IMPORT W bbsmenuwindow_getdrag(bbsmenuwindow_t *window, PNT *pos, WID *wid, PNT *pos_butup);
IMPORT VOID bbsmenuwindow_enddrag(bbsmenuwindow_t *window);
IMPORT GID bbsmenuwindow_getGID(bbsmenuwindow_t *window);
IMPORT WID bbsmenuwindow_getWID(bbsmenuwindow_t *window);
IMPORT W bbsmenuwindow_settitle(bbsmenuwindow_t *window, TC *title);
IMPORT Bool bbsmenuwindow_isactive(bbsmenuwindow_t *window);

enum {
	BCHANLHMIEVENT_TYPE_NONE,
	BCHANLHMIEVENT_TYPE_COMMON_MOUSEMOVE,
	BCHANLHMIEVENT_TYPE_COMMON_KEYDOWN,
	BCHANLHMIEVENT_TYPE_COMMON_MENU,
	BCHANLHMIEVENT_TYPE_COMMON_TIMEOUT,
	BCHANLHMIEVENT_TYPE_SUBJECT_DRAW,
	BCHANLHMIEVENT_TYPE_SUBJECT_RESIZE,
	BCHANLHMIEVENT_TYPE_SUBJECT_CLOSE,
	BCHANLHMIEVENT_TYPE_SUBJECT_BUTDN,
	BCHANLHMIEVENT_TYPE_SUBJECT_PASTE,
	BCHANLHMIEVENT_TYPE_SUBJECT_SWITCH,
	BCHANLHMIEVENT_TYPE_SUBJECT_MOUSEMOVE,
	BCHANLHMIEVENT_TYPE_BBSMENU_DRAW,
	BCHANLHMIEVENT_TYPE_BBSMENU_RESIZE,
	BCHANLHMIEVENT_TYPE_BBSMENU_CLOSE,
	BCHANLHMIEVENT_TYPE_BBSMENU_BUTDN,
	BCHANLHMIEVENT_TYPE_BBSMENU_PASTE,
	BCHANLHMIEVENT_TYPE_BBSMENU_SWITCH,
	BCHANLHMIEVENT_TYPE_BBSMENU_MOUSEMOVE,
};

struct bchanlhmi_eventdata_mousemove_t_ {
	PNT pos;
};

struct bchanlhmi_eventdata_keydown_t_ {
	TC keycode;
	UH keytop;
	UW stat;
};

struct bchanlhmi_eventdata_menu_t_ {
	PNT pos;
};

struct bchanlhmi_eventdata_timeout_t_ {
	W code;
};

struct subjectwindow_eventdata_draw_t_ {
};

struct subjectwindow_eventdata_resize_t_ {
	SIZE work_sz;
	Bool needdraw;
};

struct subjectwindow_eventdata_close_t_ {
	Bool save;
};

struct subjectwindow_eventdata_butdn_t_ {
	W type;
	PNT pos;
};

struct subjectwindow_eventdata_paste_t_ {
};

struct subjectwindow_eventdata_switch_t_ {
	Bool needdraw;
};

struct subjectwindow_eventdata_mousemove_t_ {
	PNT pos;
	UW stat;
};

struct bbsmenuwindow_eventdata_draw_t_ {
};

struct bbsmenuwindow_eventdata_resize_t_ {
	SIZE work_sz;
	Bool needdraw;
};

struct bbsmenuwindow_eventdata_close_t_ {
	Bool save;
};

struct bbsmenuwindow_eventdata_butdn_t_ {
	W type;
	PNT pos;
};

struct bbsmenuwindow_eventdata_paste_t_ {
};

struct bbsmenuwindow_eventdata_switch_t_ {
	Bool needdraw;
};

struct bbsmenuwindow_eventdata_mousemove_t_ {
	PNT pos;
	UW stat;
};

typedef struct bchanlhmi_eventdata_mousemove_t_ bchanlhmi_eventdata_mousemove_t;
typedef struct bchanlhmi_eventdata_keydown_t_ bchanlhmi_eventdata_keydown_t;
typedef struct bchanlhmi_eventdata_menu_t_ bchanlhmi_eventdata_menu_t;
typedef struct bchanlhmi_eventdata_timeout_t_ bchanlhmi_eventdata_timeout_t;
typedef struct subjectwindow_eventdata_draw_t_ subjectwindow_eventdata_draw_t;
typedef struct subjectwindow_eventdata_resize_t_ subjectwindow_eventdata_resize_t;
typedef struct subjectwindow_eventdata_close_t_ subjectwindow_eventdata_close_t;
typedef struct subjectwindow_eventdata_butdn_t_ subjectwindow_eventdata_butdn_t;
typedef struct subjectwindow_eventdata_paste_t_ subjectwindow_eventdata_paste_t;
typedef struct subjectwindow_eventdata_switch_t_ subjectwindow_eventdata_switch_t;
typedef struct subjectwindow_eventdata_mousemove_t_ subjectwindow_eventdata_mousemove_t;
typedef struct bbsmenuwindow_eventdata_draw_t_ bbsmenuwindow_eventdata_draw_t;
typedef struct bbsmenuwindow_eventdata_resize_t_ bbsmenuwindow_eventdata_resize_t;
typedef struct bbsmenuwindow_eventdata_close_t_ bbsmenuwindow_eventdata_close_t;
typedef struct bbsmenuwindow_eventdata_butdn_t_ bbsmenuwindow_eventdata_butdn_t;
typedef struct bbsmenuwindow_eventdata_paste_t_ bbsmenuwindow_eventdata_paste_t;
typedef struct bbsmenuwindow_eventdata_switch_t_ bbsmenuwindow_eventdata_switch_t;
typedef struct bbsmenuwindow_eventdata_mousemove_t_ bbsmenuwindow_eventdata_mousemove_t;

struct bchanlhmievent_t_ {
	W type;
	union  {
		bchanlhmi_eventdata_mousemove_t common_mousemove;
		bchanlhmi_eventdata_keydown_t common_keydown;
		bchanlhmi_eventdata_menu_t common_menu;
		bchanlhmi_eventdata_timeout_t common_timeout;
		subjectwindow_eventdata_draw_t subject_draw;
		subjectwindow_eventdata_resize_t subject_resize;
		subjectwindow_eventdata_close_t subject_close;
		subjectwindow_eventdata_butdn_t subject_butdn;
		subjectwindow_eventdata_paste_t subject_paste;
		subjectwindow_eventdata_switch_t subject_switch;
		subjectwindow_eventdata_mousemove_t subject_mousemove;
		bbsmenuwindow_eventdata_draw_t bbsmenu_draw;
		bbsmenuwindow_eventdata_resize_t bbsmenu_resize;
		bbsmenuwindow_eventdata_close_t bbsmenu_close;
		bbsmenuwindow_eventdata_butdn_t bbsmenu_butdn;
		bbsmenuwindow_eventdata_paste_t bbsmenu_paste;
		bbsmenuwindow_eventdata_switch_t bbsmenu_switch;
		bbsmenuwindow_eventdata_mousemove_t bbsmenu_mousemove;
	} data;
};
typedef struct bchanlhmievent_t_ bchanlhmievent_t;

typedef struct bchanlhmi_t_ bchanlhmi_t;

IMPORT bchanlhmi_t* bchanlhmi_new();
IMPORT VOID bchanlhmi_delete(bchanlhmi_t *hmi);
IMPORT W bchanlhmi_getevent(bchanlhmi_t *hmi, bchanlhmievent_t **evt);
IMPORT subjectwindow_t* bchanlhmi_newsubjectwindow(bchanlhmi_t *hmi, RECT *r, TC *title, PAT *bgpat, subjectwindow_scrollcalback scrollcallback, VP arg);
IMPORT bbsmenuwindow_t *bchanlhmi_newbbsmenuwindow(bchanlhmi_t *hmi, RECT *r, TC *title, PAT *bgpat, bbsmenuwindow_scrollcalback scrollcallback, VP arg);
IMPORT VOID bchanlhmi_deletesubjectwindow(bchanlhmi_t *hmi, subjectwindow_t *window);
IMPORT VOID bchanlhmi_deletebbsmenuwindow(bchanlhmi_t *hmi, bbsmenuwindow_t *window);

#endif
