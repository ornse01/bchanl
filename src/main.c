/*
 * main.c
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

#include	<basic.h>
#include	<bstdlib.h>
#include	<bstdio.h>
#include	<bstring.h>
#include	<errcode.h>
#include	<tstring.h>
#include	<keycode.h>
#include	<tcode.h>
#include	<btron/btron.h>
#include	<btron/dp.h>
#include	<btron/hmi.h>
#include	<btron/vobj.h>
#include	<btron/libapp.h>
#include	<btron/bsocket.h>

#include	"window.h"
#include	"subjectretriever.h"
#include	"subjectcache.h"
#include	"subjectparser.h"
#include	"subjectlist.h"
#include	"subjectlayout.h"
#include    "bbsmenuretriever.h"
#include    "bbsmenucache.h"
#include    "bbsmenuparser.h"
#include	"bbsmenufilter.h"
#include    "bbsmenulayout.h"
#include    "extbbslist.h"
#include    "util.h"

#include    "bchanl_subject.h"
#include    "bchanl_hmi.h"
#include    "bchanl_menus.h"
#include    "bchanl_panels.h"

#include	<http/http_connector.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

#define BCHANL_DBX_MENU_TEST 20
#define BCHANL_DBX_TEXT_MLIST0	21
#define BCHANL_DBX_TEXT_MLIST1	22
#define BCHANL_DBX_TEXT_MLIST2	23
#define BCHANL_DBX_VIEWER_FUSEN 24
#define BCHANL_DBX_TEXT_WINDOWTITLE_BBSMENU 25
#define BCHANL_DBX_TEXT_WINDOWTITLE_SUBJECT 26
#define BCHANL_DBX_TEXT_MSG_RETRBBSMENU 27
#define BCHANL_DBX_TEXT_MSG_RETRSUBJECT 28
#define BCHANL_DBX_TEXT_MSG_ERRRETR 29
#define BCHANL_DBX_TB_SBJTOPT_FLT 30
#define BCHANL_DBX_WS_SBJTOPT_ODR 31
#define BCHANL_DBX_WS_SBJTOPT_ODRBY	32
#define BCHANL_DBX_TEXT_CATE_EXTBBS 33

#define BCHANL_MENU_WINDOW 3

#define BCHANL_COMMONSTORAGE_EXTBBSLIST_RECTYPE 30
#define BCHANL_COMMONSTORAGE_EXTBBSLIST_SUBTYPE 1

LOCAL UB bchanl_httpheader_useragent[] = "Monazilla/1.00 (bchanl/0.201)";
LOCAL W bchanl_httpheader_useragent_len = sizeof(bchanl_httpheader_useragent) - 1;

typedef struct bchanl_hmistate_t_ bchanl_hmistate_t;
struct bchanl_hmistate_t_ {
	PTRSTL ptr;

	TC *msg_retr_bbsmenu;
	TC *msg_retr_subject;
	TC *msg_error_retr;
};

LOCAL VOID bchanl_hmistate_updateptrstyle(bchanl_hmistate_t *hmistate, PTRSTL ptr)
{
	if (hmistate->ptr == ptr) {
		return;
	}
	hmistate->ptr = ptr;
	gset_ptr(hmistate->ptr, NULL, -1, -1);
}

LOCAL VOID bchanl_hmistate_initialize(bchanl_hmistate_t *hmistate)
{
	W err;

	hmistate->ptr = PS_SELECT;

	err = dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_MSG_RETRBBSMENU, (void**)&hmistate->msg_retr_bbsmenu);
	if (err < 0) {
		DP_ER("dget_dtp: message retrieving error", err);
		hmistate->msg_retr_bbsmenu = NULL;
	}
	err = dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_MSG_RETRSUBJECT, (void**)&hmistate->msg_retr_subject);
	if (err < 0) {
		DP_ER("dget_dtp: message not modified error", err);
		hmistate->msg_retr_subject = NULL;
	}
	err = dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_MSG_ERRRETR, (void**)&hmistate->msg_error_retr);
	if (err < 0) {
		DP_ER("dget_dtp: message retrieve error error", err);
		hmistate->msg_error_retr = NULL;
	}
}

typedef struct bchanl_bbsmenu_t_ bchanl_bbsmenu_t;
struct bchanl_bbsmenu_t_ {
	W gid;

	bbsmnretriever_t *retriever;
	bbsmncache_t *cache;
	bbsmnparser_t *parser;
	bbsmnfilter_t *filter;
	bbsmnlayout_t *layout;
	bbsmndraw_t *draw;

	bchanl_subjecthash_t *subjecthash;
	extbbslist_t *extbbslist;
	extbbslist_editcontext_t *editctx;
	TC *category_extbbs;
};

#define BCHANL_NETWORK_FLAG_WAITHTTPEVENT 0x00000001

struct bchanl_t_ {
	W taskid;
	W flgid; /* for reduce TMOUT message sending. */

	bchanl_mainmenu_t mainmenu;
	VID vid;
	W exectype;

	bchanl_hmistate_t hmistate;

	http_connector_t *connector;

	sbjtretriever_t *retriever;

	bchanl_subjecthash_t *subjecthash;
	bchanl_bbsmenu_t bbsmenu;
	bchanl_subject_t *currentsubject;
	bchanl_subject_t *nextsubject;
	struct {
		Bool resnum;
		Bool since;
		Bool vigor;
	} subjectdisplay;

	bchanlhmi_t *hmi;
	subjectwindow_t *subjectwindow;
	bbsmenuwindow_t *bbsmenuwindow;
	subjectoptionwindow_t *subjectoptionwindow;
	registerexternalwindow_t *registerexternalwindow;
	externalbbswindow_t *externalbbswindow;
};
typedef struct bchanl_t_ bchanl_t;

LOCAL VOID bchanl_swapresnumberdisplay(bchanl_t *bchanl)
{
	if (bchanl->subjectdisplay.resnum != False) {
		bchanl->subjectdisplay.resnum = False;
	} else {
		bchanl->subjectdisplay.resnum = True;
	}
}

LOCAL VOID bchanl_swapsincedisplay(bchanl_t *bchanl)
{
	if (bchanl->subjectdisplay.since != False) {
		bchanl->subjectdisplay.since = False;
	} else {
		bchanl->subjectdisplay.since = True;
	}
}

LOCAL VOID bchanl_swapvigordisplay(bchanl_t *bchanl)
{
	if (bchanl->subjectdisplay.vigor != False) {
		bchanl->subjectdisplay.vigor = False;
	} else {
		bchanl->subjectdisplay.vigor = True;
	}
}

LOCAL VOID bchanl_killme(bchanl_t *bchanl);

LOCAL VOID bchanl_subjectwindow_draw(bchanl_t *bchanl)
{
	sbjtdraw_t *draw;
	RECT r;
	if (bchanl->currentsubject == NULL) {
		do {
			if (subjectwindow_startredisp(bchanl->subjectwindow, &r) == 0) {
				break;
			}
			subjectwindow_eraseworkarea(bchanl->subjectwindow, &r);
		} while (subjectwindow_endredisp(bchanl->subjectwindow) > 0);
	} else {
		draw = bchanl_subject_getdraw(bchanl->currentsubject);
		do {
			if (subjectwindow_startredisp(bchanl->subjectwindow, &r) == 0) {
				break;
			}
			subjectwindow_eraseworkarea(bchanl->subjectwindow, &r);
			sbjtdraw_draw(draw, &r);
		} while (subjectwindow_endredisp(bchanl->subjectwindow) > 0);
	}
}

LOCAL VOID bchanl_subjectwindow_scroll(bchanl_t *bchanl, W dh, W dv)
{
	sbjtdraw_t *draw;
	if (bchanl->currentsubject == NULL) {
		return;
	}
	draw = bchanl_subject_getdraw(bchanl->currentsubject);
	sbjtdraw_scrollviewrect(draw, dh, dv);
	subjectwindow_scrollworkarea(bchanl->subjectwindow, -dh, -dv);
	bchanl_subjectwindow_draw(bchanl);
}

LOCAL VOID bchanl_subjectwindow_resize(bchanl_t *bchanl, SIZE newsize)
{
	W l,t,r,b;
	sbjtdraw_t *draw;

	if (bchanl->currentsubject == NULL) {
		return;
	}
	draw = bchanl_subject_getdraw(bchanl->currentsubject);

	sbjtdraw_getviewrect(draw, &l, &t, &r, &b);

	r = l + newsize.h;
	b = t + newsize.v;

	sbjtdraw_setviewrect(draw, l, t, r, b);
	subjectwindow_setworkrect(bchanl->subjectwindow, l, t, r, b);

	bchanl_subjectwindow_draw(bchanl);
}

LOCAL VOID bchanl_subjectwindow_close(bchanl_t *bchanl)
{
	bchanl_killme(bchanl);
}

