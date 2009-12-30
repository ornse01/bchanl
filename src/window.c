/*
 * window.c
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

#include    "window.h"

#include	<basic.h>
#include	<bstdio.h>
#include	<bstdlib.h>
#include	<btron/hmi.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%z)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct commonwindow_t_ {
	WID wid;
	GID gid;
	PAID rbar;
	PAID bbar;
	commonwindow_scrollcalback scroll_callback;
	commonwindow_drawcallback draw_callback;
	commonwindow_resizecallback resize_callback;
	commonwindow_closecallback close_callback;
	commonwindow_clickcallback click_callback;
	commonwindow_dclickcallback dclick_callback;
	commonwindow_presscallback press_callback;
	commonwindow_qpresscallback qpress_callback;
	VP arg;
	W draw_l,draw_t,draw_r,draw_b;
	W work_l,work_t,work_r,work_b;
};

LOCAL VOID commonwindow_callback_draw(commonwindow_t *window, RECT *r)
{
	if (window->draw_callback != NULL) {
		(*window->draw_callback)(window->arg, r);
	}
}

LOCAL VOID commonwindow_callback_resize(commonwindow_t *window)
{
	if (window->resize_callback != NULL) {
		(*window->resize_callback)(window->arg);
	}
}

LOCAL VOID commonwindow_callback_scroll(commonwindow_t *window, W dh, W dv)
{
	if (window->scroll_callback != NULL) {
		(*window->scroll_callback)(window->arg, -dh, -dv); /* TODO */
	}
}

LOCAL VOID commonwindow_callback_close(commonwindow_t *window)
{
	if (window->close_callback != NULL) {
		(*window->close_callback)(window->arg);
	}
}

LOCAL VOID commonwindow_callback_click(commonwindow_t *window, PNT pos)
{
	if (window->click_callback != NULL) {
		(*window->click_callback)(window->arg, pos);
	}
}

LOCAL VOID commonwindow_callback_dclick(commonwindow_t *window, PNT pos)
{
	if (window->dclick_callback != NULL) {
		(*window->dclick_callback)(window->arg, pos);
	}
}

LOCAL VOID commonwindow_callback_press(commonwindow_t *window, WEVENT *wev)
{
	if (window->press_callback != NULL) {
		(*window->press_callback)(window->arg, wev);
	}
}

LOCAL VOID commonwindow_callback_qpress(commonwindow_t *window, WEVENT *wev)
{
	if (window->qpress_callback != NULL) {
		(*window->qpress_callback)(window->arg, wev);
	}
}

EXPORT commonwindow_t* commonwindow_new(WID target, VP arg)
{
	commonwindow_t* wscr;

	wscr = malloc(sizeof(commonwindow_t));
	if (wscr == NULL) {
		return NULL;
	}

	wscr->wid = target;
	wscr->gid = wget_gid(wscr->wid);
	wget_bar(target, &(wscr->rbar), &(wscr->bbar), NULL);
	cchg_par(wscr->rbar, P_NORMAL|P_NOFRAME|P_ENABLE|P_ACT|P_DRAGBREAK);
	cchg_par(wscr->bbar, P_NORMAL|P_NOFRAME|P_ENABLE|P_ACT|P_DRAGBREAK);

	wscr->scroll_callback = NULL;
	wscr->draw_callback = NULL;
	wscr->resize_callback = NULL;
	wscr->close_callback = NULL;
	wscr->click_callback = NULL;
	wscr->dclick_callback = NULL;
	wscr->press_callback = NULL;
	wscr->qpress_callback = NULL;
	wscr->arg = arg;

	wscr->work_l = 0;
	wscr->work_t = 0;
	wscr->work_r = 0;
	wscr->work_b = 0;
	wscr->draw_l = 0;
	wscr->draw_t = 0;
	wscr->draw_r = 0;
	wscr->draw_b = 0;

	return wscr;
}

EXPORT VOID commonwindow_delete(commonwindow_t *wscr)
{
	free(wscr);

}

EXPORT VOID commonwindow_setscrollcallback(commonwindow_t *wscr, commonwindow_scrollcalback scrollcallback)
{
	wscr->scroll_callback = scrollcallback;
}

EXPORT VOID commonwindow_setdrawcallback(commonwindow_t *wscr, commonwindow_drawcallback drawcallback)
{
	wscr->draw_callback = drawcallback;
}

EXPORT VOID commonwindow_setresizecallback(commonwindow_t *wscr, commonwindow_resizecallback resizecallback)
{
	wscr->resize_callback = resizecallback;
}

