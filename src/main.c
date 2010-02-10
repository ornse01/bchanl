/*
 * main.c
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

typedef struct bchanl_t_ bchanl_t;

LOCAL VOID bchanl_killme(bchanl_t *bchanl);

typedef struct bchanl_subjectwindow_t_ bchanl_subjectwindow_t;
struct bchanl_subjectwindow_t_ {
	bchanl_t *owner;

	W wid;
	W gid;
	bchanl_subject_t *sbjt;
	sbjtdraw_t *draw;
	commonwindow_t *window;
};

LOCAL VOID bchanl_subjectwindow_scroll(VP arg, W dh, W dv)
{
	bchanl_subjectwindow_t *bchanl = (bchanl_subjectwindow_t*)arg;
	if (bchanl->draw == NULL) {
		return;
	}
	sbjtdraw_scrollviewrect(bchanl->draw, dh, dv);
	wscr_wnd(bchanl->wid, NULL, -dh, -dv, W_MOVE|W_RDSET);
}

LOCAL VOID bchanl_subjectwindow_draw(VP arg, RECT *r)
{
	bchanl_subjectwindow_t *bchanl = (bchanl_subjectwindow_t*)arg;
	if (bchanl->draw == NULL) {
		return;
	}
	sbjtdraw_draw(bchanl->draw, r);
}

LOCAL VOID bchanl_subjectwindow_resize(VP arg)
{
	bchanl_subjectwindow_t *bchanl = (bchanl_subjectwindow_t*)arg;
	W l,t,r,b;
	RECT work;

	if (bchanl->draw == NULL) {
		return;
	}

	sbjtdraw_getviewrect(bchanl->draw, &l, &t, &r, &b);
	wget_wrk(bchanl->wid, &work);

	r = l + work.c.right - work.c.left;
	b = t + work.c.bottom - work.c.top;

	sbjtdraw_setviewrect(bchanl->draw, l, t, r, b);
	commonwindow_setworkrect(bchanl->window, l, t, r, b);

	wreq_dsp(bchanl->wid);
}

LOCAL VOID bchanl_subjectwindow_close(VP arg)
{
	bchanl_subjectwindow_t *bchanl = (bchanl_subjectwindow_t*)arg;
	bchanl_killme(bchanl->owner);
}

LOCAL VOID bchanl_subjectwindow_click(VP arg, PNT pos)
{
}

LOCAL VOID bchanl_subjectwindow_press(VP arg, WEVENT *wev)
{
	bchanl_subjectwindow_t *bchanl = (bchanl_subjectwindow_t*)arg;
	sbjtparser_thread_t *thread;
	WID wid_butup;
	W event_type, size, err, fsn_len;
	void *fsn;
	GID gid;
	PNT pos, p1;
	TR_VOBJREC vrec;
	TRAYREC tr_rec;
	WEVENT paste_ev;
	SEL_RGN	sel;
	RECT r0;

	if (bchanl->draw == NULL) {
		return;
	}
	if (bchanl->sbjt == NULL) {
		return;
	}

	err = sbjtdraw_findthread(bchanl->draw, wev->s.pos, &thread);
	if (err == 0) {
		return;
	}

	gid = wsta_drg(bchanl->wid, 0);
	if (gid < 0) {
		DP_ER("wsta_drg error:", gid);
		return;
	}

	gget_fra(gid, &r0);
	gset_vis(gid, r0);

	p1 = wev->s.pos;
	sel.sts = 0;
	sel.rgn.r.c.left = p1.x - 5;
	sel.rgn.r.c.top = p1.y - 5;
	sel.rgn.r.c.right = sel.rgn.r.c.left + 100;
	sel.rgn.r.c.bottom = sel.rgn.r.c.top + 20;
	adsp_sel(gid, &sel, 1);

	gset_ptr(PS_GRIP, NULL, -1, -1);
	for (;;) {
		event_type = wget_drg(&pos, wev);
		if (event_type == EV_BUTUP) {
			wid_butup = wev->s.wid;
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
	wend_drg();

	if (wid_butup == bchanl->wid) {
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
	err = bchanl_subject_createviewervobj(bchanl->sbjt, thread, fsn, fsn_len, &vrec.vseg, (LINK*)&vrec.vlnk);
	if (err < 0) {
		DP_ER("bchanl_subject_createviewervobj error", err);
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
	paste_ev.r.r.p.rightbot = wev->s.pos;
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

LOCAL W bchanl_subjectwindow_initialize(bchanl_t *owner, bchanl_subjectwindow_t *window, RECT *r, TC *tit)
{
	WID wid;
	commonwindow_t *cwindow;

	wid = wopn_wnd(WA_SIZE|WA_HHDL|WA_VHDL|WA_BBAR|WA_RBAR, 0, r, NULL, 1, tit, NULL, NULL);
	if (wid < 0) {
		return wid;
	}
	cwindow = commonwindow_new(wid, window);
	if (window == NULL) {
		return -1; /* TODO */
	}
	commonwindow_setscrollcallback(cwindow, bchanl_subjectwindow_scroll);
	commonwindow_setdrawcallback(cwindow, bchanl_subjectwindow_draw);
	commonwindow_setresizecallback(cwindow, bchanl_subjectwindow_resize);
	commonwindow_setclosecallback(cwindow, bchanl_subjectwindow_close);
	commonwindow_setclickcallback(cwindow, bchanl_subjectwindow_click);
	commonwindow_setdclickcallback(cwindow, bchanl_subjectwindow_click);
	commonwindow_setpresscallback(cwindow, bchanl_subjectwindow_press);
	commonwindow_setqpresscallback(cwindow, bchanl_subjectwindow_press);

	window->owner = owner;
	window->wid = wid;
	window->gid = wget_gid(wid);
	window->window = cwindow;
	window->draw = NULL;

	return 0;
}

