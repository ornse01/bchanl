/*
 * main.c
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
#include	"subjectlayout.h"
#include    "bbsmenuretriever.h"
#include    "bbsmenucache.h"
#include    "bbsmenuparser.h"
#include	"bbsmenufilter.h"
#include    "bbsmenulayout.h"

#include    "bchanl_subject.h"
#include    "bchanl_hmi.h"
#include    "bchanl_menus.h"

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

#define BCHANL_MENU_WINDOW 3

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
};

struct bchanl_t_ {
	W taskid;
	W mbfid;

	bchanl_mainmenu_t mainmenu;
	VID vid;
	W exectype;

	bchanl_hmistate_t hmistate;

	sbjtretriever_t *retriever;

	bchanl_subjecthash_t *subjecthash;
	bchanl_bbsmenu_t bbsmenu;
	bchanl_subject_t *currentsubject;

	bchanlhmi_t *hmi;
	subjectwindow_t *subjectwindow;
	bbsmenuwindow_t *bbsmenuwindow;

	struct {
		sbjtcache_t *cache;
		sbjtparser_t *parser;
		sbjtlayout_t *layout;
		sbjtdraw_t *draw;
	} testdata;
};
typedef struct bchanl_t_ bchanl_t;

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

LOCAL VOID bchanl_subjectwindow_scroll(VP arg, W dh, W dv)
{
	bchanl_t *bchanl = (bchanl_t*)arg;
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
	sbjtparser_thread_t *thread;
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

	err = sbjtdraw_findthread(draw, evpos, &thread, &vframe);
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
	err = bchanl_subject_createviewervobj(bchanl->currentsubject, thread, fsn, fsn_len, &vrec.vseg, (LINK*)&vrec.vlnk);
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
	subjectwindow_requestredisp(bchanl->subjectwindow);
}

/*
LOCAL VOID bchanl_subjectwindow_setdraw(bchanl_subjectwindow_t *window, sbjtdraw_t *draw)
{
	window->sbjt = NULL;
	window->draw = draw;
	wreq_dsp(window->wid);
}
*/

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

LOCAL VOID bchanl_bbsmenuwindow_scroll(VP arg, W dh, W dv)
{
	bchanl_t *bchanl = (bchanl_t*)arg;
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

LOCAL VOID bchanl_sendsubjectrequest(bchanl_t *bchanl, bchanl_subject_t *subject)
{
	sbjtcache_t *cache;
	sbjtlayout_t *layout;
	sbjtdraw_t *draw;
	TC *title;
	RECT w_work;
	W l, t, r, b, title_len, err;

	bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_BUSY);
	pdsp_msg(bchanl->hmistate.msg_retr_subject);

	cache = bchanl_subject_getcache(subject);
	err = sbjtretriever_sendrequest(bchanl->retriever, cache);
	if (err < 0) {
		pdsp_msg(bchanl->hmistate.msg_error_retr);
		bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);
		return;
	}

	pdsp_msg(NULL);
	bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);

	bchanl_subject_relayout(subject);

	bchanl_setcurrentsubject(bchanl, subject);

	subjectwindow_getworkrect(bchanl->subjectwindow, &w_work);
	draw = bchanl_subject_getdraw(subject);
	sbjtdraw_setviewrect(draw, 0, 0, w_work.c.right, w_work.c.bottom);
	subjectwindow_setworkrect(bchanl->subjectwindow, 0, 0, w_work.c.right, w_work.c.bottom);

	layout = bchanl_subject_getlayout(subject);
	sbjtlayout_getdrawrect(layout, &l, &t, &r, &b);
	subjectwindow_setdrawrect(bchanl->subjectwindow, l, t, r, b);

	bchanl_subject_gettitle(subject, &title, &title_len);
	subjectwindow_settitle(bchanl->subjectwindow, title);
}