LOCAL VOID bchanl_subjectwindow_press(bchanl_t *bchanl, PNT evpos)
{
	sbjtlist_tuple_t *tuple;
	sbjtdraw_t *draw;
	WID wid_butup;
	W event_type, size, err, fsn_len, dx, dy;
	void *fsn;
	GID gid;
	PNT pos, p1, pos_butup;
	TR_VOBJREC vrec;
	TRAYREC tr_rec;
	WEVENT paste_ev;
	SEL_RGN	sel;
	RECT r0, vframe;

	if (bchanl->currentsubject == NULL) {
		return;
	}
	draw = bchanl_subject_getdraw(bchanl->currentsubject);

	err = sbjtdraw_findthread(draw, evpos, &tuple, &vframe);
	if (err == 0) {
		return;
	}

	gid = subjectwindow_startdrag(bchanl->subjectwindow);
	if (gid < 0) {
		DP_ER("wsta_drg error:", gid);
		return;
	}

	gget_fra(gid, &r0);
	gset_vis(gid, r0);

	dx = vframe.c.left - evpos.x;
	dy = vframe.c.top - evpos.y;

	p1 = evpos;
	sel.sts = 0;
	sel.rgn.r.c.left = vframe.c.left;
	sel.rgn.r.c.top = vframe.c.top;
	sel.rgn.r.c.right = vframe.c.right;
	sel.rgn.r.c.bottom = vframe.c.bottom;
	adsp_sel(gid, &sel, 1);

	gset_ptr(PS_GRIP, NULL, -1, -1);
	for (;;) {
		event_type = subjectwindow_getdrag(bchanl->subjectwindow, &pos, &wid_butup, &pos_butup);
		if (event_type == EV_BUTUP) {
			break;
		}
		if (event_type != EV_NULL) {
			continue;
		}
		if ((pos.x == p1.x)&&(pos.y == p1.y)) {
			continue;
		}
		adsp_sel(gid, &sel, 0);
		sel.rgn.r.c.left += pos.x - p1.x;
		sel.rgn.r.c.top += pos.y - p1.y;
		sel.rgn.r.c.right += pos.x - p1.x;
		sel.rgn.r.c.bottom += pos.y - p1.y;
		adsp_sel(gid, &sel, 1);
		p1 = pos;
	}
	gset_ptr(PS_SELECT, NULL, -1, -1);
	adsp_sel(gid, &sel, 0);
	subjectwindow_enddrag(bchanl->subjectwindow);

	/* BUTUP on self window or no window or system message panel */
	if ((wid_butup == subjectwindow_getWID(bchanl->subjectwindow))||(wid_butup == 0)||(wid_butup == -1)) {
		return;
	}

	err = oget_vob(-wid_butup, &vrec.vlnk, NULL, 0, &size);
	if (err < 0) {
		return;
	}

	err = dget_dtp(64, BCHANL_DBX_VIEWER_FUSEN, (void**)&fsn);
	if (err < 0) {
		DP_ER("dget_dtp: ", err);
		return;
	}
	fsn_len = dget_siz((B*)fsn);
	err = bchanl_subject_createviewervobj(bchanl->currentsubject, tuple, fsn, fsn_len, &vrec.vseg, (LINK*)&vrec.vlnk);
	if (err < 0) {
		DP_ER("bchanl_subject_createviewervobj error", err);
		return;
	}
	if (err == BCHANL_SUBJECT_CREATEVIEWERVOBJ_CANCELED) {
		DP(("canceled\n"));
		return;
	}

	tr_rec.id = TR_VOBJ;
	tr_rec.len = sizeof(TR_VOBJREC);
	tr_rec.dt = (B*)&vrec;
	err = tset_dat(&tr_rec, 1);
	if (err < 0) {
		err = del_fil(NULL, (LINK*)&vrec.vlnk, 0);
		if (err < 0) {
			DP_ER("error del_fil:", err);
		}
		return;
	}

	paste_ev.r.type = EV_REQUEST;
	paste_ev.r.r.p.rightbot.x = pos_butup.x + dx;
	paste_ev.r.r.p.rightbot.y = pos_butup.y + dy;
	paste_ev.r.cmd = W_PASTE;
	paste_ev.r.wid = wid_butup;
	err = wsnd_evt(&paste_ev);
	if (err < 0) {
		tset_dat(NULL, 0);
		err = del_fil(NULL, (LINK*)&vrec.vlnk, 0);
		if (err < 0) {
			DP_ER("error del_fil:", err);
		}
		return;
	}
	err = wwai_rsp(NULL, W_PASTE, 60000);
	if (err != W_ACK) {
		tset_dat(NULL, 0);
		err = del_fil(NULL, (LINK*)&vrec.vlnk, 0);
		if (err < 0) {
			DP_ER("error del_fil:", err);
		}
	}

	wswi_wnd(wid_butup, NULL);
}

LOCAL VOID bchanl_subjectwindow_butdn(bchanl_t *bchanl, W dck, PNT evpos)
{
	switch (dck) {
	case	W_CLICK:
	case	W_DCLICK:
	case	W_QPRESS:
	default:
		return;
	case	W_PRESS:
		bchanl_subjectwindow_press(bchanl, evpos);
	}
}

LOCAL VOID bchanl_setcurrentsubject(bchanl_t *bchanl, bchanl_subject_t *sbjt)
{
	bchanl->currentsubject = sbjt;
	bchanl_subject_setresnumberdisplay(sbjt, bchanl->subjectdisplay.resnum);
	bchanl_subject_setsincedisplay(sbjt, bchanl->subjectdisplay.since);
	bchanl_subject_setvigordisplay(sbjt, bchanl->subjectdisplay.vigor);
	subjectwindow_requestredisp(bchanl->subjectwindow);
}

LOCAL VOID bchanl_setnextsubject(bchanl_t *bchanl, bchanl_subject_t *sbjt)
{
	bchanl->nextsubject = sbjt;
}

LOCAL VOID bchanl_bbsmenuwindow_draw(bchanl_t *bchanl)
{
	RECT r;
	do {
		if (bbsmenuwindow_startredisp(bchanl->bbsmenuwindow, &r) == 0) {
			break;
		}
		bbsmenuwindow_eraseworkarea(bchanl->bbsmenuwindow, &r);
		bbsmndraw_draw(bchanl->bbsmenu.draw, &r);
	} while (bbsmenuwindow_endredisp(bchanl->bbsmenuwindow) > 0);
}

LOCAL VOID bchanl_bbsmenuwindow_scroll(bchanl_t *bchanl, W dh, W dv)
{
	bbsmndraw_scrollviewrect(bchanl->bbsmenu.draw, dh, dv);
	bbsmenuwindow_scrollworkarea(bchanl->bbsmenuwindow, -dh, -dv);
	bchanl_bbsmenuwindow_draw(bchanl);
}

LOCAL VOID bchanl_bbsmenuwindow_resize(bchanl_t *bchanl, SIZE newsize)
{
	W l,t,r,b;

	bbsmndraw_getviewrect(bchanl->bbsmenu.draw, &l, &t, &r, &b);

	r = l + newsize.h;
	b = t + newsize.v;

	bbsmndraw_setviewrect(bchanl->bbsmenu.draw, l, t, r, b);
	bbsmenuwindow_setworkrect(bchanl->bbsmenuwindow, l, t, r, b);

	bchanl_bbsmenuwindow_draw(bchanl);
}

LOCAL VOID bchanl_bbsmenuwindow_close(bchanl_t *bchanl)
{
	bchanl_killme(bchanl);
}

LOCAL VOID bchanl_updatesubjectorder(bchanl_t *bchanl, SUBJECTOPTIONWINDOW_ORDERVALUE_T order, SUBJECTOPTIONWINDOW_ORDERBYVALUE_T orderby, TC *filterword, W filterword_len)
{
	Bool descending;
	W sbjt_orderby;

	if (order == SUBJECTOPTIONWINDOW_ORDERVALUE_DESCENDING) {
		descending = True;
	} else {
		descending = False;
	}
	switch (orderby) {
	case SUBJECTOPTIONWINDOW_ORDERBYVALUE_NUMBER:
	default:
		sbjt_orderby = BCHANL_SUBJECT_SORTBY_NUMBER;
		break;
	case SUBJECTOPTIONWINDOW_ORDERBYVALUE_RES:
		sbjt_orderby = BCHANL_SUBJECT_SORTBY_RES;
		break;
	case SUBJECTOPTIONWINDOW_ORDERBYVALUE_SINCE:
		sbjt_orderby = BCHANL_SUBJECT_SORTBY_SINCE;
		break;
	case SUBJECTOPTIONWINDOW_ORDERBYVALUE_VIGOR:
		sbjt_orderby = BCHANL_SUBJECT_SORTBY_VIGOR;
		break;
	}

	bchanl_subject_reorder(bchanl->currentsubject, filterword, filterword_len, sbjt_orderby, descending);

	subjectwindow_requestredisp(bchanl->subjectwindow);
}

LOCAL VOID bchanl_changesubjectorder(bchanl_t *bchanl, W neworder)
{
	SUBJECTOPTIONWINDOW_ORDERBYVALUE_T orderby;
	W len;
	TC buf[512];

	if (bchanl->currentsubject == NULL) {
		return;
	}

	subjectoptionwindow_getorderbyvalue(bchanl->subjectoptionwindow, &orderby);
	len = subjectoptionwindow_getfiltertext(bchanl->subjectoptionwindow, buf, 512);

	bchanl_updatesubjectorder(bchanl, neworder, orderby, buf, len);
}

LOCAL VOID bchanl_changesubjectorderby(bchanl_t *bchanl, W neworderby)
{
	SUBJECTOPTIONWINDOW_ORDERBYVALUE_T order;
	W len;
	TC buf[512];

	if (bchanl->currentsubject == NULL) {
		return;
	}

	subjectoptionwindow_getordervalue(bchanl->subjectoptionwindow, &order);
	len = subjectoptionwindow_getfiltertext(bchanl->subjectoptionwindow, buf, 512);

	bchanl_updatesubjectorder(bchanl, order, neworderby, buf, len);
}

LOCAL VOID bchanl_changesubjectfilterword(bchanl_t *bchanl, TC *newstr, W newstr_len)
{
	sbjtlayout_t *layout;
	sbjtdraw_t *draw;
	SUBJECTOPTIONWINDOW_ORDERVALUE_T order;
	SUBJECTOPTIONWINDOW_ORDERBYVALUE_T orderby;
	RECT w_work;
	W l, t, r, b;

	if (bchanl->currentsubject == NULL) {
		return;
	}

	subjectoptionwindow_getordervalue(bchanl->subjectoptionwindow, &order);
	subjectoptionwindow_getorderbyvalue(bchanl->subjectoptionwindow, &orderby);

	bchanl_updatesubjectorder(bchanl, order, orderby, newstr, newstr_len);

	subjectwindow_getworkrect(bchanl->subjectwindow, &w_work);
	draw = bchanl_subject_getdraw(bchanl->currentsubject);
	sbjtdraw_setviewrect(draw, 0, 0, w_work.c.right, w_work.c.bottom);
	subjectwindow_setworkrect(bchanl->subjectwindow, 0, 0, w_work.c.right, w_work.c.bottom);

	layout = bchanl_subject_getlayout(bchanl->currentsubject);
	sbjtlayout_getdrawrect(layout, &l, &t, &r, &b);
	subjectwindow_setdrawrect(bchanl->subjectwindow, l, t, r, b);
}

LOCAL VOID bchanl_changedisplayattribute(bchanl_t *bchanl)
{
	sbjtlayout_t *layout;
	SUBJECTOPTIONWINDOW_ORDERBYVALUE_T order;
	SUBJECTOPTIONWINDOW_ORDERBYVALUE_T orderby;
	W len;
	TC buf[512];
	W l, t, r, b;

	if (bchanl->currentsubject == NULL) {
		return;
	}

	subjectoptionwindow_getordervalue(bchanl->subjectoptionwindow, &order);
	subjectoptionwindow_getorderbyvalue(bchanl->subjectoptionwindow, &orderby);
	len = subjectoptionwindow_getfiltertext(bchanl->subjectoptionwindow, buf, 512);

	bchanl_updatesubjectorder(bchanl, order, orderby, buf, len);

	layout = bchanl_subject_getlayout(bchanl->currentsubject);
	sbjtlayout_getdrawrect(layout, &l, &t, &r, &b);
	subjectwindow_setdrawrect(bchanl->subjectwindow, l, t, r, b);
}