LOCAL VOID bchanl_subjectwindow_finalize(bchanl_subjectwindow_t *window)
{
	commonwindow_delete(window->window);
	wcls_wnd(window->wid, CLR);
}

LOCAL VOID bchanl_subjectwindow_setsubject(bchanl_subjectwindow_t *window, bchanl_subject_t *sbjt)
{
	window->sbjt = sbjt;
	window->draw = bchanl_subject_getdraw(sbjt);
	wreq_dsp(window->wid);
}

LOCAL VOID bchanl_subjectwindow_setdraw(bchanl_subjectwindow_t *window, sbjtdraw_t *draw)
{
	window->sbjt = NULL;
	window->draw = draw;
	wreq_dsp(window->wid);
}

typedef struct bchanl_bbsmenuwindow_t_ bchanl_bbsmenuwindow_t;
struct bchanl_bbsmenuwindow_t_ {
	bchanl_t *owner;

	W wid;
	W gid;
	bbsmndraw_t *draw;
	bchanl_subjecthash_t *subjecthash;
	commonwindow_t *window;
};

LOCAL VOID bchanl_bbsmenuwindow_scroll(VP arg, W dh, W dv)
{
	bchanl_bbsmenuwindow_t *bchanl = (bchanl_bbsmenuwindow_t*)arg;
	bbsmndraw_scrollviewrect(bchanl->draw, dh, dv);
	wscr_wnd(bchanl->wid, NULL, -dh, -dv, W_MOVE|W_RDSET);
}

LOCAL VOID bchanl_bbsmenuwindow_draw(VP arg, RECT *r)
{
	bchanl_bbsmenuwindow_t *bchanl = (bchanl_bbsmenuwindow_t*)arg;
	bbsmndraw_draw(bchanl->draw, r);
}

LOCAL VOID bchanl_bbsmenuwindow_resize(VP arg)
{
	bchanl_bbsmenuwindow_t *bchanl = (bchanl_bbsmenuwindow_t*)arg;
	W l,t,r,b;
	RECT work;

	bbsmndraw_getviewrect(bchanl->draw, &l, &t, &r, &b);
	wget_wrk(bchanl->wid, &work);

	r = l + work.c.right - work.c.left;
	b = t + work.c.bottom - work.c.top;

	bbsmndraw_setviewrect(bchanl->draw, l, t, r, b);
	commonwindow_setworkrect(bchanl->window, l, t, r, b);

	wreq_dsp(bchanl->wid);
}

