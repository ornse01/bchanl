/*
 * bchanl_hmi.c
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

#include    "bchanl_hmi.h"
#include    "hmi_wscr.h"

#include	<bstdio.h>
#include	<bstdlib.h>
#include	<tcode.h>
#include	<tstring.h>
#include	<btron/btron.h>
#include	<btron/hmi.h>
#include	<btron/vobj.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct subjectwindow_t_ {
	WID wid;
	GID gid;
	subjectwindow_scrollcalback scroll_callback;
	windowscroll_t wscr;
	VP arg;
	WEVENT savedwev;
};

struct bbsmenuwindow_t_ {
	WID wid;
	GID gid;
	bbsmenuwindow_scrollcalback scroll_callback;
	windowscroll_t wscr;
	VP arg;
	WEVENT savedwev;
};

#define BCHANLHMI_FLAG_SWITCHBUTDN 0x00000001

struct bchanlhmi_t_ {
	WEVENT wev;
	bchanlhmievent_t evt;
	UW flag;
	subjectwindow_t *subjectwindow;
	bbsmenuwindow_t *bbsmenuwindow;
};

EXPORT W subjectwindow_startredisp(subjectwindow_t *window, RECT *r)
{
	return wsta_dsp(window->wid, r, NULL);
}

EXPORT W subjectwindow_endredisp(subjectwindow_t *window)
{
	return wend_dsp(window->wid);
}

EXPORT W subjectwindow_eraseworkarea(subjectwindow_t *window, RECT *r)
{
	return wera_wnd(window->wid, r);
}

EXPORT W subjectwindow_scrollworkarea(subjectwindow_t *window, W dh, W dv)
{
	return wscr_wnd(window->wid, NULL, dh, dv, W_MOVE|W_RDSET);
}

EXPORT VOID subjectwindow_scrollbyvalue(subjectwindow_t *window, W dh, W dv)
{
	windowscroll_scrollbyvalue(&window->wscr, dh, dv);
}

EXPORT W subjectwindow_requestredisp(subjectwindow_t *window)
{
	return wreq_dsp(window->wid);
}

EXPORT GID subjectwindow_startdrag(subjectwindow_t *window)
{
	return wsta_drg(window->wid, 0);
}

EXPORT W subjectwindow_getdrag(subjectwindow_t *window, PNT *pos, WID *wid, PNT *pos_butup)
{
	W etype;

	etype = wget_drg(pos, &window->savedwev);
	*wid = window->savedwev.s.wid;
	if (etype == EV_BUTUP) {
		*pos_butup = window->savedwev.s.pos;
	}

	return etype;
}

EXPORT VOID subjectwindow_enddrag(subjectwindow_t *window)
{
	wend_drg();
}

EXPORT VOID subjectwindow_responsepasterequest(subjectwindow_t *window, W nak, PNT *pos)
{
	if (pos != NULL) {
		window->savedwev.r.r.p.rightbot.x = pos->x;
		window->savedwev.r.r.p.rightbot.y = pos->y;
	}
	wrsp_evt(&window->savedwev, nak);
}

EXPORT W subjectwindow_getworkrect(subjectwindow_t *window, RECT *r)
{
	return wget_wrk(window->wid, r);
}

EXPORT GID subjectwindow_getGID(subjectwindow_t *window)
{
	return wget_gid(window->wid);
}

EXPORT WID subjectwindow_getWID(subjectwindow_t *window)
{
	return window->wid;
}

EXPORT W subjectwindow_settitle(subjectwindow_t *window, TC *title)
{
	return wset_tit(window->wid, -1, title, 0);
}

EXPORT Bool subjectwindow_isactive(subjectwindow_t *window)
{
	WID wid;
	wid = wget_act(NULL);
	if (window->wid == wid) {
		return True;
	}
	return False;
}

EXPORT W subjectwindow_setdrawrect(subjectwindow_t *window, W l, W t, W r, W b)
{
	return windowscroll_setdrawrect(&window->wscr, l, t, r, b);
}

EXPORT W subjectwindow_setworkrect(subjectwindow_t *window, W l, W t, W r, W b)
{
	return windowscroll_setworkrect(&window->wscr, l, t, r, b);
}

EXPORT W bbsmenuwindow_startredisp(bbsmenuwindow_t *window, RECT *r)
{
	return wsta_dsp(window->wid, r, NULL);
}

EXPORT W bbsmenuwindow_endredisp(bbsmenuwindow_t *window)
{
	return wend_dsp(window->wid);
}

EXPORT W bbsmenuwindow_eraseworkarea(bbsmenuwindow_t *window, RECT *r)
{
	return wera_wnd(window->wid, r);
}

EXPORT W bbsmenuwindow_scrollworkarea(bbsmenuwindow_t *window, W dh, W dv)
{
	return wscr_wnd(window->wid, NULL, dh, dv, W_MOVE|W_RDSET);
}

EXPORT VOID bbsmenuwindow_scrollbyvalue(bbsmenuwindow_t *window, W dh, W dv)
{
	windowscroll_scrollbyvalue(&window->wscr, dh, dv);
}

EXPORT W bbsmenuwindow_requestredisp(bbsmenuwindow_t *window)
{
	return wreq_dsp(window->wid);
}

EXPORT GID bbsmenuwindow_startdrag(bbsmenuwindow_t *window)
{
	return wsta_drg(window->wid, 0);
}

EXPORT W bbsmenuwindow_getdrag(bbsmenuwindow_t *window, PNT *pos, WID *wid, PNT *pos_butup)
{
	W etype;

	etype = wget_drg(pos, &window->savedwev);
	*wid = window->savedwev.s.wid;
	if (etype == EV_BUTUP) {
		*pos_butup = window->savedwev.s.pos;
	}

	return etype;
}

EXPORT VOID bbsmenuwindow_enddrag(bbsmenuwindow_t *window)
{
	wend_drg();
}

EXPORT VOID bbsmenuwindow_responsepasterequest(bbsmenuwindow_t *window, W nak, PNT *pos)
{
	if (pos != NULL) {
		window->savedwev.r.r.p.rightbot.x = pos->x;
		window->savedwev.r.r.p.rightbot.y = pos->y;
	}
	wrsp_evt(&window->savedwev, nak);
}

EXPORT W bbsmenuwindow_getworkrect(bbsmenuwindow_t *window, RECT *r)
{
	return wget_wrk(window->wid, r);
}

EXPORT GID bbsmenuwindow_getGID(bbsmenuwindow_t *window)
{
	return wget_gid(window->wid);
}

EXPORT WID bbsmenuwindow_getWID(bbsmenuwindow_t *window)
{
	return window->wid;
}

EXPORT W bbsmenuwindow_settitle(bbsmenuwindow_t *window, TC *title)
{
	return wset_tit(window->wid, -1, title, 0);
}

EXPORT Bool bbsmenuwindow_isactive(bbsmenuwindow_t *window)
{
	WID wid;
	wid = wget_act(NULL);
	if (window->wid == wid) {
		return True;
	}
	return False;
}

EXPORT W bbsmenuwindow_setdrawrect(bbsmenuwindow_t *window, W l, W t, W r, W b)
{
	return windowscroll_setdrawrect(&window->wscr, l, t, r, b);
}

EXPORT W bbsmenuwindow_setworkrect(bbsmenuwindow_t *window, W l, W t, W r, W b)
{
	return windowscroll_setworkrect(&window->wscr, l, t, r, b);
}

LOCAL VOID bchanlhmi_setswitchbutdnflag(bchanlhmi_t *hmi)
{
	hmi->flag = hmi->flag | BCHANLHMI_FLAG_SWITCHBUTDN;
}

LOCAL VOID bchanlhmi_clearswitchbutdnflag(bchanlhmi_t *hmi)
{
	hmi->flag = hmi->flag & ~BCHANLHMI_FLAG_SWITCHBUTDN;
}

LOCAL Bool bchanlhmi_issetswitchbutdnflag(bchanlhmi_t *hmi)
{
	if ((hmi->flag & BCHANLHMI_FLAG_SWITCHBUTDN) == 0) {
		return False;
	}
	return True;
}

LOCAL WID bchanlhmi_getsubjectWID(bchanlhmi_t *hmi)
{
	if (hmi->subjectwindow == NULL) {
		return -1;
	}
	return hmi->subjectwindow->wid;
}

LOCAL WID bchanlhmi_getbbsmenuWID(bchanlhmi_t *hmi)
{
	if (hmi->bbsmenuwindow == NULL) {
		return -1;
	}
	return hmi->bbsmenuwindow->wid;
}

LOCAL VOID bchanlhmi_weventrequest(bchanlhmi_t *hmi, WEVENT *wev, bchanlhmievent_t *evt)
{
	WID subject_wid, bbsmenu_wid;

	subject_wid = bchanlhmi_getsubjectWID(hmi);
	bbsmenu_wid = bchanlhmi_getbbsmenuWID(hmi);

	switch (wev->g.cmd) {
	case	W_REDISP:	/*再表示要求*/
		if (wev->g.wid == subject_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_DRAW;
		} else if (wev->g.wid == bbsmenu_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_DRAW;
		}
		break;
	case	W_PASTE:	/*貼込み要求*/
		if (wev->g.wid == subject_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_PASTE;
			memcpy(&hmi->subjectwindow->savedwev, wev, sizeof(WEVENT));
		} else if (wev->g.wid == bbsmenu_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_PASTE;
			memcpy(&hmi->bbsmenuwindow->savedwev, wev, sizeof(WEVENT));
		} else {
			wrsp_evt(wev, 1); /*NACK*/
		}
		break;
	case	W_DELETE:	/*保存終了*/
		wrsp_evt(wev, 0);	/*ACK*/
		if (wev->g.wid == subject_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_CLOSE;
			evt->data.subject_close.save = True;
		} else if (wev->g.wid == bbsmenu_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_CLOSE;
			evt->data.bbsmenu_close.save = True;
		}
		break;
	case	W_FINISH:	/*廃棄終了*/
		wrsp_evt(wev, 0);	/*ACK*/
		if (wev->g.wid == subject_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_CLOSE;
			evt->data.subject_close.save = False;
		} else if (wev->g.wid == bbsmenu_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_CLOSE;
			evt->data.bbsmenu_close.save = False;
		}
		break;
	}
}