LOCAL VOID bchanl_sendsubjectrequest(bchanl_t *bchanl, bchanl_subject_t *subject)
{
	sbjtcache_t *cache;
	W err;

	bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_BUSY);
	pdsp_msg(bchanl->hmistate.msg_retr_subject);

	cache = bchanl_subject_getcache(subject);
	err = sbjtretriever_sendrequest(bchanl->retriever, cache);
	if (err < 0) {
		pdsp_msg(bchanl->hmistate.msg_error_retr);
		bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);
		return;
	}
	bchanl_setnextsubject(bchanl, subject);
	set_flg(bchanl->flgid, BCHANL_NETWORK_FLAG_WAITHTTPEVENT);
}

LOCAL VOID bchanl_bbsmenuwindow_click(bchanl_t *bchanl, PNT pos)
{
	bbsmnparser_item_t *item;
	bchanl_subject_t *subject;
	W fnd;
	UB *host, *board;
	W host_len, board_len;
	UH port;
	TC *title;
	W title_len;

	fnd = bbsmndraw_findboard(bchanl->bbsmenu.draw, pos, &item);
	if (fnd == 0) {
		DP(("not found\n"));
		return;
	}
	if (item->category != NULL) {
		return;
	}
	bbsmnparser_item_gethostboard(item, &host, &host_len, &port, &board, &board_len);
	subject = bchanl_subjecthash_search(bchanl->subjecthash, host, host_len, port, board, board_len);
	if (subject == NULL) {
		DP(("not found by subject hash"));
		return;
	}
	bchanl_subject_gettitle(subject, &title, &title_len);

	bchanl_sendsubjectrequest(bchanl, subject);
}

LOCAL VOID bchanl_bbsmenuwindow_butdn(bchanl_t *bchanl, W dck, PNT evpos)
{
	switch (dck) {
	case	W_DCLICK:
	case	W_PRESS:
	case	W_QPRESS:
	default:
		return;
	case	W_CLICK:
		bchanl_bbsmenuwindow_click(bchanl, evpos);
	}
}

LOCAL W bchanl_bbsmenu_initialize(bchanl_bbsmenu_t *bchanl, GID gid, bchanl_subjecthash_t *subjecthash, LINK *storage, http_connector_t *connector)
{
	bbsmnretriever_t *retriever;
	bbsmncache_t *cache;
	bbsmnparser_t *parser;
	bbsmnfilter_t *filter;
	bbsmnlayout_t *layout;
	bbsmndraw_t *draw;
	extbbslist_t *extbbslist;
	TC *category_extbbs;
	W err;

	cache = bbsmncache_new();
	if (cache == NULL) {
		goto error_cache;
	}
	retriever = bbsmnretriever_new(connector, bchanl_httpheader_useragent, bchanl_httpheader_useragent_len);
	if (retriever == NULL) {
		goto error_retriever;
	}
	parser = bbsmnparser_new(cache);
	if (parser == NULL) {
		goto error_parser;
	}
	filter = bbsmnfilter_new();
	if (filter == NULL) {
		goto error_filter;
	}
	layout = bbsmnlayout_new(gid);
	if (layout == NULL) {
		goto error_layout;
	}
	draw = bbsmndraw_new(layout);
	if (draw == NULL) {
		goto error_draw;
	}
	extbbslist = extbbslist_new(storage, BCHANL_COMMONSTORAGE_EXTBBSLIST_RECTYPE, BCHANL_COMMONSTORAGE_EXTBBSLIST_SUBTYPE);
	if (extbbslist == NULL) {
		DP_ER("extbbslist_new", 0);
		goto error_extbbslist;
	}
	err = extbbslist_readfile(extbbslist);
	if (err < 0) {
		DP_ER("extbbslist_readfile", 0);
		goto error_extbbslist_readfile;
	}
	dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_CATE_EXTBBS, (void**)&category_extbbs);

	bchanl->gid = gid;
	bchanl->retriever = retriever;
	bchanl->cache = cache;
	bchanl->parser = parser;
	bchanl->filter = filter;
	bchanl->layout = layout;
	bchanl->draw = draw;
	bchanl->subjecthash = subjecthash;
	bchanl->extbbslist = extbbslist;
	bchanl->editctx = NULL;
	bchanl->category_extbbs = category_extbbs;

	return 0;

error_extbbslist_readfile:
	extbbslist_delete(extbbslist);
error_extbbslist:
	bbsmndraw_delete(draw);
error_draw:
	bbsmnlayout_delete(layout);
error_layout:
	bbsmnfilter_delete(filter);
error_filter:
	bbsmnparser_delete(parser);
error_parser:
	bbsmnretriever_delete(retriever);
error_retriever:
	bbsmncache_delete(cache);
error_cache:
	return -1; /* TODO */
}

LOCAL W bchanl_bbsmenu_appenditemtohash(bchanl_bbsmenu_t *bchanl, bbsmnparser_item_t *item)
{
	W err;
	UB *host, *board;
	W host_len, board_len;
	UH port;

	bbsmnparser_item_gethostboard(item, &host, &host_len, &port, &board, &board_len);
	err = bchanl_subjecthash_append(bchanl->subjecthash, host, host_len, port, board, board_len, item->title, item->title_len);
	return err;
}

LOCAL VOID bchanl_bbsmenu_registerexternalbbs(bchanl_bbsmenu_t *bchanl, TC *title, W title_len, TC *url, W url_len)
{
	extbbslist_editcontext_append(bchanl->editctx, title, title_len, url, url_len);
}

LOCAL VOID bchanl_bbsmenu_relayoutcache(bchanl_bbsmenu_t *bchanl)
{
	W err, ret;
	bbsmnparser_t *parser = bchanl->parser;
	bbsmnparser_item_t *item;
	bbsmnfilter_t *filter = bchanl->filter;
	bbsmnlayout_t *layout = bchanl->layout;

	for (;;) {
		err = bbsmnparser_getnextitem(parser, &item);
		if (err != 1) {
			break;
		}
		bbsmnfilter_inputitem(filter, item);
		for (;;) {
			ret = bbsmnfilter_outputitem(filter, &item);
			if (item != NULL) {
				if (item->category == NULL) {
					err = bchanl_bbsmenu_appenditemtohash(bchanl, item);
					if (err < 0) {
						return;
					}
				}
				err = bbsmnlayout_appenditem(layout, item);
				if (err < 0) {
					return;
				}
			}
			if (ret != BBSMNFILTER_OUTPUTITEM_CONTINUE) {
				break;
			}
		}
		if (ret == BBSMNFILTER_OUTPUTITEM_END) {
			printf("D\n");
			break;
		}
		if (ret != BBSMNFILTER_OUTPUTITEM_WAITNEXT) {
			/* TODO: error */
			break;
		}
/*
		if (item == NULL) {
			printf("F\n");
			break;
		}
*/
	}
}

LOCAL VOID bchanl_bbsmenu_relayoutexternal(bchanl_bbsmenu_t *bchanl)
{
	W err, ret, category_len, title_len, url_len;
	Bool cont;
	TC *category, *title;
	UB *url;
	extbbslist_readcontext_t *ctx;
	bbsmnparser_t *parser = bchanl->parser;
	bbsmnparser_item_t *item;
	bbsmnlayout_t *layout = bchanl->layout;
	extbbslist_t *list = bchanl->extbbslist;

	ret = extbbslist_number(bchanl->extbbslist);
	if (ret <= 0) {
		return;
	}

	category = bchanl->category_extbbs;
	category_len = tc_strlen(category);
	item = bbsmnparser_newcategoryitem(parser, category, category_len);
	if (item == NULL) {
		return;
	}
	err = bbsmnlayout_appenditem(layout, item);
	if (err < 0) {
		return;
	}

	ctx = extbbslist_startread(list);
	if (ctx == NULL) {
		return;
	}
	for (;;) {
		cont = extbbslist_readcontext_getnext(ctx, &title, &title_len, &url, &url_len);
		if (cont == False) {
			break;
		}

		item = bbsmnparser_newboarditem(parser, title, title_len, url, url_len);
		if (item == NULL) {
			break;
		}
		err = bchanl_bbsmenu_appenditemtohash(bchanl, item);
		if (err < 0) {
			break;
		}
		err = bbsmnlayout_appenditem(layout, item);
		if (err < 0) {
			break;
		}
	}
	extbbslist_endread(list, ctx);
}

LOCAL VOID bchanl_bbsmenu_relayout(bchanl_bbsmenu_t *bchanl, bbsmenuwindow_t *window)
{
	W l, t, r, b;

	bbsmnlayout_clear(bchanl->layout);
	bbsmnfilter_clear(bchanl->filter);
	bbsmnparser_clear(bchanl->parser);

	bchanl_bbsmenu_relayoutcache(bchanl);
	bchanl_bbsmenu_relayoutexternal(bchanl);

	bbsmnlayout_getdrawrect(bchanl->layout, &l, &t, &r, &b);
	bbsmenuwindow_setdrawrect(window, l, t, r, b);

	bbsmenuwindow_requestredisp(window);
}

LOCAL Bool bchanl_registerexternalbbs(bchanl_t *bchanl)
{
	TC title[128];
	TC url[256];
	W title_len, url_len, l, t, r, b;
	TCURL_CHECK_VALID_BBSURL ret;
	GID gid;

	title_len = registerexternalwindow_getboradnametext(bchanl->registerexternalwindow, title, 128);
	if (title_len < 0) {
		DP_ER("registerexternalwindow_getboradnametext error", title_len);
		return True;
	}
	title[title_len] = TNULL;
	url_len = registerexternalwindow_geturltext(bchanl->registerexternalwindow, url, 255);
	if (url_len < 0) {
		DP_ER("registerexternalwindow_geturltext error", url_len);
		return True;
	}
	url[url_len] = TNULL;

	ret = tcurl_check_valid_bbsurl(url, url_len);
	switch (ret) {
	case TCURL_CHECK_VALID_BBSURL_NO_LAST_SLSH:
		url[url_len] = TK_SLSH;
		url_len++;
		/* intentional */
	case TCURL_CHECK_VALID_BBSURL_OK:
		break;
	case TCURL_CHECK_VALID_BBSURL_INVALID_SCHEME:
		bchan_panels_urlerror_scheme();
		return False;
	case TCURL_CHECK_VALID_BBSURL_INVALID_HOST:
		bchan_panels_urlerror_host();
		return False;
	case TCURL_CHECK_VALID_BBSURL_INVALID_PATH:
		bchan_panels_urlerror_path();
		return False;
	}

	bchanl_bbsmenu_registerexternalbbs(&bchanl->bbsmenu, title, title_len, url, url_len);

	registerexternalwindow_setboradnametext(bchanl->registerexternalwindow, NULL, 0);
	registerexternalwindow_seturltext(bchanl->registerexternalwindow, NULL, 0);

	gid = externalbbswindow_getGID(bchanl->externalbbswindow);
	extbbslist_editcontext_getdrawrect(bchanl->bbsmenu.editctx, gid, &l, &t, &r, &b);
	externalbbswindow_setdrawrect(bchanl->externalbbswindow, l, t, r, b);
	externalbbswindow_requestredisp(bchanl->externalbbswindow);

	return True;
}