LOCAL VOID bchanl_bbsmenuwindow_close(VP arg)
{
	bchanl_bbsmenuwindow_t *bchanl = (bchanl_bbsmenuwindow_t*)arg;
	bchanl_killme(bchanl->owner);
}

LOCAL VOID bchanl_sendsubjectrequest(bchanl_t *bchanl, bchanl_subject_t *subject);

LOCAL VOID bchanl_bbsmenuwindow_click(VP arg, PNT pos)
{
	bchanl_bbsmenuwindow_t *bchanl = (bchanl_bbsmenuwindow_t*)arg;
	bbsmnparser_item_t *item;
	bchanl_subject_t *subject;
	W fnd;
	UB *host, *board;
	W host_len, board_len;
	TC *title;
	W title_len;

	fnd = bbsmndraw_findboard(bchanl->draw, pos, &item);
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

	bchanl_sendsubjectrequest(bchanl->owner, subject);
}

LOCAL VOID bchanl_bbsmenuwindow_press(VP arg, WEVENT *wev)
{
}

LOCAL W bchanl_bbsmenuwindow_initialize(bchanl_t *owner, bchanl_bbsmenuwindow_t *window, RECT *r, TC *tit, bchanl_subjecthash_t *subjecthash)
{
	WID wid;
	commonwindow_t *cwindow;

	wid = wopn_wnd(WA_SIZE|WA_HHDL|WA_VHDL|WA_BBAR|WA_RBAR, 0, r, NULL, 1, tit, NULL, NULL);
	if (wid < 0) {
		return wid;
	}
	cwindow = commonwindow_new(wid, window);
	if (cwindow == NULL) {
		return -1; /* TODO */
	}
	commonwindow_setscrollcallback(cwindow, bchanl_bbsmenuwindow_scroll);
	commonwindow_setdrawcallback(cwindow, bchanl_bbsmenuwindow_draw);
	commonwindow_setresizecallback(cwindow, bchanl_bbsmenuwindow_resize);
	commonwindow_setclosecallback(cwindow, bchanl_bbsmenuwindow_close);
	commonwindow_setclickcallback(cwindow, bchanl_bbsmenuwindow_click);
	commonwindow_setdclickcallback(cwindow, bchanl_bbsmenuwindow_click);
	commonwindow_setpresscallback(cwindow, bchanl_bbsmenuwindow_press);
	commonwindow_setqpresscallback(cwindow, bchanl_bbsmenuwindow_press);

	window->owner = owner;
	window->wid = wid;
	window->gid = wget_gid(wid);
	window->window = cwindow;
	window->draw = NULL;
	window->subjecthash = subjecthash;

	return 0;
}

LOCAL VOID bchanl_bbsmenuwindow_finalize(bchanl_bbsmenuwindow_t *window)
{
	commonwindow_delete(window->window);
	wcls_wnd(window->wid, CLR);
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

LOCAL VOID bchanl_bbsmenu_relayout(bchanl_bbsmenu_t *bchanl, bchanl_bbsmenuwindow_t *bchanl_window)
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
	commonwindow_setdrawrect(bchanl_window->window, l, t, r, b);

	wreq_dsp(bchanl_window->wid);
}

struct bchanl_t_ {
	W taskid;
	W mbfid;

	MENUITEM *mnitem;
	MNID mnid;
	VID vid;
	W exectype;

	bchanl_hmistate_t hmistate;

	sbjtretriever_t *retriever;

	bchanl_subjecthash_t *subjecthash;
	bchanl_subjectwindow_t subjectwindow;
	bchanl_bbsmenuwindow_t bbsmenuwindow;
	bchanl_bbsmenu_t bbsmenu;

	struct {
		sbjtcache_t *cache;
		sbjtparser_t *parser;
		sbjtlayout_t *layout;
		sbjtdraw_t *draw;
	} testdata;
};

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

static	WEVENT	wev0;