LOCAL VOID subjectwindow_resize(subjectwindow_t *window, SIZE *sz)
{
	RECT work;
	Bool workchange = False;

	wget_wrk(window->wid, &work);
	if (work.c.left != 0) {
		work.c.left = 0;
		workchange = True;
	}
	if (work.c.top != 0) {
		work.c.top = 0;
		workchange = True;
	}
	wset_wrk(window->wid, &work);
	gset_vis(window->gid, work);

	if (workchange == True) {
		wera_wnd(window->wid, NULL);
		wreq_dsp(window->wid);
	}

	sz->v = work.c.bottom - work.c.top;
	sz->h = work.c.right - work.c.left;
}

LOCAL VOID bbsmenuwindow_resize(bbsmenuwindow_t *window, SIZE *sz)
{
	RECT work;
	Bool workchange = False;

	wget_wrk(window->wid, &work);
	if (work.c.left != 0) {
		work.c.left = 0;
		workchange = True;
	}
	if (work.c.top != 0) {
		work.c.top = 0;
		workchange = True;
	}
	wset_wrk(window->wid, &work);
	gset_vis(window->gid, work);

	if (workchange == True) {
		wera_wnd(window->wid, NULL);
		wreq_dsp(window->wid);
	}

	sz->v = work.c.bottom - work.c.top;
	sz->h = work.c.right - work.c.left;
}