LOCAL VOID bchanl_externalbbswindow_draw(bchanl_t *bchanl)
{
	RECT r;

	do {
		if (externalbbswindow_startredisp(bchanl->externalbbswindow, &r) == 0) {
			break;
		}
		externalbbswindow_eraseworkarea(bchanl->externalbbswindow, &r);
		extbbslist_editcontext_draw(bchanl->bbsmenu.editctx, externalbbswindow_getGID(bchanl->externalbbswindow), &r);
	} while (externalbbswindow_endredisp(bchanl->externalbbswindow) > 0);
}

LOCAL VOID bchanl_externalbbswindow_resize(bchanl_t *bchanl, SIZE newsize)
{
	W l,t,r,b;

	extbbslist_editcontext_getviewrect(bchanl->bbsmenu.editctx, &l, &t, &r, &b);

	r = l + newsize.h;
	b = t + newsize.v;

	extbbslist_editcontext_setviewrect(bchanl->bbsmenu.editctx, l, t, r, b);
	externalbbswindow_setworkrect(bchanl->externalbbswindow, l, t, r, b);
}

LOCAL VOID bchanl_externalbbswindow_close(bchanl_t *bchanl)
{
	Bool changed, save = False;
	BCHAN_PANELS_SAVECONFIRM_RESULT confirm;

	changed = extbbslist_editcontext_ischanged(bchanl->bbsmenu.editctx);
	if (changed != False) {
		confirm = bchan_panels_saveconfirm();
		switch (confirm) {
		case BCHAN_PANELS_SAVECONFIRM_RESULT_CANCEL:
			return;
		case BCHAN_PANELS_SAVECONFIRM_RESULT_OK_NOSAVE:
			save = False;
			break;
		case BCHAN_PANELS_SAVECONFIRM_RESULT_OK_SAVE:
			save = True;
			break;
		default:
			break;
		}
	}

	extbbslist_endedit(bchanl->bbsmenu.extbbslist, bchanl->bbsmenu.editctx, save);
	bchanl->bbsmenu.editctx = NULL;
	externalbbswindow_close(bchanl->externalbbswindow);
	if (save != False) {
		bchanl_bbsmenu_relayout(&bchanl->bbsmenu, bchanl->bbsmenuwindow);
	}
}

LOCAL VOID bchanl_externalbbswindow_butdn(bchanl_t *bchanl, W type, PNT pos)
{
	Bool found;
	W sel;

	if (type == W_CLICK) {
		found = extbbslist_editcontext_finditem(bchanl->bbsmenu.editctx, pos, &sel);
		if (found != False) {
			extbbslist_editcontext_setselect(bchanl->bbsmenu.editctx, sel);
		}
		externalbbswindow_requestredisp(bchanl->externalbbswindow);
	}
}

LOCAL W bchanl_externalbbswindow_paste_readtray(bchanl_t *bchanl)
{
	W err, name_len, url_len;
	TC *name, *url;

	err = tray_getextbbsinfo(NULL, &name_len, NULL, &url_len);
	if (err < 0) {
		return 1;
	}

	name = malloc(sizeof(TC)*(name_len+1));
	if (name == NULL) {
		return 1;
	}
	url = malloc(sizeof(TC)*url_len+1);
	if (url == NULL) {
		free(name);
		return 1;
	}

	err = tray_getextbbsinfo(name, &name_len, url, &url_len);
	if (err < 0) {
		free(url);
		free(name);
		return 1;
	}
	name[name_len] = TNULL;
	url[url_len] = TNULL;

	registerexternalwindow_setboradnametext(bchanl->registerexternalwindow, name, name_len);
	registerexternalwindow_seturltext(bchanl->registerexternalwindow, url, url_len);
	registerexternalwindow_open(bchanl->registerexternalwindow);

	free(url);
	free(name);

	return 0;
}

LOCAL VOID bchanl_externalbbswindow_paste(bchanl_t *bchanl)
{
	W nak;
	PNT p = {0x8000, 0x8000};
	nak = bchanl_externalbbswindow_paste_readtray(bchanl);
	externalbbswindow_responsepasterequest(bchanl->externalbbswindow, nak, &p);
}

LOCAL VOID bchanl_externalbbswindow_scroll(bchanl_t *bchanl, W dh, W dv)
{
	extbbslist_editcontext_scrollviewrect(bchanl->bbsmenu.editctx, dh, dv);
	externalbbswindow_scrollworkarea(bchanl->externalbbswindow, -dh, -dv);
}

#define BCHANL_MESSAGE_RETRIEVER_RELAYOUT 1
#define BCHANL_MESSAGE_RETRIEVER_ERROR -1
#define BCHANL_MESSAGE_HTTP_EVENT 2

LOCAL Bool bchanl_bbsmenu_httpevent(bchanl_bbsmenu_t *bchanl, http_connector_event *hevent)
{
	Bool ok;
	W err;

	ok = bbsmnretriever_iswaitingendpoint(bchanl->retriever, hevent->endpoint);
	if (ok == False) {
		return False;
	}
	err = bbsmnretriever_recievehttpevent(bchanl->retriever, bchanl->cache, hevent);

	switch (err) {
	case BBSMNRETRIEVER_REQUEST_ALLRELOAD:
		req_tmg(0, BCHANL_MESSAGE_RETRIEVER_RELAYOUT);
		break;
	case BBSMNRETRIEVER_REQUEST_WAITNEXT:
		break;
	default:
		req_tmg(0, BCHANL_MESSAGE_RETRIEVER_ERROR);
		DP_ER("bbsmnretriever_recievehttpevent", err);
		break;
	}

	return True;
}

LOCAL Bool bchanl_subject_httpevent(bchanl_t *bchanl, http_connector_event *hevent)
{
	Bool ok;
	W err;
	sbjtcache_t *cache;
	sbjtlayout_t *layout;
	sbjtdraw_t *draw;
	TC *title;
	RECT w_work;
	W l, t, r, b, title_len;

	if (bchanl->nextsubject == NULL) {
		return False;
	}	

	ok = sbjtretriever_iswaitingendpoint(bchanl->retriever, hevent->endpoint);
	if (ok == False) {
		return False;
	}
	cache = bchanl_subject_getcache(bchanl->nextsubject);
	err = sbjtretriever_recievehttpevent(bchanl->retriever, cache, hevent);

	switch (err) {
	case SBJTRETRIEVER_REQUEST_ALLRELOAD:
		/* should asynchronous layout? */

		subjectoptionwindow_setfiltertext(bchanl->subjectoptionwindow, NULL, 0);
		err = subjectoptionwindow_setordervalue(bchanl->subjectoptionwindow, SUBJECTOPTIONWINDOW_ORDERVALUE_ASCENDING);
		subjectoptionwindow_setorderbyvalue(bchanl->subjectoptionwindow, SUBJECTOPTIONWINDOW_ORDERBYVALUE_NUMBER);

		bchanl_setcurrentsubject(bchanl, bchanl->nextsubject);
		bchanl_setnextsubject(bchanl, NULL);
		bchanl_subject_relayout(bchanl->currentsubject);

		subjectwindow_getworkrect(bchanl->subjectwindow, &w_work);
		draw = bchanl_subject_getdraw(bchanl->currentsubject);
		sbjtdraw_setviewrect(draw, 0, 0, w_work.c.right, w_work.c.bottom);
		subjectwindow_setworkrect(bchanl->subjectwindow, 0, 0, w_work.c.right, w_work.c.bottom);

		layout = bchanl_subject_getlayout(bchanl->currentsubject);
		sbjtlayout_getdrawrect(layout, &l, &t, &r, &b);
		subjectwindow_setdrawrect(bchanl->subjectwindow, l, t, r, b);

		bchanl_subject_gettitle(bchanl->currentsubject, &title, &title_len);
		subjectwindow_settitle(bchanl->subjectwindow, title);

		pdsp_msg(NULL);
		bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);

		break;
	case SBJTRETRIEVER_REQUEST_WAITNEXT:
		break;
	default:
		req_tmg(0, BCHANL_MESSAGE_RETRIEVER_ERROR);
		DP_ER("sbjtretriever_recievehttpevent", err);
		break;
	}

	return True;
}

LOCAL VOID bchanl_http_task(W arg)
{
	bchanl_t *bchanl;
	http_connector_t *connector;
	bbsmnretriever_t *retr;
	bbsmncache_t *cache;
	W err;

	bchanl = (bchanl_t*)arg;
	connector = bchanl->connector;
	retr = bchanl->bbsmenu.retriever;
	cache = bchanl->bbsmenu.cache;

	for (;;) {
		err = http_connector_waitconnection(connector, T_FOREVER);
		if (err < 0) {
			DP_ER("http_connector_waitconnection", err);
			req_tmg(0, BCHANL_MESSAGE_RETRIEVER_ERROR);
			break;
		}

		err = wai_flg(bchanl->flgid, BCHANL_NETWORK_FLAG_WAITHTTPEVENT, WF_AND, T_FOREVER);
		if (err < 0) {
			DP_ER("wai_flg", err);
		}
		req_tmg(0, BCHANL_MESSAGE_HTTP_EVENT);
	}

	ext_tsk();
}

LOCAL VOID bchanl_handle_httpevent(bchanl_t *bchanl)
{
	W err;
	http_connector_event hevent;
	Bool rcv;

	set_flg(bchanl->flgid, BCHANL_NETWORK_FLAG_WAITHTTPEVENT);

	err = http_connector_getevent(bchanl->connector, &hevent);
	if (err < 0) {
		return;
	}

	rcv = bchanl_bbsmenu_httpevent(&bchanl->bbsmenu, &hevent);
	if (rcv != False) {
		return;
	}

	rcv = bchanl_subject_httpevent(bchanl, &hevent);
}

LOCAL W bchanl_prepare_network(bchanl_t *bchanl)
{
	if (bchanl->retriever == NULL) {
		return 0;
	}

	bchanl->taskid = cre_tsk(bchanl_http_task, -1, (W)bchanl);
	if (bchanl->taskid < 0) {
		DP_ER("error cre_tsk:", bchanl->taskid);
		return -1;
	}
	bchanl->flgid = cre_flg(0, DELEXIT);
	if (bchanl->flgid < 0) {
		ter_tsk(bchanl->taskid);
		bchanl->taskid = -1;
		DP_ER("error cre_flg:", bchanl->flgid);
		return -1;
	}

	return 0;
}