LOCAL W bchanl_initialize(bchanl_t *bchanl, VID vid, W exectype)
{
	static	RECT	r0 = {{400, 100, 700+7, 200+30}};
	static	RECT	r1 = {{100, 100, 300+7, 300+30}};
	TC *title0 = NULL, *title1 = NULL;
	W len, err;
	GID gid;
	MENUITEM *mnitem_dbx, *mnitem;
	MNID mnid;
	RECT w_work;
	sbjtretriever_t *retriever;
	bchanl_subjecthash_t *subjecthash;

	retriever = sbjtretriever_new();
	if (retriever == NULL) {
		DP_ER("sbjtretriever_new error", 0);
		goto error_retriever;
	}
	dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_WINDOWTITLE_SUBJECT, (void**)&title0);
	err = bchanl_subjectwindow_initialize(bchanl, &(bchanl->subjectwindow), &r0, title0);
	if (err < 0) {
		DP_ER("bchanl_subjectwindow_initialize error", err);
		goto error_subjectwindow;
	}
	gid = wget_gid(bchanl->subjectwindow.wid);
	subjecthash = bchanl_subjecthash_new(gid, 100);
	if (subjecthash == NULL) {
		DP_ER("bchanl_subjecthash_new error", 0);
		goto error_subjecthash;
	}
	dget_dtp(TEXT_DATA, BCHANL_DBX_TEXT_WINDOWTITLE_BBSMENU, (void**)&title1);
	err = bchanl_bbsmenuwindow_initialize(bchanl, &(bchanl->bbsmenuwindow), &r1, title1, subjecthash);
	if (err < 0) {
		DP_ER("bchanl_bbsmenuwindow_initialize error", err);
		goto error_bbsmenuwindow;
	}
	err = bchanl_bbsmenu_initialize(&(bchanl->bbsmenu), bchanl->bbsmenuwindow.gid, subjecthash);
	if (err < 0) {
		DP_ER("bchanl_bbsmenu_initialize error", err);
		goto error_bbsmenu;
	}
	err = dget_dtp(8, BCHANL_DBX_MENU_TEST, (void**)&mnitem_dbx);
	if (err < 0) {
		DP_ER("dget_dtp error %d", err);
		goto error_dget_dtp;
	}
	len = dget_siz((B*)mnitem_dbx);
	mnitem = malloc(len);
	if (mnitem == NULL) {
		DP_ER("malloc error", 0);
		goto error_mnitem;
	}
	memcpy(mnitem, mnitem_dbx, len);
	mnid = mcre_men(BCHANL_MENU_WINDOW+2, mnitem, NULL);
	if (mnid < 0) {
		DP_ER("mcre_men error", mnid);
		goto error_mcre_men;
	}

	bchanl_hmistate_initialize(&bchanl->hmistate);

	if (exectype == EXECREQ) {
		osta_prc(vid, bchanl->bbsmenuwindow.wid);
	}

	wget_wrk(bchanl->bbsmenuwindow.wid, &w_work);
	bbsmndraw_setviewrect(bchanl->bbsmenu.draw, 0, 0, w_work.c.right, w_work.c.bottom);
	commonwindow_setworkrect(bchanl->bbsmenuwindow.window, 0, 0, w_work.c.right, w_work.c.bottom);

	bchanl->bbsmenuwindow.draw = bchanl->bbsmenu.draw;

	bchanl->retriever = retriever;
	bchanl->subjecthash = subjecthash;

	bchanl->testdata.cache = NULL;
	bchanl->testdata.parser = NULL;
	bchanl->testdata.layout = NULL;
	bchanl->testdata.draw = NULL;

	bchanl->mnitem = mnitem;
	bchanl->mnid = mnid;
	bchanl->vid = vid;
	bchanl->exectype = exectype;

	return 0;

error_mcre_men:
	free(mnitem);
error_mnitem:
error_dget_dtp:
error_bbsmenu:
	bchanl_bbsmenuwindow_finalize(&(bchanl->bbsmenuwindow));
error_bbsmenuwindow:
	bchanl_subjecthash_delete(subjecthash);
error_subjecthash:
	bchanl_subjectwindow_finalize(&(bchanl->subjectwindow));