LOCAL VOID bchanl_bbsmenuwindow_click(bchanl_t *bchanl, PNT pos)
{
	bbsmnparser_item_t *item;
	bchanl_subject_t *subject;
	W fnd;
	UB *host, *board;
	W host_len, board_len;
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
	bbsmnparser_item_gethostboard(item, &host, &host_len, &board, &board_len);
	subject = bchanl_subjecthash_search(bchanl->subjecthash, host, host_len, board, board_len);
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

LOCAL W bchanl_bbsmenu_initialize(bchanl_bbsmenu_t *bchanl, GID gid, bchanl_subjecthash_t *subjecthash)
{
	bbsmnretriever_t *retriever;
	bbsmncache_t *cache;
	bbsmnparser_t *parser;
	bbsmnfilter_t *filter;
	bbsmnlayout_t *layout;
	bbsmndraw_t *draw;

	cache = bbsmncache_new();
	if (cache == NULL) {
		goto error_cache;
	}
	retriever = bbsmnretriever_new();
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

	bchanl->gid = gid;
	bchanl->retriever = retriever;
	bchanl->cache = cache;
	bchanl->parser = parser;
	bchanl->filter = filter;
	bchanl->layout = layout;
	bchanl->draw = draw;
	bchanl->subjecthash = subjecthash;

	return 0;

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

	bbsmnparser_item_gethostboard(item, &host, &host_len, &board, &board_len);
	err = bchanl_subjecthash_append(bchanl->subjecthash, host, host_len, board, board_len, item->title, item->title_len);
	return err;
}

LOCAL VOID bchanl_bbsmenu_relayout(bchanl_bbsmenu_t *bchanl, bbsmenuwindow_t *window)
{
	W err, l, t, r, b, ret;
	bbsmnparser_t *parser = bchanl->parser;
	bbsmnparser_item_t *item;
	bbsmnfilter_t *filter = bchanl->filter;
	bbsmnlayout_t *layout = bchanl->layout;

	bbsmnlayout_clear(layout);
	bbsmnfilter_clear(filter);
	bbsmnparser_clear(parser);

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

	bbsmnlayout_getdrawrect(bchanl->layout, &l, &t, &r, &b);
	bbsmenuwindow_setdrawrect(window, l, t, r, b);

	bbsmenuwindow_requestredisp(window);
}

#define BCHANL_MESSAGE_RETRIEVER_RELAYOUT 1
#define BCHANL_MESSAGE_RETRIEVER_ERROR -1

LOCAL VOID bchanl_retriever_task(W arg)
{
	bchanl_t *bchanl;
	bbsmnretriever_t *retr;
	bbsmncache_t *cache;
	W msg,err;

	bchanl = (bchanl_t*)arg;
	retr = bchanl->bbsmenu.retriever;
	cache = bchanl->bbsmenu.cache;

	for (;;) {
		DP(("before rcv_mbf %d\n", bchanl->mbfid));
		err = rcv_mbf(bchanl->mbfid, (VP)&msg, T_FOREVER);
		DP_ER("rcv_mbf error:",err);
		if (err != 4) {
			continue;
		}

		err = bbsmnretriever_sendrequest(retr, cache);

		switch (err) {
		case BBSMNRETRIEVER_REQUEST_ALLRELOAD:
			req_tmg(0, BCHANL_MESSAGE_RETRIEVER_RELAYOUT);
			break;
		default:
			req_tmg(0, BCHANL_MESSAGE_RETRIEVER_ERROR);
			DP_ER("bbsmnretreiver_request error",err);
			break;
		}
	}

	ext_tsk();
}

LOCAL W bchanl_prepare_network(bchanl_t *bchanl)
{
	if (bchanl->retriever == NULL) {
		return 0;
	}

	bchanl->mbfid = cre_mbf(sizeof(W), sizeof(W), DELEXIT);
	if (bchanl->mbfid < 0) {
		DP_ER("error cre_mbf:", bchanl->mbfid);
		return -1;
	}
	bchanl->taskid = cre_tsk(bchanl_retriever_task, -1, (W)bchanl);
	if (bchanl->taskid < 0) {
		del_mbf(bchanl->mbfid);
		bchanl->mbfid = -1;
		DP_ER("error cre_tsk:", bchanl->taskid);
		return -1;
	}

	return 0;
}

LOCAL W bchanl_networkrequest_bbsmenu(bchanl_t *bchanl)
{
	W msg = 1, err;
	static UW lastrequest = 0;
	UW etime;

	if (bchanl->mbfid < 0) {
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

	err = snd_mbf(bchanl->mbfid, &msg, sizeof(W), T_FOREVER);
	if (err < 0) {
		DP_ER("snd_mbf error:", err);
		return err;
	}

	bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_BUSY);
	pdsp_msg(bchanl->hmistate.msg_retr_bbsmenu);

	return 0;
}

LOCAL W bchanl_initialize(bchanl_t *bchanl, VID vid, W exectype)
{
	static	RECT	r0 = {{400, 100, 700+7, 200+30}};
	static	RECT	r1 = {{100, 100, 300+7, 300+30}};
	TC *title0 = NULL, *title1 = NULL;
	W err;
	WID wid;
	GID gid;
	RECT w_work;
	sbjtretriever_t *retriever;
	bchanlhmi_t *hmi;
	bchanl_subjecthash_t *subjecthash;
	subjectwindow_t *subjectwindow;
	bbsmenuwindow_t *bbsmenuwindow;

	retriever = sbjtretriever_new();
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
	subjectwindow = bchanlhmi_newsubjectwindow(hmi, &r0, title0, NULL, bchanl_subjectwindow_scroll, bchanl);
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
	dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_WINDOWTITLE_BBSMENU, (void**)&title1);
	bbsmenuwindow = bchanlhmi_newbbsmenuwindow(hmi, &r1, title1, NULL, bchanl_bbsmenuwindow_scroll, bchanl);
	if (bbsmenuwindow == NULL) {
		DP_ER("bchanlhmi_newbbsmenuwindow error", 0);
		goto error_bbsmenuwindow;
	}
	gid = bbsmenuwindow_getGID(bbsmenuwindow);
	err = bchanl_bbsmenu_initialize(&(bchanl->bbsmenu), gid, subjecthash);
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

	bchanl->retriever = retriever;
	bchanl->subjecthash = subjecthash;

	bchanl->testdata.cache = NULL;
	bchanl->testdata.parser = NULL;
	bchanl->testdata.layout = NULL;
	bchanl->testdata.draw = NULL;
	bchanl->currentsubject = NULL;

	bchanl->vid = vid;
	bchanl->exectype = exectype;

	bchanl->hmi = hmi;
	bchanl->subjectwindow = subjectwindow;
	bchanl->bbsmenuwindow = bbsmenuwindow;

	return 0;

error_mainmenu:
	//bchanl_bbsmenu_finalize(&(bchanl->bbsmenu));
error_bbsmenu:
	bchanlhmi_deletebbsmenuwindow(hmi, bbsmenuwindow);
error_bbsmenuwindow:
	bchanl_subjecthash_delete(subjecthash);
error_subjecthash:
	bchanlhmi_deletesubjectwindow(hmi, subjectwindow);
error_subjectwindow:
	bchanlhmi_delete(hmi);
error_bchanlhmi:
	sbjtretriever_delete(retriever);
error_retriever:
	return -1; /* TODO */
}