LOCAL W bchanl_networkrequest_bbsmenu(bchanl_t *bchanl)
{
	W err;
	static UW lastrequest = 0;
	UW etime;

	if (bchanl->flgid < 0) {
		return 0;
	}

	err = get_etm(&etime);
	if (err < 0) {
		DP_ER("get_etm error:", err);
		return err;
	}
	if (lastrequest + 10000 > etime) {
		return 0;
	}
	lastrequest = etime;

	bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_BUSY);
	pdsp_msg(bchanl->hmistate.msg_retr_bbsmenu);

	err = bbsmnretriever_sendrequest(bchanl->bbsmenu.retriever, bchanl->bbsmenu.cache);
	if (err < 0) {
		DP_ER("bbsmnretriever_sendrequest error:", err);
		bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);
		return err;
	}
	set_flg(bchanl->flgid, BCHANL_NETWORK_FLAG_WAITHTTPEVENT);

	return 0;
}

LOCAL W bchanl_initialize(bchanl_t *bchanl, VID vid, W exectype, LINK *storage)
{
	static	RECT	r0 = {{400, 100, 700+7, 200+30}};
	static	RECT	r1 = {{100, 100, 300+7, 300+30}};
	static	RECT	r2 = {{400, 300, 800+7, 400+30}};
	static	PAT	white = {{0, 16, 16, 0x10ffffff, 0, FILL100}};
	static	PAT	bgpat0;
	static	PAT *bgpat;
	TC *title0 = NULL, *title1 = NULL;
	W err;
	WID wid;
	GID gid;
	RECT w_work;
	PNT p0 = {450, 0};
	http_connector_t *connector;
	sbjtretriever_t *retriever;
	bchanlhmi_t *hmi;
	bchanl_subjecthash_t *subjecthash;
	subjectwindow_t *subjectwindow;
	bbsmenuwindow_t *bbsmenuwindow;
	subjectoptionwindow_t *subjectoptionwindow;
	registerexternalwindow_t *registerexternalwindow;
	externalbbswindow_t *externalbbswindow;

	err = wget_inf(WI_PANELBACK, &bgpat0, sizeof(bgpat0));
	if (err != sizeof(bgpat0)) {
		bgpat = &white;
	} else {
		bgpat = &bgpat0;
	}

	connector = http_connector_new();
	if (connector == NULL) {
		DP_ER("http_connector_new error", 0);
		goto error_http_connector;
	}

	retriever = sbjtretriever_new(connector, bchanl_httpheader_useragent, bchanl_httpheader_useragent_len);
	if (retriever == NULL) {
		DP_ER("sbjtretriever_new error", 0);
		goto error_retriever;
	}
	hmi = bchanlhmi_new();
	if (hmi == NULL) {
		DP_ER("bchanlhmi_new error", 0);
		goto error_bchanlhmi;
	}
	dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_WINDOWTITLE_SUBJECT, (void**)&title0);
	subjectwindow = bchanlhmi_newsubjectwindow(hmi, &r0, 0, title0, NULL);
	if (subjectwindow == NULL) {
		DP_ER("bchanlhmi_newsubjectwindow error", 0);
		goto error_subjectwindow;
	}
	gid = subjectwindow_getGID(subjectwindow);
	subjecthash = bchanl_subjecthash_new(gid, 100);
	if (subjecthash == NULL) {
		DP_ER("bchanl_subjecthash_new error", 0);
		goto error_subjecthash;
	}
	subjectoptionwindow = bchanlhmi_newsubjectoptionwindow(hmi, &p0, subjectwindow, NULL, bgpat, BCHANL_DBX_TB_SBJTOPT_FLT, BCHANL_DBX_WS_SBJTOPT_ODR, BCHANL_DBX_WS_SBJTOPT_ODRBY);
	if (subjectoptionwindow == NULL) {
		DP_ER("bchanlhmi_newsubjectoptionwindow", 0);
		goto error_subjectoptionwindow;
	}
	dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_WINDOWTITLE_BBSMENU, (void**)&title1);
	bbsmenuwindow = bchanlhmi_newbbsmenuwindow(hmi, &r1, 0, title1, NULL);
	if (bbsmenuwindow == NULL) {
		DP_ER("bchanlhmi_newbbsmenuwindow error", 0);
		goto error_bbsmenuwindow;
	}
	gid = bbsmenuwindow_getGID(bbsmenuwindow);
	registerexternalwindow = bchanlhmi_newregisterexternalwindow(hmi, &p0, 0, NULL, bgpat);
	if (registerexternalwindow == NULL) {
		DP_ER("bchanlhmi_newregisterexternalwindow error", 0);
		goto error_registerexternalwindow;
	}
	externalbbswindow = bchanlhmi_newexternalbbswindow(hmi, &r2, 0, NULL, NULL);
	if (externalbbswindow == NULL) {
		DP_ER("bchanlhmi_newexternalbbswindow", 0);
		goto error_externalbbswindow;
	}
	err = bchanl_bbsmenu_initialize(&(bchanl->bbsmenu), gid, subjecthash, storage, connector);
	if (err < 0) {
		DP_ER("bchanl_bbsmenu_initialize error", err);
		goto error_bbsmenu;
	}
	err = bchanl_mainmenu_initialize(&(bchanl->mainmenu), BCHANL_DBX_MENU_TEST);
	if (err < 0) {
		DP_ER("bchanl_mainmenu_initialize %d", err);
		goto error_mainmenu;
	}

	bchanl_hmistate_initialize(&bchanl->hmistate);

	if (exectype == EXECREQ) {
		wid = bbsmenuwindow_getWID(bbsmenuwindow);
		osta_prc(vid, wid);
	}

	bbsmenuwindow_getworkrect(bbsmenuwindow, &w_work);
	bbsmndraw_setviewrect(bchanl->bbsmenu.draw, 0, 0, w_work.c.right, w_work.c.bottom);
	bbsmenuwindow_setworkrect(bbsmenuwindow, 0, 0, w_work.c.right, w_work.c.bottom);

	bchanl->connector = connector;
	bchanl->retriever = retriever;
	bchanl->subjecthash = subjecthash;

	bchanl->currentsubject = NULL;
	bchanl->nextsubject = NULL;
	bchanl->subjectdisplay.resnum = True;
	bchanl->subjectdisplay.since = False;
	bchanl->subjectdisplay.vigor = False;

	bchanl->vid = vid;
	bchanl->exectype = exectype;

	bchanl->hmi = hmi;
	bchanl->subjectwindow = subjectwindow;
	bchanl->bbsmenuwindow = bbsmenuwindow;
	bchanl->subjectoptionwindow = subjectoptionwindow;
	bchanl->registerexternalwindow = registerexternalwindow;
	bchanl->externalbbswindow = externalbbswindow;

	return 0;

error_mainmenu:
	//bchanl_bbsmenu_finalize(&(bchanl->bbsmenu));
error_bbsmenu:
	bchanlhmi_deleteexternalbbswindow(hmi, externalbbswindow);
error_externalbbswindow:
	bchanlhmi_deleteregisterexternalwindow(hmi, registerexternalwindow);
error_registerexternalwindow:
	bchanlhmi_deletebbsmenuwindow(hmi, bbsmenuwindow);
error_bbsmenuwindow:
	bchanlhmi_deletesubjectoptionwindow(hmi, subjectoptionwindow);
error_subjectoptionwindow:
	bchanl_subjecthash_delete(subjecthash);
error_subjecthash:
	bchanlhmi_deletesubjectwindow(hmi, subjectwindow);
error_subjectwindow:
	bchanlhmi_delete(hmi);
error_bchanlhmi:
	sbjtretriever_delete(retriever);
error_retriever:
	http_connector_delete(connector);
error_http_connector:
	return -1; /* TODO */
}

LOCAL VOID bchanl_killme(bchanl_t *bchanl)
{
	gset_ptr(PS_BUSY, NULL, -1, -1);
	pdsp_msg(NULL);

	extbbslist_writefile(bchanl->bbsmenu.extbbslist);
	if (bchanl->exectype == EXECREQ) {
		oend_prc(bchanl->vid, NULL, 0);
	}
	bchanl_mainmenu_finalize(&bchanl->mainmenu);
	bchanlhmi_deleteexternalbbswindow(bchanl->hmi, bchanl->externalbbswindow);
	bchanlhmi_deleteregisterexternalwindow(bchanl->hmi, bchanl->registerexternalwindow);
	bchanlhmi_deletebbsmenuwindow(bchanl->hmi, bchanl->bbsmenuwindow);
	bchanlhmi_deletesubjectoptionwindow(bchanl->hmi, bchanl->subjectoptionwindow);
	bchanl_subjecthash_delete(bchanl->subjecthash);
	bchanlhmi_deletesubjectwindow(bchanl->hmi, bchanl->subjectwindow);
	bchanlhmi_delete(bchanl->hmi);
	sbjtretriever_delete(bchanl->retriever);
	http_connector_delete(bchanl->connector);

	ext_prc(0);
}

LOCAL VOID bchanl_readbbsmenutestdata(bchanl_bbsmenu_t *bchanl, bbsmenuwindow_t *bchanl_window)
{
	TC fname[] = {TK_b, TK_b, TK_s, TK_m, TK_e, TK_n, TK_u, TK_PROD, TK_h, TK_t, TK_m, TK_l, TNULL};
	LINK lnk;
	W fd, len, err;
	UB *bin;
	RECT w_work;
	bbsmncache_t *cache = bchanl->cache;
	bbsmndraw_t *draw = bchanl->draw;

	err = get_lnk(fname, &lnk, F_NORM);
	if (err < 0) {
		DP_ER("error get_lnk", err);
		return;
	}
	fd = opn_fil(&lnk, F_READ, NULL);
	if (fd < 0) {
		return;
	}
	err = rea_rec(fd, 0, NULL, 0, &len, NULL);
	if (err < 0) {
		cls_fil(fd);
		return;
	}
	bin = malloc(len);
	if (bin == NULL) {
		cls_fil(fd);
		return;
	}
	err = rea_rec(fd, 0, bin, len, 0, NULL);
	if (err < 0) {
		free(bin);
		cls_fil(fd);
		return;
	}
	cls_fil(fd);

	bbsmncache_appenddata(cache, bin, len);
	free(bin);

	bbsmenuwindow_getworkrect(bchanl_window, &w_work);
	bbsmndraw_setviewrect(draw, 0, 0, w_work.c.right, w_work.c.bottom);
	bbsmenuwindow_setworkrect(bchanl_window, 0, 0, w_work.c.right, w_work.c.bottom);
}