EXPORT VOID commonwindow_setclosecallback(commonwindow_t *wscr, commonwindow_closecallback closecallback)
{
	wscr->close_callback = closecallback;
}

EXPORT VOID commonwindow_setclickcallback(commonwindow_t *wscr, commonwindow_clickcallback clickcallback)
{
	wscr->click_callback = clickcallback;
}

EXPORT VOID commonwindow_setdclickcallback(commonwindow_t *wscr, commonwindow_dclickcallback dclickcallback)
{
	wscr->dclick_callback = dclickcallback;
}

EXPORT VOID commonwindow_setpresscallback(commonwindow_t *wscr, commonwindow_presscallback presscallback)
{
	wscr->press_callback = presscallback;
}

EXPORT VOID commonwindow_setqpresscallback(commonwindow_t *wscr, commonwindow_qpresscallback qpresscallback)
{
	wscr->qpress_callback = qpresscallback;
}

EXPORT W commonwindow_setworkrect(commonwindow_t *wscr, W l, W t, W r, W b)
{
	wscr->work_l = l;
	wscr->work_t = t;
	wscr->work_r = r;
	wscr->work_b = b;
	return commonwindow_updatebar(wscr);
}

LOCAL VOID commonwindow_redisp(commonwindow_t *window)
{
	RECT r;
	do {
		if (wsta_dsp(window->wid, &r, NULL) == 0) {
			break;
		}
		wera_wnd(window->wid, &r);
		commonwindow_callback_draw(window, &r);
	} while (wend_dsp(window->wid) > 0);
}

LOCAL W commonwindow_updaterbar(commonwindow_t *wscr)
{
	W sbarval[4],err;

	sbarval[0] = wscr->work_t;
	sbarval[1] = wscr->work_b;
	sbarval[2] = 0;
	sbarval[3] = wscr->draw_b - wscr->draw_t;
	if (sbarval[1] > sbarval[3]) {
		sbarval[1] = sbarval[3];
	}
	if((err = cset_val(wscr->rbar, 4, sbarval)) < 0){
	    DP(("commonwindow_updaterbar:cset_val rbar\n"));
		DP(("                       :%d %d %d %d\n", sbarval[0], sbarval[1], sbarval[2], sbarval[3]));
		return err;
	}

	return 0;
}

LOCAL W commonwindow_updatebbar(commonwindow_t *wscr)
{
	W sbarval[4],err;

	sbarval[0] = wscr->work_r;
	sbarval[1] = wscr->work_l;
	sbarval[2] = wscr->draw_r - wscr->draw_l;
	sbarval[3] = 0;
	if (sbarval[0] > sbarval[2]) {
		sbarval[0] = sbarval[2];
	}
	if((err = cset_val(wscr->bbar, 4, sbarval)) < 0){
		DP(("commonwindow_updatebbar:cset_val bbar\n"));
		DP(("                       :%d %d %d %d\n", sbarval[0], sbarval[1], sbarval[2], sbarval[3]));
		return err;
    }

	return 0;
}

EXPORT W commonwindow_updatebar(commonwindow_t *wscr)
{
	W err;

	err = commonwindow_updaterbar(wscr);
	if (err < 0) {
		return err;
	}

	err = commonwindow_updatebbar(wscr);
	if (err < 0) {
		return err;
	}

	return 0;
}