LOCAL VOID bchanl_killme(bchanl_t *bchanl)
{
	gset_ptr(PS_BUSY, NULL, -1, -1);
	pdsp_msg(NULL);

	if (bchanl->testdata.draw != NULL) {
		sbjtdraw_delete(bchanl->testdata.draw);
	}
	if (bchanl->testdata.layout != NULL) {
		sbjtlayout_delete(bchanl->testdata.layout);
	}
	if (bchanl->testdata.parser != NULL) {
		sbjtparser_delete(bchanl->testdata.parser);
	}
	if (bchanl->testdata.cache != NULL) {
		sbjtcache_delete(bchanl->testdata.cache);
	}

	if (bchanl->exectype == EXECREQ) {
		oend_prc(bchanl->vid, NULL, 0);
	}
	bchanl_mainmenu_finalize(&bchanl->mainmenu);
	bchanlhmi_deletebbsmenuwindow(bchanl->hmi, bchanl->bbsmenuwindow);
	bchanl_subjecthash_delete(bchanl->subjecthash);
	bchanlhmi_deletesubjectwindow(bchanl->hmi, bchanl->subjectwindow);
	bchanlhmi_delete(bchanl->hmi);
	sbjtretriever_delete(bchanl->retriever);

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

	req_tmg(0, BCHANL_MESSAGE_RETRIEVER_RELAYOUT);

	bbsmenuwindow_getworkrect(bchanl_window, &w_work);
	bbsmndraw_setviewrect(draw, 0, 0, w_work.c.right, w_work.c.bottom);
	bbsmenuwindow_setworkrect(bchanl_window, 0, 0, w_work.c.right, w_work.c.bottom);
}