LOCAL VOID bchanl_subjectwindow_keydwn(bchanl_t *bchanl, UH keycode, TC ch, UW stat)
{
	W l,t,r,b,l1,t1,r1,b1,scr;
	sbjtlayout_t *layout;
	sbjtdraw_t *draw;

	if (bchanl->currentsubject == NULL) {
		return;
	}
	draw = bchanl_subject_getdraw(bchanl->currentsubject);

	switch (ch) {
	case KC_CC_U:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		if (t < 16) {
			scr = -t;
		} else {
			scr = -16;
		}
		subjectwindow_scrollbyvalue(bchanl->subjectwindow, 0, scr);
		bchanl_subjectwindow_scroll(bchanl, 0, scr);
		break;
	case KC_CC_D:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		layout = bchanl_subject_getlayout(bchanl->currentsubject);
		sbjtlayout_getdrawrect(layout, &l1, &t1, &r1, &b1);
		if (b + 16 > b1) {
			scr = b1 - b;
		} else {
			scr = 16;
		}
		if (scr > 0) {
			subjectwindow_scrollbyvalue(bchanl->subjectwindow, 0, scr);
			bchanl_subjectwindow_scroll(bchanl, 0, scr);
		}
		break;
	case KC_CC_R:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		layout = bchanl_subject_getlayout(bchanl->currentsubject);
		sbjtlayout_getdrawrect(layout, &l1, &t1, &r1, &b1);
		if (r + 16 > r1) {
			scr = r1 - r;
		} else {
			scr = 16;
		}
		if (scr > 0) {
			subjectwindow_scrollbyvalue(bchanl->subjectwindow, scr, 0);
			bchanl_subjectwindow_scroll(bchanl, scr, 0);
		}
		break;
	case KC_CC_L:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		if (l < 16) {
			scr = -l;
		} else {
			scr = -16;
		}
		subjectwindow_scrollbyvalue(bchanl->subjectwindow, scr, 0);
		bchanl_subjectwindow_scroll(bchanl, scr, 0);
		break;
	case KC_PG_U:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		if (t < (b - t)) {
			scr = -t;
		} else {
			scr = - (b - t);
		}
		subjectwindow_scrollbyvalue(bchanl->subjectwindow, 0, scr);
		bchanl_subjectwindow_scroll(bchanl, 0, scr);
		break;
	case KC_PG_D:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		layout = bchanl_subject_getlayout(bchanl->currentsubject);
		sbjtlayout_getdrawrect(layout, &l1, &t1, &r1, &b1);
		if (b + (b - t) > b1) {
			scr = b1 - b;
		} else {
			scr = (b - t);
		}
		if (scr > 0) {
			subjectwindow_scrollbyvalue(bchanl->subjectwindow, 0, scr);
			bchanl_subjectwindow_scroll(bchanl, 0, scr);
		}
		break;
	case KC_PG_R:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		layout = bchanl_subject_getlayout(bchanl->currentsubject);
		sbjtlayout_getdrawrect(layout, &l1, &t1, &r1, &b1);
		if (r + (r - l) > r1) {
			scr = r1 - r;
		} else {
			scr = (r - l);
		}
		if (scr > 0) {
			subjectwindow_scrollbyvalue(bchanl->subjectwindow, scr, 0);
			bchanl_subjectwindow_scroll(bchanl, scr, 0);
		}
		break;
	case KC_PG_L:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		if (l < (r - l)) {
			scr = -l;
		} else {
			scr = - (r - l);
		}
		subjectwindow_scrollbyvalue(bchanl->subjectwindow, scr, 0);
		bchanl_subjectwindow_scroll(bchanl, scr, 0);
		break;
	case TK_E: /* temporary */
		if (stat & ES_CMD) {
			bchanl_killme(bchanl);
		}
		break;
	}
}

LOCAL VOID bchanl_bbsmenuwindow_keydwn(bchanl_t *bchanl, UH keycode, TC ch, UW stat)
{
	W l,t,r,b,l1,t1,r1,b1,scr;
	bbsmndraw_t *draw = bchanl->bbsmenu.draw;
	bbsmnlayout_t *layout = bchanl->bbsmenu.layout;

	switch (ch) {
	case KC_CC_U:
		bbsmndraw_getviewrect(draw, &l, &t, &r, &b);
		if (t < 16) {
			scr = -t;
		} else {
			scr = -16;
		}
		bbsmenuwindow_scrollbyvalue(bchanl->bbsmenuwindow, 0, scr);
		bchanl_bbsmenuwindow_scroll(bchanl, 0, scr);
		break;
	case KC_CC_D:
		bbsmndraw_getviewrect(draw, &l, &t, &r, &b);
		bbsmnlayout_getdrawrect(layout, &l1, &t1, &r1, &b1);
		if (b + 16 > b1) {
			scr = b1 - b;
		} else {
			scr = 16;
		}
		if (scr > 0) {
			bbsmenuwindow_scrollbyvalue(bchanl->bbsmenuwindow, 0, scr);
			bchanl_bbsmenuwindow_scroll(bchanl, 0, scr);
		}
		break;
	case KC_CC_R:
	case KC_CC_L:
		break;
	case KC_PG_U:
		bbsmndraw_getviewrect(draw, &l, &t, &r, &b);
		if (t < (b - t)) {
			scr = -t;
		} else {
			scr = - (b - t);
		}
		bbsmenuwindow_scrollbyvalue(bchanl->bbsmenuwindow, 0, scr);
		bchanl_bbsmenuwindow_scroll(bchanl, 0, scr);
		break;
	case KC_PG_D:
		bbsmndraw_getviewrect(draw, &l, &t, &r, &b);
		bbsmnlayout_getdrawrect(layout, &l1, &t1, &r1, &b1);
		if (b + (b - t) > b1) {
			scr = b1 - b;
		} else {
			scr = (b - t);
		}
		if (scr > 0) {
			bbsmenuwindow_scrollbyvalue(bchanl->bbsmenuwindow, 0, scr);
			bchanl_bbsmenuwindow_scroll(bchanl, 0, scr);
		}
		break;
	case KC_PG_R:
	case KC_PG_L:
		break;
	case KC_PF5:
		bchanl_networkrequest_bbsmenu(bchanl);
		break;
	case TK_E: /* temporary */
		if (stat & ES_CMD) {
			bchanl_killme(bchanl);
		}
		break;
	}
}


LOCAL VOID bchanl_keydwn(bchanl_t *bchanl, UH keytop, TC ch, UW stat)
{
	Bool act;

	act = subjectwindow_isactive(bchanl->subjectwindow);
	if (act == True) {
		bchanl_subjectwindow_keydwn(bchanl, keytop, ch, stat);
		return;
	}
	act = bbsmenuwindow_isactive(bchanl->bbsmenuwindow);
	if (act == True) {
		bchanl_bbsmenuwindow_keydwn(bchanl, keytop, ch, stat);
		return;
	}
}

enum BCHANL_TEXTBOX_MENU_TYPE_ {
	BCHANL_TEXTBOX_MENU_TYPE_NONE,
	BCHANL_TEXTBOX_MENU_TYPE_FILTER,
	BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE,
	BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL,
};
typedef enum BCHANL_TEXTBOX_MENU_TYPE_ BCHANL_TEXTBOX_MENU_TYPE;

LOCAL VOID bchanl_setupmenu(bchanl_t *bchanl, BCHANL_TEXTBOX_MENU_TYPE type)
{
	Bool isactive, isopen, isopen_extbbs, selected = False, fromtray, totray, trayempty;
	W index, num;

	isactive = subjectwindow_isactive(bchanl->subjectwindow);
	isopen = subjectoptionwindow_isopen(bchanl->subjectoptionwindow);
	isopen_extbbs = externalbbswindow_isopen(bchanl->externalbbswindow);
	if (isopen_extbbs != False) {
		index = extbbslist_editcontext_getselect(bchanl->bbsmenu.editctx);
		if (index >= 0) {
			selected = True;
		}
	}
	switch (type) {
	case BCHANL_TEXTBOX_MENU_TYPE_NONE:
	default:
		fromtray = totray = False;
		break;
	case BCHANL_TEXTBOX_MENU_TYPE_FILTER:
		trayempty = tray_isempty();
		if (trayempty == False) {
			fromtray = True;
		} else {
			fromtray = False;
		}
		num = subjectoptionwindow_cutfiltertext(bchanl->subjectoptionwindow, NULL, 0, False);
		if (num > 0) {
			totray = True;
		} else {
			totray = False;
		}
		break;
	case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE:
		trayempty = tray_isempty();
		if (trayempty == False) {
			fromtray = True;
		} else {
			fromtray = False;
		}
		num = registerexternalwindow_cutboradnametext(bchanl->registerexternalwindow, NULL, 0, False);
		if (num > 0) {
			totray = True;
		} else {
			totray = False;
		}
		break;
	case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL:
		trayempty = tray_isempty();
		if (trayempty == False) {
			fromtray = True;
		} else {
			fromtray = False;
		}
		num = registerexternalwindow_cuturltext(bchanl->registerexternalwindow, NULL, 0, False);
		if (num > 0) {
			totray = True;
		} else {
			totray = False;
		}
		break;
	}

	bchanl_mainmenu_setup(&bchanl->mainmenu, isactive, isopen, isopen_extbbs, selected, fromtray, totray, bchanl->subjectdisplay.resnum, bchanl->subjectdisplay.since, bchanl->subjectdisplay.vigor);
}