LOCAL VOID bchanlhmi_weventbutdn(bchanlhmi_t *hmi, WEVENT *wev, bchanlhmievent_t *evt)
{
	W i;
	WID subject_wid, bbsmenu_wid;

	subject_wid = bchanlhmi_getsubjectWID(hmi);
	bbsmenu_wid = bchanlhmi_getbbsmenuWID(hmi);

	switch	(wev->s.cmd) {
	case	W_PICT:
		switch (wchk_dck(wev->s.time)) {
		case	W_DCLICK:
			if (wev->s.wid == subject_wid) {
				evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_CLOSE;
				evt->data.subject_close.save = True; /* TODO: tmp value. */
			} else if (wev->s.wid == bbsmenu_wid) {
				evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_CLOSE;
				evt->data.bbsmenu_close.save = True; /* TODO: tmp value. */
			}
			return;
		case	W_PRESS:
			break;
		default:
			return;
		}
	case	W_FRAM:
	case	W_TITL:
		if (wmov_drg(wev, NULL) > 0) {
			if (wev->s.wid == subject_wid) {
				evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_DRAW;
			} else if (wev->s.wid == bbsmenu_wid) {
				evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_DRAW;
			}
		}
		return;
	case	W_LTHD:
	case	W_RTHD:
	case	W_LBHD:
	case	W_RBHD:
		switch (wchk_dck(wev->s.time)) {
		case	W_DCLICK:
			i = wchg_wnd(wev->s.wid, NULL, W_MOVE);
			break;
		case	W_PRESS:
			i = wrsz_drg(wev, NULL, NULL);
			break;
		default:
			return;
		}

		if (wev->s.wid == subject_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_RESIZE;
			subjectwindow_resize(hmi->subjectwindow, &evt->data.subject_resize.work_sz);
			windowscroll_updatebar(&hmi->subjectwindow->wscr);
			if (i > 0) {
				//evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_DRAW;
				/* TODO: queueing */
				evt->data.subject_resize.needdraw = True;
			} else {
				evt->data.subject_resize.needdraw = False;
			}
		} else if (wev->s.wid == bbsmenu_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_RESIZE;
			bbsmenuwindow_resize(hmi->bbsmenuwindow, &evt->data.bbsmenu_resize.work_sz);
			windowscroll_updatebar(&hmi->bbsmenuwindow->wscr);
			if (i > 0) {
				//evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_DRAW;
				/* TODO: queueing */
				evt->data.bbsmenu_resize.needdraw = True;
			} else {
				evt->data.bbsmenu_resize.needdraw = False;
			}
		}
		return;
	case	W_RBAR:
		if (wev->s.wid == subject_wid) {
			windowscroll_weventrbar(&hmi->subjectwindow->wscr, wev);
		} else if (wev->s.wid == bbsmenu_wid) {
			windowscroll_weventrbar(&hmi->bbsmenuwindow->wscr, wev);
		}
		/* ngword window need not do nothing. */
		return;
	case	W_BBAR:
		if (wev->s.wid == subject_wid) {
			windowscroll_weventbbar(&hmi->subjectwindow->wscr, wev);
		} else if (wev->s.wid == bbsmenu_wid) {
			windowscroll_weventbbar(&hmi->bbsmenuwindow->wscr, wev);
		}
		/* ngword window need not do nothing. */
		return;
	case	W_WORK:
		if (wev->s.wid == subject_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_BUTDN;
			evt->data.subject_butdn.type = wchk_dck(wev->s.time);
			evt->data.subject_butdn.pos = wev->s.pos;
			memcpy(&hmi->subjectwindow->savedwev, wev, sizeof(WEVENT));
		} else if (wev->s.wid == bbsmenu_wid) {
			evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_BUTDN;
			evt->data.bbsmenu_butdn.type = wchk_dck(wev->s.time);
			evt->data.bbsmenu_butdn.pos = wev->s.pos;
			memcpy(&hmi->bbsmenuwindow->savedwev, wev, sizeof(WEVENT));
		}
		return;
	}

	return;
}