LOCAL W bchanl_readsubjecttestdata(bchanl_t *bchanl, TC *fname)
{
	W fd, len, err, l, t, r, b;
	LINK lnk;
	UB *bin;
	GID gid;
	RECT w_work;
	COLOR color;
	FSSPEC fspec;
	sbjtcache_t *cache = bchanl->testdata.cache;
	sbjtparser_t *parser = bchanl->testdata.parser;
	sbjtparser_thread_t *item;
	sbjtlayout_t *layout = bchanl->testdata.layout;
	sbjtdraw_t *draw = bchanl->testdata.draw;

	cache = sbjtcache_new();
	if (cache == NULL) {
		return -1;
	}
	bchanl->testdata.cache = cache;
	parser = sbjtparser_new(cache);
	if (parser == NULL) {
		return -1;
	}
	bchanl->testdata.parser = parser;
	gid = subjectwindow_getGID(bchanl->subjectwindow);
	layout = sbjtlayout_new(gid);
	if (layout == NULL) {
		return -1;
	}
	bchanl->testdata.layout = layout;
	draw = sbjtdraw_new(layout);
	if (draw == NULL) {
		return -1;
	}
	bchanl->testdata.draw = draw;

	err = wget_inf(WI_FSVOBJ, &fspec, sizeof(FSSPEC));
	if (err >= 0) {
		sbjtlayout_setfsspec(layout, &fspec);
	}
	err = wget_inf(WI_VOBJBGCOL, &color, sizeof(COLOR));
	if (err >= 0) {
		sbjtlayout_setvobjbgcol(layout, color);
	}

	err = get_lnk(fname, &lnk, F_NORM);
	if (err < 0) {
		DP_ER("error get_lnk", err);
		return err;
	}
	fd = opn_fil(&lnk, F_READ, NULL);
	if (fd < 0) {
		return fd;
	}
	err = rea_rec(fd, 0, NULL, 0, &len, NULL);
	if (err < 0) {
		cls_fil(fd);
		return err;
	}
	bin = malloc(len);
	if (bin == NULL) {
		cls_fil(fd);
		return -1;
	}
	err = rea_rec(fd, 0, bin, len, 0, NULL);
	if (err < 0) {
		free(bin);
		cls_fil(fd);
		return err;
	}
	cls_fil(fd);

	sbjtcache_appenddata(cache, bin, len);
	free(bin);

	for (;;) {
		err = sbjtparser_getnextthread(parser, &item);
		if (err != 1) {
			break;
		}
		if (item == NULL) {
			break;
		}
		err = sbjtlayout_appendthread(layout, item);
		if (err < 0) {
			return err;
		}
	}

	//bchanl_subjectwindow_setdraw(&bchanl->subjectwindow, draw);

	subjectwindow_getworkrect(bchanl->subjectwindow, &w_work);
	sbjtdraw_setviewrect(draw, 0, 0, w_work.c.right, w_work.c.bottom);
	subjectwindow_setworkrect(bchanl->subjectwindow, 0, 0, w_work.c.right, w_work.c.bottom);

	sbjtlayout_getdrawrect(layout, &l, &t, &r, &b);
	subjectwindow_setdrawrect(bchanl->subjectwindow, l, t, r, b);

	return 0;
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
		break;
	case KC_PG_U:
		sbjtdraw_getviewrect(draw, &l, &t, &r, &b);
		if (t < (b - t)) {
			scr = -t;
		} else {
			scr = - (b - t);
		}
		subjectwindow_scrollbyvalue(bchanl->subjectwindow, 0, scr);
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

LOCAL VOID bchanl_setupmenu(bchanl_t *bchanl)
{
	bchanl_mainmenu_setup(&bchanl->mainmenu);
}

LOCAL VOID bchanl_selectmenu(bchanl_t *bchanl, W sel)
{
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
	}
	return;
}

LOCAL VOID bchanl_popupmenu(bchanl_t *bchanl, PNT pos)
{
	W sel;
	bchanl_setupmenu(bchanl);
	gset_ptr(PS_SELECT, NULL, -1, -1);
	sel = bchanl_mainmenu_popup(&bchanl->mainmenu, pos);
	if (sel > 0) {
		bchanl_selectmenu(bchanl, sel);
	}
}

LOCAL W bchanl_keyselect(bchanl_t *bchanl, TC keycode)
{
	W sel;
	bchanl_setupmenu(bchanl);
	sel = bchanl_mainmenu_keyselect(&bchanl->mainmenu, keycode);
	if (sel > 0) {
		bchanl_selectmenu(bchanl, sel);
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
	}
}