LOCAL VOID bchanl_selectmenu(bchanl_t *bchanl, W sel, BCHANL_TEXTBOX_MENU_TYPE type)
{
	Bool isopen;
	RECT work;
#define BCHANL_SELECTMENU_STRBUF_LENGTH 256
	TC str[BCHANL_SELECTMENU_STRBUF_LENGTH];
	W index, len = 0, l, t, r, b;
	GID gid;

	switch(sel) {
	case BCHANL_MAINMENU_SELECT_CLOSE: /* [終了] */
		bchanl_killme(bchanl);
		break;
	case BCHANL_MAINMENU_SELECT_REDISPLAY: /* [再表示] */
		subjectwindow_requestredisp(bchanl->subjectwindow);
		bbsmenuwindow_requestredisp(bchanl->bbsmenuwindow);
		break;
	case BCHANL_MAINMENU_SELECT_BBSMENUFETCH: /* [板一覧再取得] */
		bchanl_networkrequest_bbsmenu(bchanl);
		break;
	case BCHANL_MAINMENU_SELECT_SUBJECTOPTION: /* [スレ一覧設定] */
		isopen = subjectoptionwindow_isopen(bchanl->subjectoptionwindow);
		if (isopen == False) {
			subjectoptionwindow_open(bchanl->subjectoptionwindow);
		} else {
			subjectoptionwindow_close(bchanl->subjectoptionwindow);
		}
		break;
	case BCHANL_MAINMENU_SELECT_EXTBBS_MANAGER: /* [外部板の追加] */
		isopen = externalbbswindow_isopen(bchanl->externalbbswindow);
		if (isopen == False) {

			bchanl->bbsmenu.editctx = extbbslist_startedit(bchanl->bbsmenu.extbbslist);
			if (bchanl->bbsmenu.editctx == NULL) {
				break;
			}
			externalbbswindow_open(bchanl->externalbbswindow);
			externalbbswindow_getworkrect(bchanl->externalbbswindow, &work);
			extbbslist_editcontext_setviewrect(bchanl->bbsmenu.editctx, 0, 0, work.c.right - work.c.left, work.c.bottom - work.c.top);
			externalbbswindow_setworkrect(bchanl->externalbbswindow, 0, 0, work.c.right - work.c.left, work.c.bottom - work.c.top);
			gid = externalbbswindow_getGID(bchanl->externalbbswindow);
			extbbslist_editcontext_getdrawrect(bchanl->bbsmenu.editctx, gid, &l, &t, &r, &b);
			externalbbswindow_setdrawrect(bchanl->externalbbswindow, l, t, r, b);
		}
		break;
	case BCHANL_MAINMENU_SELECT_EXTBBS_REGISTER:
		isopen = registerexternalwindow_isopen(bchanl->registerexternalwindow);
		if (isopen == False) {
			registerexternalwindow_open(bchanl->registerexternalwindow);
		}
		break;
	case BCHANL_MAINMENU_SELECT_EXTBBS_UP:
		isopen = externalbbswindow_isopen(bchanl->externalbbswindow);
		if (isopen != False) {
			index = extbbslist_editcontext_getselect(bchanl->bbsmenu.editctx);
			if (index < 0) {
				break;
			}
			extbbslist_editcontext_swapitem(bchanl->bbsmenu.editctx, index-1, index);
			externalbbswindow_requestredisp(bchanl->externalbbswindow);
		}
		break;
	case BCHANL_MAINMENU_SELECT_EXTBBS_DOWN:
		isopen = externalbbswindow_isopen(bchanl->externalbbswindow);
		if (isopen != False) {
			index = extbbslist_editcontext_getselect(bchanl->bbsmenu.editctx);
			if (index < 0) {
				break;
			}
			extbbslist_editcontext_swapitem(bchanl->bbsmenu.editctx, index, index+1);
			externalbbswindow_requestredisp(bchanl->externalbbswindow);
		}
		break;
	case BCHANL_MAINMENU_SELECT_EXTBBS_DELETE:
		isopen = externalbbswindow_isopen(bchanl->externalbbswindow);
		if (isopen != False) {
			index = extbbslist_editcontext_getselect(bchanl->bbsmenu.editctx);
			if (index < 0) {
				break;
			}
			extbbslist_editcontext_deleteitem(bchanl->bbsmenu.editctx, index);
			gid = externalbbswindow_getGID(bchanl->externalbbswindow);
			extbbslist_editcontext_getdrawrect(bchanl->bbsmenu.editctx, gid, &l, &t, &r, &b);
			externalbbswindow_setdrawrect(bchanl->externalbbswindow, l, t, r, b);
			externalbbswindow_requestredisp(bchanl->externalbbswindow);
		}
		break;
	case BCHANL_MAINMENU_SELECT_EDIT_COPY_TO_TRAY:
		switch (type) {
		case BCHANL_TEXTBOX_MENU_TYPE_FILTER:
			len = subjectoptionwindow_cutfiltertext(bchanl->subjectoptionwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, False);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE:
			len = registerexternalwindow_cutboradnametext(bchanl->registerexternalwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, False);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL:
			len = registerexternalwindow_cuturltext(bchanl->registerexternalwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, False);
			break;
		default:
			break;
		}
		if (len > 0) {
			tray_pushstring(str, len);
		}
		break;
	case BCHANL_MAINMENU_SELECT_EDIT_COPY_FROM_TRAY:
		len = tray_popstring(str, BCHANL_SELECTMENU_STRBUF_LENGTH);
		switch (type) {
		case BCHANL_TEXTBOX_MENU_TYPE_FILTER:
			subjectoptionwindow_insertfiltertext(bchanl->subjectoptionwindow, str, len);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE:
			registerexternalwindow_insertboradnametext(bchanl->registerexternalwindow, str, len);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL:
			registerexternalwindow_inserturltext(bchanl->registerexternalwindow, str, len);
			break;
		default:
			break;
		}
		break;
	case BCHANL_MAINMENU_SELECT_EDIT_MOVE_TO_TRAY:
		switch (type) {
		case BCHANL_TEXTBOX_MENU_TYPE_FILTER:
			len = subjectoptionwindow_cutfiltertext(bchanl->subjectoptionwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, True);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE:
			len = registerexternalwindow_cutboradnametext(bchanl->registerexternalwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, True);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL:
			len = registerexternalwindow_cuturltext(bchanl->registerexternalwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, True);
			break;
		default:
			break;
		}
		if (len > 0) {
			tray_pushstring(str, len);
		}
		break;
	case BCHANL_MAINMENU_SELECT_EDIT_MOVE_FROM_TRAY:
		len = tray_popstring(str, BCHANL_SELECTMENU_STRBUF_LENGTH);
		switch (type) {
		case BCHANL_TEXTBOX_MENU_TYPE_FILTER:
			subjectoptionwindow_insertfiltertext(bchanl->subjectoptionwindow, str, len);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE:
			registerexternalwindow_insertboradnametext(bchanl->registerexternalwindow, str, len);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL:
			registerexternalwindow_inserturltext(bchanl->registerexternalwindow, str, len);
			break;
		default:
			break;
		}
		tray_deletedata();
		break;
	case BCHANL_MAINMENU_SELECT_EDIT_DELETE:
		switch (type) {
		case BCHANL_TEXTBOX_MENU_TYPE_FILTER:
			subjectoptionwindow_cutfiltertext(bchanl->subjectoptionwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, True);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE:
			registerexternalwindow_cutboradnametext(bchanl->registerexternalwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, True);
			break;
		case BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL:
			registerexternalwindow_cuturltext(bchanl->registerexternalwindow, str, BCHANL_SELECTMENU_STRBUF_LENGTH, True);
			break;
		default:
			break;
		}
		break;
	case BCHANL_MAINMENU_SELECT_DISPLAY_RESNUMBER:
		bchanl_swapresnumberdisplay(bchanl);
		if (bchanl->currentsubject != NULL) {
			bchanl_subject_setresnumberdisplay(bchanl->currentsubject, bchanl->subjectdisplay.resnum);
			bchanl_changedisplayattribute(bchanl);
		}
		break;
	case BCHANL_MAINMENU_SELECT_DISPLAY_SINCE:
		bchanl_swapsincedisplay(bchanl);
		if (bchanl->currentsubject != NULL) {
			bchanl_subject_setsincedisplay(bchanl->currentsubject, bchanl->subjectdisplay.since);
			bchanl_changedisplayattribute(bchanl);
		}
		break;
	case BCHANL_MAINMENU_SELECT_DISPLAY_VIGOR:
		bchanl_swapvigordisplay(bchanl);
		if (bchanl->currentsubject != NULL) {
			bchanl_subject_setvigordisplay(bchanl->currentsubject, bchanl->subjectdisplay.vigor);
			bchanl_changedisplayattribute(bchanl);
		}
		break;
	}
	return;
}

LOCAL VOID bchanl_popupmenu(bchanl_t *bchanl, PNT pos, BCHANL_TEXTBOX_MENU_TYPE type)
{
	W sel;
	bchanl_setupmenu(bchanl, type);
	gset_ptr(PS_SELECT, NULL, -1, -1);
	sel = bchanl_mainmenu_popup(&bchanl->mainmenu, pos);
	if (sel > 0) {
		bchanl_selectmenu(bchanl, sel, type);
	}
}

LOCAL W bchanl_keyselect(bchanl_t *bchanl, TC keycode, BCHANL_TEXTBOX_MENU_TYPE type)
{
	W sel;
	bchanl_setupmenu(bchanl, type);
	sel = bchanl_mainmenu_keyselect(&bchanl->mainmenu, keycode);
	if (sel > 0) {
		bchanl_selectmenu(bchanl, sel, type);
	}
	return 0;
}

LOCAL VOID bchanl_handletimeout(bchanl_t *bchanl, W code)
{
	switch (code) {
	case BCHANL_MESSAGE_RETRIEVER_RELAYOUT:
		bchanl_bbsmenu_relayout(&bchanl->bbsmenu, bchanl->bbsmenuwindow);
		bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);
		pdsp_msg(NULL);
		break;
	case BCHANL_MESSAGE_RETRIEVER_ERROR:
		bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);
		pdsp_msg(NULL);
		break;
	case BCHANL_MESSAGE_HTTP_EVENT:
		bchanl_handle_httpevent(bchanl);
		break;
	}
}