LOCAL VOID bchanlhmi_weventswitch(bchanlhmi_t *hmi, WEVENT *wev, bchanlhmievent_t *evt)
{
	WID subject_wid, bbsmenu_wid;

	subject_wid = bchanlhmi_getsubjectWID(hmi);
	bbsmenu_wid = bchanlhmi_getbbsmenuWID(hmi);

	if (wev->s.wid == subject_wid) {
		evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_SWITCH;
		evt->data.subject_switch.needdraw = False;
	} else if (wev->s.wid == bbsmenu_wid) {
		evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_SWITCH;
		evt->data.bbsmenu_switch.needdraw = False;
	}
	bchanlhmi_setswitchbutdnflag(hmi);
}

LOCAL VOID bchanlhmi_weventreswitch(bchanlhmi_t *hmi, WEVENT *wev, bchanlhmievent_t *evt)
{
	WID subject_wid, bbsmenu_wid;

	subject_wid = bchanlhmi_getsubjectWID(hmi);
	bbsmenu_wid = bchanlhmi_getbbsmenuWID(hmi);

	if (wev->s.wid == subject_wid) {
		evt->type = BCHANLHMIEVENT_TYPE_SUBJECT_SWITCH;
		evt->data.subject_switch.needdraw = True;
	} else if (wev->s.wid == bbsmenu_wid) {
		evt->type = BCHANLHMIEVENT_TYPE_BBSMENU_SWITCH;
		evt->data.bbsmenu_switch.needdraw = True;
	}
	bchanlhmi_setswitchbutdnflag(hmi);
}