LOCAL VOID bchanl_eventdispatch(bchanl_t *bchanl)
{
	bchanlhmievent_t *evt;
	W sel, err;

	err = bchanlhmi_getevent(bchanl->hmi, &evt);
	if (err < 0) {
		return;
	}

	switch (evt->type) {
	case BCHANLHMIEVENT_TYPE_COMMON_MOUSEMOVE:
		break;
	case BCHANLHMIEVENT_TYPE_COMMON_KEYDOWN:
		if (evt->data.common_keydown.stat & ES_CMD) {	/*命令キー*/
			bchanl_setupmenu(bchanl);
			sel = bchanl_keyselect(bchanl, evt->data.common_keydown.keycode);
			if (sel > 0) {
				bchanl_selectmenu(bchanl, sel);
				break;
			}
		}
		bchanl_keydwn(bchanl, evt->data.common_keydown.keytop, evt->data.common_keydown.keycode, evt->data.common_keydown.stat);
		break;
	case BCHANLHMIEVENT_TYPE_COMMON_MENU:
		bchanl_popupmenu(bchanl, evt->data.common_menu.pos);
		break;
	case BCHANLHMIEVENT_TYPE_COMMON_TIMEOUT:
		bchanl_handletimeout(bchanl, evt->data.common_timeout.code);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECT_DRAW:
		bchanl_subjectwindow_draw(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECT_RESIZE:
		bchanl_subjectwindow_resize(bchanl, evt->data.subject_resize.work_sz);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECT_CLOSE:
		bchanl_subjectwindow_close(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECT_BUTDN:
		bchanl_subjectwindow_butdn(bchanl, evt->data.subject_butdn.type, evt->data.subject_butdn.pos);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECT_PASTE:
		subjectwindow_responsepasterequest(bchanl->subjectwindow, /* NACK */ 1, NULL);
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECT_SWITCH:
		if (evt->data.subject_switch.needdraw == True) {
			bchanl_subjectwindow_draw(bchanl);
		}
		break;
	case BCHANLHMIEVENT_TYPE_SUBJECT_MOUSEMOVE:
		gset_ptr(bchanl->hmistate.ptr, NULL, -1, -1);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENU_DRAW:
		bchanl_bbsmenuwindow_draw(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENU_RESIZE:
		bchanl_bbsmenuwindow_resize(bchanl, evt->data.bbsmenu_resize.work_sz);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENU_CLOSE:
		bchanl_bbsmenuwindow_close(bchanl);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENU_BUTDN:
		bchanl_bbsmenuwindow_butdn(bchanl, evt->data.bbsmenu_butdn.type, evt->data.bbsmenu_butdn.pos);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENU_PASTE:
		bbsmenuwindow_responsepasterequest(bchanl->bbsmenuwindow, /* NACK */ 1, NULL);
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENU_SWITCH:
		if (evt->data.bbsmenu_switch.needdraw == True) {
			bchanl_bbsmenuwindow_draw(bchanl);
		}
		break;
	case BCHANLHMIEVENT_TYPE_BBSMENU_MOUSEMOVE:
		gset_ptr(bchanl->hmistate.ptr, NULL, -1, -1);
		break;
	case BCHANLHMIEVENT_TYPE_NONE:
	}
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
	CLI_arg arg;
	LINK dbx;
	bchanl_t bchanl;

	err = dopn_dat(NULL);
	if (err < 0) {
		DP_ER("dopn_dat error:", err);
		ext_prc(0);
	}

	switch (msg->msg_type) {
	case 0: /* CLI */
		arg = MESSAGEtoargv(msg);
		err = get_lnk((TC[]){TK_b, TK_c, TK_h, TK_a, TK_n, TK_l, TK_PROD, TK_d, TK_b, TK_x,TNULL}, &dbx, F_NORM);
		if (err < 0) {
			DP_ER("get_lnk:databox error", err);
			ext_prc(0);
		}
		err = dopn_dat(&dbx);
		if (err < 0) {
			DP_ER("dopn_dat error", err);
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
		err = dopn_dat(&((M_EXECREQ*)msg)->self);
		if (err < 0) {
			DP_ER("dopn_dat error", err);
			ext_prc(0);
		}
		vid = ((M_EXECREQ*)msg)->vid;
		break;
	default:
		ext_prc(0);
		break;
	}

	err = bchanl_initialize(&bchanl, vid, msg->msg_type);
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
		if (arg.ac > 1) {
			err = bchanl_readsubjecttestdata(&bchanl, arg.argv[1]);
			if (err < 0) {
				DP_ER("bchanl_readsubjecttestdata error\n", err);
				bchanl_killme(&bchanl);
				return err;
			}
		}
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