LOCAL VOID bchanl_eventdispatch(bchanl_t *bchanl)
{
	bchanlhmievent_t *evt;
	W sel, err;
	Bool close;

	err = bchanlhmi_getevent(bchanl->hmi, &evt);
	if (err < 0) {
		return;
	}

	switch (evt->type) {
	case BCHANLHMIEVENT_TYPE_COMMON_MOUSEMOVE:
		break;
	case BCHANLHMIEVENT_TYPE_COMMON_KEYDOWN:
		if (evt->data.common_keydown.stat & ES_CMD) {	/*命令キー*/
			bchanl_setupmenu(bchanl, BCHANL_TEXTBOX_MENU_TYPE_NONE);
			sel = bchanl_keyselect(bchanl, evt->data.common_keydown.keycode, BCHANL_TEXTBOX_MENU_TYPE_NONE);
			if (sel > 0) {
				bchanl_selectmenu(bchanl, sel, BCHANL_TEXTBOX_MENU_TYPE_NONE);
				break;
			}
		}
		bchanl_keydwn(bchanl, evt->data.common_keydown.keytop, evt->data.common_keydown.keycode, evt->data.common_keydown.stat);
		break;
	case BCHANLHMIEVENT_TYPE_COMMON_MENU:
		bchanl_popupmenu(bchanl, evt->data.common_menu.pos, BCHANL_TEXTBOX_MENU_TYPE_NONE);
		break;
	case BCHANLHMIEVENT_TYPE_COMMON_TIMEOUT:
		bchanl_handletimeout(bchanl, evt->data.common_timeout.code);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTWINDOW_DRAW:
		bchanl_subjectwindow_draw(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTWINDOW_RESIZE:
		bchanl_subjectwindow_resize(bchanl, evt->data.subjectwindow_resize.work_sz);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTWINDOW_CLOSE:
		bchanl_subjectwindow_close(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTWINDOW_BUTDN:
		bchanl_subjectwindow_butdn(bchanl, evt->data.subjectwindow_butdn.type, evt->data.subjectwindow_butdn.pos);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTWINDOW_PASTE:
		subjectwindow_responsepasterequest(bchanl->subjectwindow, /* NACK */ 1, NULL);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTWINDOW_MOUSEMOVE:
		gset_ptr(bchanl->hmistate.ptr, NULL, -1, -1);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTWINDOW_SCROLL:
		bchanl_subjectwindow_scroll(bchanl, evt->data.subjectwindow_scroll.dh, evt->data.subjectwindow_scroll.dv);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENUWINDOW_DRAW:
		bchanl_bbsmenuwindow_draw(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENUWINDOW_RESIZE:
		bchanl_bbsmenuwindow_resize(bchanl, evt->data.bbsmenuwindow_resize.work_sz);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENUWINDOW_CLOSE:
		bchanl_bbsmenuwindow_close(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENUWINDOW_BUTDN:
		bchanl_bbsmenuwindow_butdn(bchanl, evt->data.bbsmenuwindow_butdn.type, evt->data.bbsmenuwindow_butdn.pos);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENUWINDOW_MOUSEMOVE:
		gset_ptr(bchanl->hmistate.ptr, NULL, -1, -1);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENUWINDOW_SCROLL:
		bchanl_bbsmenuwindow_scroll(bchanl, evt->data.bbsmenuwindow_scroll.dh, evt->data.bbsmenuwindow_scroll.dv);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTOPTIONWINDOW_PARTS_FILTER_DETERMINE:
		bchanl_changesubjectfilterword(bchanl, evt->data.subjectoptionwindow_filter_determine.value, evt->data.subjectoptionwindow_filter_determine.len);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTOPTIONWINDOW_PARTS_FILTER_COPY:
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTOPTIONWINDOW_PARTS_FILTER_MOVE:
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTOPTIONWINDOW_PARTS_FILTER_MENU:
		bchanl_popupmenu(bchanl, evt->data.subjectoptionwindow_filter_menu.pos, BCHANL_TEXTBOX_MENU_TYPE_FILTER);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTOPTIONWINDOW_PARTS_FILTER_KEYMENU:
		bchanl_keyselect(bchanl, evt->data.subjectoptionwindow_filter_keymenu.keycode, BCHANL_TEXTBOX_MENU_TYPE_FILTER);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTOPTIONWINDOW_PARTS_ORDER_CHANGE:
		bchanl_changesubjectorder(bchanl, evt->data.subjectoptionwindow_order_change.value);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECTOPTIONWINDOW_PARTS_ORDERBY_CHANGE:
		bchanl_changesubjectorderby(bchanl, evt->data.subjectoptionwindow_order_change.value);
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_BORADNAME_DETERMINE:
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_BORADNAME_COPY:
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_BORADNAME_MOVE:
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_BORADNAME_MENU:
		bchanl_popupmenu(bchanl, evt->data.registerexternalwindow_boradname_menu.pos, BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE);
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_BORADNAME_KEYMENU:
		bchanl_keyselect(bchanl, evt->data.registerexternalwindow_boradname_keymenu.keycode, BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_TITLE);
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_URL_DETERMINE:
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_URL_COPY:
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_URL_MOVE:
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_URL_MENU:
		bchanl_popupmenu(bchanl, evt->data.registerexternalwindow_url_menu.pos, BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL);
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_URL_KEYMENU:
		bchanl_keyselect(bchanl, evt->data.registerexternalwindow_url_keymenu.keycode, BCHANL_TEXTBOX_MENU_TYPE_EXTBBS_URL);
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_DETERMINE_PUSH:
		close = bchanl_registerexternalbbs(bchanl);
		if (close != False) {
			registerexternalwindow_close(bchanl->registerexternalwindow);
		}
		break;
	case BCHANLHMIEVENT_TYPE_REGISTEREXTERNALWINDOW_PARTS_CANCEL_PUSH:
		registerexternalwindow_close(bchanl->registerexternalwindow);
		break;
	case BCHANLHMIEVENT_TYPE_EXTERNALBBSWINDOW_DRAW:
		bchanl_externalbbswindow_draw(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_EXTERNALBBSWINDOW_RESIZE:
		bchanl_externalbbswindow_resize(bchanl, evt->data.externalbbswindow_resize.work_sz);
		break;
	case BCHANLHMIEVENT_TYPE_EXTERNALBBSWINDOW_CLOSE:
		bchanl_externalbbswindow_close(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_EXTERNALBBSWINDOW_BUTDN:
		bchanl_externalbbswindow_butdn(bchanl, evt->data.externalbbswindow_butdn.type, evt->data.externalbbswindow_butdn.pos);
		break;
	case BCHANLHMIEVENT_TYPE_EXTERNALBBSWINDOW_PASTE:
		bchanl_externalbbswindow_paste(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_EXTERNALBBSWINDOW_SCROLL:
		bchanl_externalbbswindow_scroll(bchanl, evt->data.externalbbswindow_scroll.dh, evt->data.externalbbswindow_scroll.dv);
		break;
	case BCHANLHMIEVENT_TYPE_NONE:
	}
}

LOCAL TC filename_dbg_databox[] = (TC[]){TK_b, TK_c, TK_h, TK_a, TK_n, TK_l, TK_PROD, TK_d, TK_b, TK_x, TNULL};
LOCAL TC filename_dbg_storage[] = (TC[]){TK_c, TK_o, TK_m, TK_m, TK_o, TK_n, TK_s, TK_t, TK_o, TK_r, TK_a, TK_g, TK_e, TK_2, TNULL};
LOCAL TC filename_storage[] = (TC[]){TK_c, TK_o, TK_m, TK_m, TK_o, TK_n, TK_s, TK_t, TK_o, TK_r, TK_a, TK_g, TK_e, TNULL};

LOCAL W main_CLI_args(VID *vid, LINK *storage)
{
	W err;
	LINK dbx;

	*vid = -1;
	err = get_lnk(filename_dbg_databox, &dbx, F_NORM);
	if (err < 0) {
		DP_ER("get_lnk:test databox error", err);
		return err;
	}
	err = dopn_dat(&dbx);
	if (err < 0) {
		DP_ER("dopn_dat error", err);
		return err;
	}
	err = get_lnk(filename_dbg_storage, storage, F_NORM);
	if (err < 0) {
		DP_ER("get_lnk;commonstorage error", err);
		return err;
	}

	return 0;
}

LOCAL W main_EXECREC_args(M_EXECREQ *msg, VID *vid, LINK *storage)
{
	W err;
	LINK lnk;

	err = dopn_dat(&msg->self);
	if (err < 0) {
		DP_ER("dopn_dat", err);
		return err;
	}

	lnk = msg->self;
	err = get_lnk(filename_storage, &lnk, F_BASED);
	if (err < 0) {
		DP_ER("get_lnk;commonstorage error", err);
		return err;
	}
	*storage = lnk;

	*vid = msg->vid;

	return 0;
}

typedef struct _arg {
	W ac;
	TC **argv;
} CLI_arg;

LOCAL    CLI_arg   MESSAGEtoargv(const MESSAGE *src)
{
	W len,i,ac;
	TC *str;
	TC **argv;
	CLI_arg ret;

	len = src->msg_size / sizeof(TC);
	str = (TC*)(src->msg_body.ANYMSG.msg_str);
	ac = 0;
	for(i=0;i<len;i++){
		if(str[i] == TK_KSP){
			str[i] = TNULL;
			continue;
		}
		ac++;
		for(;i<len;i++){
			if(str[i] == TK_KSP){
				i--;
				break;
			}
		}
	}

	argv = (TC**)malloc(sizeof(TC*)*ac);

	ac = 0;
	for(i=0;i<len;i++){
		if(str[i] == TNULL){
			str[i] = TNULL;
			continue;
		}
		argv[ac++] = str+i;
		for(;i<len;i++){
			if(str[i] == TNULL){
				i--;
				break;
			}
		}
	}

	ret.ac = ac;
	ret.argv = argv;

	return ret;
}

EXPORT	W	MAIN(MESSAGE *msg)
{
	W err;
	VID vid = -1;
	LINK storage;
	CLI_arg arg;
	bchanl_t bchanl;

	err = dopn_dat(NULL);
	if (err < 0) {
		DP_ER("dopn_dat error:", err);
		ext_prc(0);
	}

	switch (msg->msg_type) {
	case 0: /* CLI */
		arg = MESSAGEtoargv(msg);
		err = main_CLI_args(&vid, &storage);
		if (err < 0) {
			ext_prc(0);
		}
		break;
	case DISPREQ:
		oend_req(((M_DISPREQ*)msg)->vid, -1);
		ext_prc(0);
		break;
	case PASTEREQ:
		oend_req(((M_PASTEREQ*)msg)->vid, -1);
		ext_prc(0);
		break;
	case EXECREQ:
		if ((((M_EXECREQ*)msg)->mode & 2) == 0) {
			ext_prc(0);
		}
		err = main_EXECREC_args((M_EXECREQ*)msg, &vid, &storage);
		if (err < 0) {
			ext_prc(0);
		}
		break;
	default:
		ext_prc(0);
		break;
	}

	err = bchanl_initialize(&bchanl, vid, msg->msg_type, &storage);
	if (err < 0) {
		DP_ER("bchanl_initialize error", err);
		ext_prc(0);
	}
	err = bchanl_prepare_network(&bchanl);
	if (err < 0) {
		DP_ER("bchanl_prepare_network error", err);
		bchanl_killme(&bchanl);
		return err;
	}

	if (msg->msg_type == 0) {
		bchanl_readbbsmenutestdata(&(bchanl.bbsmenu), bchanl.bbsmenuwindow);
		req_tmg(0, BCHANL_MESSAGE_RETRIEVER_RELAYOUT);
	} else if (msg->msg_type == EXECREQ) {
		bchanl_networkrequest_bbsmenu(&bchanl);
	}

	subjectwindow_requestredisp(bchanl.subjectwindow);
	bbsmenuwindow_requestredisp(bchanl.bbsmenuwindow);

	for (;;) {
		bchanl_eventdispatch(&bchanl);
	}

	return 0;
}