LOCAL VOID bchanlhmi_receivemessage(bchanlhmi_t *hmi, bchanlhmievent_t *evt)
{
	MESSAGE msg;
	W err;

    err = rcv_msg(MM_ALL, &msg, sizeof(MESSAGE), WAIT|NOCLR);
	if (err >= 0) {
		if (msg.msg_type == MS_TMOUT) { /* should be use other type? */
			evt->type = BCHANLHMIEVENT_TYPE_COMMON_TIMEOUT;
			evt->data.common_timeout.code = msg.msg_body.TMOUT.code;
		}
	}
	clr_msg(MM_ALL, MM_ALL);
}

EXPORT W bchanlhmi_getevent(bchanlhmi_t *hmi, bchanlhmievent_t **evt)
{
	WEVENT	*wev0;
	WID subject_wid, bbsmenu_wid;
	Bool ok;

	subject_wid = bchanlhmi_getsubjectWID(hmi);
	bbsmenu_wid = bchanlhmi_getbbsmenuWID(hmi);

	hmi->evt.type = BCHANLHMIEVENT_TYPE_NONE;
	wev0 = &hmi->wev;

	ok = bchanlhmi_issetswitchbutdnflag(hmi);
	if (ok == True) {
		bchanlhmi_weventbutdn(hmi, wev0, &hmi->evt);
		bchanlhmi_clearswitchbutdnflag(hmi);
		return 0;
	}

	wget_evt(wev0, WAIT);
	switch (wev0->s.type) {
	case	EV_NULL:
		cidl_par(wev0->s.wid, &wev0->s.pos);
		if ((wev0->s.wid != subject_wid)&&(wev0->s.wid != bbsmenu_wid)) {
			hmi->evt.type = BCHANLHMIEVENT_TYPE_COMMON_MOUSEMOVE;
			hmi->evt.data.common_mousemove.pos = wev0->s.pos;
			break;		/*ウィンドウ外*/
		}
		if (wev0->s.cmd != W_WORK)
			break;		/*作業領域外*/
		if (wev0->s.stat & ES_CMD)
			break;	/*命令キーが押されている*/
		if (wev0->s.wid == subject_wid) {
			hmi->evt.type = BCHANLHMIEVENT_TYPE_SUBJECT_MOUSEMOVE;
			hmi->evt.data.subject_mousemove.pos = wev0->s.pos;
			hmi->evt.data.subject_mousemove.stat = wev0->s.stat;
			break;
		}
		if (wev0->s.wid == bbsmenu_wid) {
			hmi->evt.type = BCHANLHMIEVENT_TYPE_BBSMENU_MOUSEMOVE;
			hmi->evt.data.bbsmenu_mousemove.pos = wev0->s.pos;
			hmi->evt.data.bbsmenu_mousemove.stat = wev0->s.stat;
			break;
		}
		break;
	case	EV_REQUEST:
		bchanlhmi_weventrequest(hmi, wev0, &hmi->evt);
		break;
	case	EV_RSWITCH:
		bchanlhmi_weventreswitch(hmi, wev0, &hmi->evt);
		break;
	case	EV_SWITCH:
		bchanlhmi_weventswitch(hmi, wev0, &hmi->evt);
		break;
	case	EV_BUTDWN:
		bchanlhmi_weventbutdn(hmi, wev0, &hmi->evt);
		break;
	case	EV_KEYDWN:
	case	EV_AUTKEY:
		hmi->evt.type = BCHANLHMIEVENT_TYPE_COMMON_KEYDOWN;
		hmi->evt.data.common_keydown.keycode = wev0->e.data.key.code;
		hmi->evt.data.common_keydown.keytop = wev0->e.data.key.keytop;
		hmi->evt.data.common_keydown.stat = wev0->e.stat;
		break;
	case	EV_INACT:
		pdsp_msg(NULL);
		break;
	case	EV_DEVICE:
		oprc_dev(&wev0->e, NULL, 0);
		break;
	case	EV_MSG:
		bchanlhmi_receivemessage(hmi, &hmi->evt);
		break;
	case	EV_MENU:
		hmi->evt.type = BCHANLHMIEVENT_TYPE_COMMON_MENU;
		hmi->evt.data.common_menu.pos = wev0->s.pos;
		break;
	}

	*evt = &hmi->evt;

	return 0;
}

LOCAL VOID subjectwindow_scroll(VP arg, W dh, W dv)
{
	subjectwindow_t *window = (subjectwindow_t*)arg;
	(*window->scroll_callback)(window->arg, dh, dv);
}