error_subjectwindow:
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
	mdel_men(bchanl->mnid);
	free(bchanl->mnitem);
	bchanl_bbsmenuwindow_finalize(&(bchanl->bbsmenuwindow));
	bchanl_subjecthash_delete(bchanl->subjecthash);
	bchanl_subjectwindow_finalize(&(bchanl->subjectwindow));
	sbjtretriever_delete(bchanl->retriever);

	ext_prc(0);
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

	bchanl_subjectwindow_setsubject(&bchanl->subjectwindow, subject);

	wget_wrk(bchanl->subjectwindow.wid, &w_work);
	draw = bchanl_subject_getdraw(subject);
	sbjtdraw_setviewrect(draw, 0, 0, w_work.c.right, w_work.c.bottom);
	commonwindow_setworkrect(bchanl->subjectwindow.window, 0, 0, w_work.c.right, w_work.c.bottom);

	layout = bchanl_subject_getlayout(subject);
	sbjtlayout_getdrawrect(layout, &l, &t, &r, &b);
	commonwindow_setdrawrect(bchanl->subjectwindow.window, l, t, r, b);

	bchanl_subject_gettitle(subject, &title, &title_len);
	wset_tit(bchanl->subjectwindow.wid, -1, title, 0);
}


LOCAL VOID bchanl_readbbsmenutestdata(bchanl_bbsmenu_t *bchanl, bchanl_bbsmenuwindow_t *bchanl_window)
{
	TC fname[] = {TK_b, TK_b, TK_s, TK_m, TK_e, TK_n, TK_u, TK_PROD, TK_h, TK_t, TK_m, TK_l, TNULL};
	LINK lnk;
	W fd, len, err;
	UB *bin;
	RECT w_work;
	bbsmncache_t *cache = bchanl->cache;

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

	wget_wrk(bchanl_window->wid, &w_work);
	bbsmndraw_setviewrect(bchanl_window->draw, 0, 0, w_work.c.right, w_work.c.bottom);
	commonwindow_setworkrect(bchanl_window->window, 0, 0, w_work.c.right, w_work.c.bottom);
}

LOCAL W bchanl_readsubjecttestdata(bchanl_t *bchanl, TC *fname)
{
	W fd, len, err, l, t, r, b;
	LINK lnk;
	UB *bin;
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
	layout = sbjtlayout_new(bchanl->subjectwindow.gid);
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

	bchanl_subjectwindow_setdraw(&bchanl->subjectwindow, draw);

	wget_wrk(bchanl->subjectwindow.wid, &w_work);
	sbjtdraw_setviewrect(draw, 0, 0, w_work.c.right, w_work.c.bottom);
	commonwindow_setworkrect(bchanl->subjectwindow.window, 0, 0, w_work.c.right, w_work.c.bottom);

	sbjtlayout_getdrawrect(layout, &l, &t, &r, &b);
	commonwindow_setdrawrect(bchanl->subjectwindow.window, l, t, r, b);

	return 0;
}

LOCAL VOID bchanl_subjectwindow_keydwn(bchanl_subjectwindow_t *window, UH keycode, TC ch)
{
	W l,t,r,b,l1,t1,r1,b1,scr;
	sbjtlayout_t *layout;

	if (window->draw == NULL) {
		return;
	}

	switch (ch) {
	case KC_CC_U:
		sbjtdraw_getviewrect(window->draw, &l, &t, &r, &b);
		if (t < 16) {
			scr = -t;
		} else {
			scr = -16;
		}
		commonwindow_scrollbyvalue(window->window, 0, scr);
		break;
	case KC_CC_D:
		sbjtdraw_getviewrect(window->draw, &l, &t, &r, &b);
		layout = bchanl_subject_getlayout(window->sbjt);
		sbjtlayout_getdrawrect(layout, &l1, &t1, &r1, &b1);
		if (b + 16 > b1) {
			scr = b1 - b;
		} else {
			scr = 16;
		}
		commonwindow_scrollbyvalue(window->window, 0, scr);
		break;
	case KC_CC_R:
		sbjtdraw_getviewrect(window->draw, &l, &t, &r, &b);
		layout = bchanl_subject_getlayout(window->sbjt);
		sbjtlayout_getdrawrect(layout, &l1, &t1, &r1, &b1);
		if (r + 16 > r1) {
			scr = r1 - r;
		} else {
			scr = 16;
		}
		commonwindow_scrollbyvalue(window->window, scr, 0);
		break;
	case KC_CC_L:
		sbjtdraw_getviewrect(window->draw, &l, &t, &r, &b);
		if (l < 16) {
			scr = -l;
		} else {
			scr = -16;
		}
		commonwindow_scrollbyvalue(window->window, scr, 0);
		break;
	case KC_PG_U:
	case KC_PG_D:
	case KC_PG_R:
	case KC_PG_L:
		/* area scroll */
		break;
	}
}