LOCAL W commonwindow_scrollbar(commonwindow_t *wscr, WEVENT *wev, PAID scrbarPAID)
{
	W i,err,barval[4];
	W *clo = barval,*chi = barval+1,*lo = barval+2,*hi = barval+3;
	RECT sbarRECT;
	W pressval,smoothscrolldiff;
	W dh,dv;
	WID wid;

	wid = wev->g.wid;

	for(;;) {
		if((i = cact_par(scrbarPAID, wev)) < 0) {
			DP_ER("cact_par error", i);
			return i;
		}
		cget_val(scrbarPAID, 4, barval);
		if((i & 0xc) == 0x8) { /*ジャンプ*/
			if (scrbarPAID == wscr->rbar) { /*右バー*/
				dh = 0;
				dv = -(*clo-wscr->work_t);
				wscr->work_t -= dv;
				wscr->work_b -= dv;
				commonwindow_callback_scroll(wscr, dh, dv);
			} else if (scrbarPAID == wscr->bbar) { /*下バー*/
				dh = -(*chi-wscr->work_l);
				dv = 0;
				wscr->work_l -= dh;
				wscr->work_r -= dh;
				commonwindow_callback_scroll(wscr, dh, dv);
			}
			commonwindow_redisp(wscr);
			if ((i & 0x6000) == 0x6000) {
				/*ジャンプ移動中の値の変更*/
				continue;
			}
			break;
		}
		switch(i) {
			/*スムーススクロール*/
			/* プレス位置とノブの位置に比例した速度でスクロール*/
		case 0x6000:	/*上へのスムーススクロールで中断*/
		case 0x6001:	/*下へのスムーススクロールで中断*/
			if ((err = cget_pos(scrbarPAID, &sbarRECT)) < 0) {
				continue;
			}
			pressval = (wev->s.pos.y - sbarRECT.c.top)*
				(*hi - *lo)/(sbarRECT.c.bottom-sbarRECT.c.top) + *lo;
			smoothscrolldiff = (pressval - (*chi+*clo)/2)/5;
			if (smoothscrolldiff == 0) {
				continue;
			}
			if ((*clo + smoothscrolldiff) < *lo) {
				if (*lo >= *clo) {
					continue;
				}
				dh = 0;
				dv = -(*lo - *clo);
			} else if ((*chi + smoothscrolldiff) > *hi) {
				if (*hi <= *chi) {
					continue;
				}
				dh = 0;
				dv = -(*hi - *chi);
			} else {
				dh = 0;
				dv = -smoothscrolldiff;
			}
			wscr->work_t -= dv;
			wscr->work_b -= dv;
			commonwindow_callback_scroll(wscr, dh, dv);
			commonwindow_redisp(wscr);
			commonwindow_updaterbar(wscr);
			continue;
		case 0x6002:	/*左へのスムーススクロールで中断*/
		case 0x6003:	/*右へのスムーススクロールで中断*/
			if ((err = cget_pos(scrbarPAID, &sbarRECT)) < 0) {
				continue;
			}
			pressval = (wev->s.pos.x - sbarRECT.c.left)*
				(*lo - *hi)/(sbarRECT.c.right-sbarRECT.c.left) + *hi;
			smoothscrolldiff = (pressval - (*clo+*chi)/2)/5;
			if (smoothscrolldiff == 0) {
				continue;
			}
			if ((*clo + smoothscrolldiff) > *lo) {
				if (*lo <= *clo) {
					continue;
				}
				dh = -(*lo - *clo);
				dv = 0;
			} else if ((*chi + smoothscrolldiff) < *hi) {
				if (*hi >= *chi) {
					continue;
				}
				dh = -(*hi - *chi);
				dv = 0;
			} else {
				dh = -smoothscrolldiff;
				dv = 0;
			}
			wscr->work_l -= dh;
			wscr->work_r -= dh;
			commonwindow_callback_scroll(wscr, dh, dv);
			commonwindow_redisp(wscr);
			commonwindow_updatebbar(wscr);
			continue;
		case 0x5004:	/*上へのエリアスクロールで終了*/
			if ((wscr->work_t - (*chi-*clo)) < *lo) {
				dh = 0;
				dv = -(*lo - wscr->work_t);
			} else {
				dh = 0;
				dv = (*chi - *clo);
			}
			wscr->work_t -= dv;
			wscr->work_b -= dv;
			commonwindow_callback_scroll(wscr, dh, dv);
			commonwindow_redisp(wscr);
			commonwindow_updaterbar(wscr);
			break;
		case 0x5005:	/*下へのエリアスクロールで終了*/
			if((wscr->work_b + (*chi-*clo)) > *hi){
				dh = 0;
				dv = -(*hi - wscr->work_b);
			} else {
				dh = 0;
				dv = -(*chi - *clo);
			}
			wscr->work_t -= dv;
			wscr->work_b -= dv;
			commonwindow_callback_scroll(wscr, dh, dv);
			commonwindow_redisp(wscr);
			commonwindow_updaterbar(wscr);
			break;
		case 0x5006:	/*左へのエリアスクロールで終了*/
			if((wscr->work_l - (*clo-*chi)) < *hi){
				dh = -(*hi - wscr->work_l);
				dv = 0;
			} else {
				dh = *clo - *chi;
				dv = 0;
			}
			wscr->work_l -= dh;
			wscr->work_r -= dh;
			commonwindow_callback_scroll(wscr, dh, dv);
			commonwindow_redisp(wscr);
			commonwindow_updatebbar(wscr);
			break;
		case 0x5007:	/*右へのエリアスクロールで終了*/
			if((wscr->work_r + (*clo-*chi)) > *lo){
				dh = -(*lo - wscr->work_r);
				dv = 0;
			} else {
				dh = -(*clo - *chi);
				dv = 0;
			}
			wscr->work_l -= dh;
			wscr->work_r -= dh;
			commonwindow_callback_scroll(wscr, dh, dv);
			commonwindow_redisp(wscr);
			commonwindow_updatebbar(wscr);
			break;
		}
		break;
	}

	return 0;
}