LOCAL subjectwindow_t* subjectwindow_new(RECT *r, TC *title, PAT *bgpat, subjectwindow_scrollcalback scrollcallback, VP arg)
{
	subjectwindow_t* window;
	W err;

	window = malloc(sizeof(subjectwindow_t));
	if (window == NULL) {
		return NULL;
	}

	window->wid = wopn_wnd(WA_SIZE|WA_HHDL|WA_VHDL|WA_BBAR|WA_RBAR, 0, r, NULL, 1, title, bgpat, NULL);
	if (window->wid < 0) {
		free(window);
		return NULL;
	}
	err = windowscroll_initialize(&window->wscr, window->wid, subjectwindow_scroll, (VP)window);
	if (err < 0) {
		wcls_wnd(window->wid, CLR);
		free(window);
		return NULL;
	}
	window->gid = wget_gid(window->wid);
	window->scroll_callback = scrollcallback;
	window->arg = arg;

	return window;
}

LOCAL VOID subjectwindow_delete(subjectwindow_t *window)
{
	wcls_wnd(window->wid, CLR);
	windowscroll_finalize(&window->wscr);
	free(window);
}

EXPORT subjectwindow_t* bchanlhmi_newsubjectwindow(bchanlhmi_t *hmi, RECT *r, TC *title, PAT *bgpat, subjectwindow_scrollcalback scrollcallback, VP arg)
{
	hmi->subjectwindow = subjectwindow_new(r, title, bgpat, scrollcallback, arg);
	return hmi->subjectwindow;
}

EXPORT VOID bchanlhmi_deletesubjectwindow(bchanlhmi_t *hmi, subjectwindow_t *window)
{
	subjectwindow_delete(window);
	hmi->subjectwindow = NULL;
}

LOCAL VOID bbsmenuwindow_scroll(VP arg, W dh, W dv)
{
	bbsmenuwindow_t *window = (bbsmenuwindow_t*)arg;
	(*window->scroll_callback)(window->arg, dh, dv);
}

LOCAL bbsmenuwindow_t* bbsmenuwindow_new(RECT *r, TC *title, PAT *bgpat, bbsmenuwindow_scrollcalback scrollcallback, VP arg)
{
	bbsmenuwindow_t* window;
	W err;

	window = malloc(sizeof(bbsmenuwindow_t));
	if (window == NULL) {
		return NULL;
	}

	window->wid = wopn_wnd(WA_SIZE|WA_HHDL|WA_VHDL|WA_BBAR|WA_RBAR, 0, r, NULL, 1, title, bgpat, NULL);
	if (window->wid < 0) {
		free(window);
		return NULL;
	}
	err = windowscroll_initialize(&window->wscr, window->wid, bbsmenuwindow_scroll, (VP)window);
	if (err < 0) {
		wcls_wnd(window->wid, CLR);
		free(window);
		return NULL;
	}
	window->gid = wget_gid(window->wid);
	window->scroll_callback = scrollcallback;
	window->arg = arg;

	return window;
}

LOCAL VOID bbsmenuwindow_delete(bbsmenuwindow_t *window)
{
	wcls_wnd(window->wid, CLR);
	windowscroll_finalize(&window->wscr);
	free(window);
}

EXPORT bbsmenuwindow_t* bchanlhmi_newbbsmenuwindow(bchanlhmi_t *hmi, RECT *r, TC *title, PAT *bgpat, bbsmenuwindow_scrollcalback scrollcallback, VP arg)
{
	hmi->bbsmenuwindow = bbsmenuwindow_new(r, title, bgpat, scrollcallback, arg);
	return hmi->bbsmenuwindow;
}

EXPORT VOID bchanlhmi_deletebbsmenuwindow(bchanlhmi_t *hmi, bbsmenuwindow_t *window)
{
	bbsmenuwindow_delete(window);
	hmi->bbsmenuwindow = NULL;
}

EXPORT bchanlhmi_t* bchanlhmi_new()
{
	bchanlhmi_t *hmi;

	hmi = (bchanlhmi_t *)malloc(sizeof(bchanlhmi_t));
	if (hmi == NULL) {
		return NULL;
	}
	hmi->subjectwindow = NULL;
	hmi->bbsmenuwindow = NULL;

	return hmi;
}

EXPORT VOID bchanlhmi_delete(bchanlhmi_t *hmi)
{
	if (hmi->bbsmenuwindow != NULL) {
		bbsmenuwindow_delete(hmi->bbsmenuwindow);
	}
	if (hmi->subjectwindow != NULL) {
		subjectwindow_delete(hmi->subjectwindow);
	}
	free(hmi);
}