LOCAL VOID bchanl_bbsmenuwindow_keydwn(bchanl_bbsmenuwindow_t *window, UH keycode, TC ch)
{
	W l,t,r,b,l1,t1,r1,b1,scr;

	switch (ch) {
	case KC_CC_U:
		bbsmndraw_getviewrect(window->draw, &l, &t, &r, &b);
		if (t < 16) {
			scr = -t;
		} else {
			scr = -16;
		}
		commonwindow_scrollbyvalue(window->window, 0, scr);
		break;
	case KC_CC_D:
		bbsmndraw_getviewrect(window->draw, &l, &t, &r, &b);
		bbsmnlayout_getdrawrect(window->owner->bbsmenu.layout, &l1, &t1, &r1, &b1);
		if (b + 16 > b1) {
			scr = b1 - b;
		} else {
			scr = 16;
		}
		commonwindow_scrollbyvalue(window->window, 0, scr);
		break;
	case KC_CC_R:
	case KC_CC_L:
	case KC_PG_U:
	case KC_PG_D:
	case KC_PG_R:
	case KC_PG_L:
		/* area scroll */
		break;
	case KC_PF5:
		bchanl_networkrequest_bbsmenu(window->owner);
		break;
	}
}

LOCAL VOID bchanl_setupmenu(bchanl_t *bchanl)
{
	wget_dmn(&(bchanl->mnitem[BCHANL_MENU_WINDOW].ptr));
	mset_itm(bchanl->mnid, BCHANL_MENU_WINDOW, bchanl->mnitem+BCHANL_MENU_WINDOW);
	oget_men(0, NULL, &(bchanl->mnitem[BCHANL_MENU_WINDOW+1].ptr), NULL, NULL);
	mset_itm(bchanl->mnid, BCHANL_MENU_WINDOW+1, bchanl->mnitem+BCHANL_MENU_WINDOW+1);
}

LOCAL VOID bchanl_selectmenu(bchanl_t *bchanl, W i)
{
	switch(i >> 8) {
	case 0: /* [終了] */
		bchanl_killme(bchanl);
		break;
	case 1: /* [表示] */
		switch(i & 0xff) {
		case 1: /* [再表示] */
			wreq_dsp(bchanl->subjectwindow.wid);
			wreq_dsp(bchanl->bbsmenuwindow.wid);
			break;
		}
		break;
	case 2:	/* [操作] */
		switch(i & 0xff) {
		case 1: /* [板一覧再取得] */
			bchanl_networkrequest_bbsmenu(bchanl);
			break;
		}
		break;
	case BCHANL_MENU_WINDOW: /* [ウィンドウ] */
		wexe_dmn(i);
		break;
	case BCHANL_MENU_WINDOW+1: /* [小物] */
	    oexe_apg(0, i);
		break;
	}
	return;
}

LOCAL VOID bchanl_popupmenu(bchanl_t *bchanl, PNT pos)
{
	W	i;

	bchanl_setupmenu(bchanl);
	gset_ptr(PS_SELECT, NULL, -1, -1);
	i = msel_men(bchanl->mnid, pos);
	if (i > 0) {
		bchanl_selectmenu(bchanl, i);
	}
}