EXPORT VOID commonwindow_scrollbyvalue(commonwindow_t *wscr, W dh, W dv)
{
	wscr->work_l += dh;
	wscr->work_t += dv;
	wscr->work_r += dh;
	wscr->work_b += dv;
	commonwindow_callback_scroll(wscr, -dh, -dv);
	commonwindow_redisp(wscr);
	commonwindow_updatebar(wscr);
}

LOCAL W commonwindow_weventrbar(commonwindow_t *wscr, WEVENT *wev)
{
	return commonwindow_scrollbar(wscr, wev, wscr->rbar);
}

LOCAL W commonwindow_weventbbar(commonwindow_t *wscr, WEVENT *wev)
{
	return commonwindow_scrollbar(wscr, wev, wscr->bbar);
}

EXPORT W commonwindow_setdrawrect(commonwindow_t *wscr, W l, W t, W r, W b)
{
	wscr->draw_l = l;
	wscr->draw_t = t;
	wscr->draw_r = r;
	wscr->draw_b = b;
	return commonwindow_updatebar(wscr);
}

EXPORT VOID commonwindow_weventbutdn(commonwindow_t *wscr, WEVENT *wev)
{
	W i;
	RECT r;

	switch	(wev->s.cmd) {
		case	W_PICT:
			switch (wchk_dck(wev->s.time)) {
				case	W_DCLICK:
					commonwindow_callback_close(wscr);
				case	W_PRESS:
					break;
				default:
					return;
			}
		case	W_FRAM:
		case	W_TITL:
			if (wmov_drg(wev, NULL) > 0) {
				commonwindow_redisp(wscr);
			}
			return;
		case	W_LTHD:
		case	W_RTHD:
		case	W_LBHD:
		case	W_RBHD:
			switch (wchk_dck(wev->s.time)) {
				case	W_DCLICK:
					i = wchg_wnd(wscr->wid, NULL, W_MOVE);
					break;
				case	W_PRESS:
					i = wrsz_drg(wev, NULL, NULL);
					break;
				default:
					return;
			}
			wget_wrk(wscr->wid, &r);
			if (r.c.left != 0) {
				r.c.left = 0;
				wera_wnd(wscr->wid, NULL);
				wreq_dsp(wscr->wid);
			}
			if (r.c.top != 0) {
				r.c.top = 0;
				wera_wnd(wscr->wid, NULL);
				wreq_dsp(wscr->wid);
			}
			wset_wrk(wscr->wid, &r); /*左上を原点に*/
			gset_vis(wscr->gid, r);	/*作業領域を表示長方形に*/

			commonwindow_callback_resize(wscr);
			commonwindow_updatebar(wscr);
			if (i > 0) {
				commonwindow_redisp(wscr);
			}
			return;
		case	W_RBAR:
			commonwindow_weventrbar(wscr, wev);
			return;
		case	W_BBAR:
			commonwindow_weventbbar(wscr, wev);
			return;
		case	W_WORK:
			switch (wchk_dck(wev->s.time)) {
				case	W_CLICK:
					commonwindow_callback_click(wscr, wev->s.pos);
					return;
				case	W_DCLICK:
					commonwindow_callback_dclick(wscr, wev->s.pos);
					return;
				case	W_PRESS:
					commonwindow_callback_press(wscr, wev);
					return;
				case	W_QPRESS:
					commonwindow_callback_qpress(wscr, wev);
					return;
				default:
					return;
			}
			return;
	}
	return;
}

EXPORT VOID commonwindow_weventrequest(commonwindow_t *wscr, WEVENT *wev)
{
	switch (wev->g.cmd) {
	case	W_REDISP:
		commonwindow_redisp(wscr);
		break;
	case	W_PASTE:
		wrsp_evt(wev, 1);	/*NACK*/
		break;
	case	W_DELETE:
	case	W_FINISH:
		wrsp_evt(wev, 0);	/*ACK*/
		commonwindow_callback_close(wscr);
	}
}

EXPORT VOID commonwindow_weventswitch(commonwindow_t *wscr, WEVENT *wev)
{
	commonwindow_weventbutdn(wscr, wev);
}

EXPORT VOID commonwindow_weventreswitch(commonwindow_t *wscr, WEVENT *wev)
{
	commonwindow_redisp(wscr);
	commonwindow_weventbutdn(wscr, wev);
}