LOCAL VOID receive_message(bchanl_t *bchanl)
{
	MESSAGE msg;
	W code, err;

    err = rcv_msg(MM_ALL, &msg, sizeof(MESSAGE), WAIT|NOCLR);
	if (err >= 0) {
		if (msg.msg_type == MS_TMOUT) { /* should be use other type? */
			code = msg.msg_body.TMOUT.code;
			switch (code) {
			case BCHANL_MESSAGE_RETRIEVER_RELAYOUT:
				bchanl_bbsmenu_relayout(&bchanl->bbsmenu, &bchanl->bbsmenuwindow);
				bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);
				pdsp_msg(NULL);
				break;
			case BCHANL_MESSAGE_RETRIEVER_ERROR:
				bchanl_hmistate_updateptrstyle(&bchanl->hmistate, PS_SELECT);
				pdsp_msg(NULL);
				break;
			}
		}
	}
	clr_msg(MM_ALL, MM_ALL);
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
	W	i, err;
	WID wid, wid_bbsmenu, act;
	VID vid = -1;
	CLI_arg arg;
	LINK dbx;
	bchanl_t bchanl;
	commonwindow_t *window, *window_bbsmenu;

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
	window = bchanl.subjectwindow.window;
	wid = bchanl.subjectwindow.wid;
	window_bbsmenu = bchanl.bbsmenuwindow.window;
	wid_bbsmenu = bchanl.bbsmenuwindow.wid;

	if (msg->msg_type == 0) {
		bchanl_readbbsmenutestdata(&(bchanl.bbsmenu), &(bchanl.bbsmenuwindow));
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

	wreq_dsp(bchanl.subjectwindow.wid);
	wreq_dsp(bchanl.bbsmenuwindow.wid);

	for (;;) {
		wget_evt(&wev0, WAIT);
		switch (wev0.s.type) {
			case	EV_NULL:
				if ((wev0.s.wid != wid)&&(wev0.g.wid != wid_bbsmenu)) {
					gset_ptr(bchanl.hmistate.ptr, NULL, -1, -1);
					break;		/*ウィンドウ外*/
				}
				if (wev0.s.cmd != W_WORK)
					break;		/*作業領域外*/
				if (wev0.s.stat & ES_CMD)
					break;	/*命令キーが押されている*/
				gset_ptr(bchanl.hmistate.ptr, NULL, -1, -1);
				break;
			case	EV_REQUEST:
				if (wev0.g.wid == wid) {
					commonwindow_weventrequest(window, &wev0);
				} else if (wev0.g.wid == wid_bbsmenu) {
					commonwindow_weventrequest(window_bbsmenu, &wev0);
				}
				break;
			case	EV_RSWITCH:
				if (wev0.s.wid == wid) {
					commonwindow_weventreswitch(window, &wev0);
				} else if (wev0.g.wid == wid_bbsmenu) {
					commonwindow_weventreswitch(window_bbsmenu, &wev0);
				}
				break;
			case	EV_SWITCH:
				if (wev0.s.wid == wid) {
					commonwindow_weventswitch(window, &wev0);
				} else if (wev0.g.wid == wid_bbsmenu) {
					commonwindow_weventswitch(window_bbsmenu, &wev0);
				}
				break;
			case	EV_BUTDWN:
				if (wev0.g.wid == wid) {
					commonwindow_weventbutdn(window, &wev0);
				} else if (wev0.g.wid == wid_bbsmenu) {
					commonwindow_weventbutdn(window_bbsmenu, &wev0);
				}
				break;
			case	EV_KEYDWN:
				if (wev0.s.stat & ES_CMD) {
					bchanl_setupmenu(&bchanl);
					i = mfnd_key(bchanl.mnid, wev0.e.data.key.code);
					if (i >= 0) {
						bchanl_selectmenu(&bchanl, i);
						break;
					}
				}
			case	EV_AUTKEY:
				act = wget_act(NULL);
				if (act == wid) {
					bchanl_subjectwindow_keydwn(&bchanl.subjectwindow, wev0.e.data.key.keytop, wev0.e.data.key.code);
				} else if (act == wid_bbsmenu) {
					bchanl_bbsmenuwindow_keydwn(&bchanl.bbsmenuwindow, wev0.e.data.key.keytop, wev0.e.data.key.code);
				}
				break;
			case	EV_INACT:
				pdsp_msg(NULL);
				break;
			case	EV_DEVICE:
				oprc_dev(&wev0.e, NULL, 0);
				break;
			case	EV_MSG:
				receive_message(&bchanl);
				break;
			case	EV_MENU:
				bchanl_popupmenu(&bchanl, wev0.s.pos);
				break;
		}
	}

	return 0;
}
