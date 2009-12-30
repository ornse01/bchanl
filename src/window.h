/*
 * window.h
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
#include	<btron/hmi.h>

#ifndef __WINDOW_H__
#define __WINDOW_H__

typedef struct commonwindow_t_ commonwindow_t;

typedef VOID (*commonwindow_scrollcalback)(VP arg, W dh, W dv);
typedef VOID (*commonwindow_drawcallback)(VP arg, RECT *r);
typedef VOID (*commonwindow_resizecallback)(VP arg);
typedef VOID (*commonwindow_closecallback)(VP arg);
typedef VOID (*commonwindow_clickcallback)(VP arg, PNT pos);
typedef VOID (*commonwindow_dclickcallback)(VP arg, PNT pos);
typedef VOID (*commonwindow_presscallback)(VP arg, WEVENT *wev);
typedef VOID (*commonwindow_qpresscallback)(VP arg, WEVENT *wev);

IMPORT commonwindow_t* commonwindow_new(WID target, VP arg);
IMPORT VOID commonwindow_delete(commonwindow_t *wscr);
IMPORT VOID commonwindow_setscrollcallback(commonwindow_t *wscr, commonwindow_scrollcalback scrollcallback);
IMPORT VOID commonwindow_setdrawcallback(commonwindow_t *wscr, commonwindow_drawcallback drawcallback);
IMPORT VOID commonwindow_setresizecallback(commonwindow_t *wscr, commonwindow_resizecallback resizecallback);
IMPORT VOID commonwindow_setclosecallback(commonwindow_t *wscr, commonwindow_closecallback closecallback);
IMPORT VOID commonwindow_setclickcallback(commonwindow_t *wscr, commonwindow_clickcallback clickcallback);
IMPORT VOID commonwindow_setdclickcallback(commonwindow_t *wscr, commonwindow_dclickcallback dclickcallback);
IMPORT VOID commonwindow_setpresscallback(commonwindow_t *wscr, commonwindow_presscallback presscallback);
IMPORT VOID commonwindow_setqpresscallback(commonwindow_t *wscr, commonwindow_qpresscallback qpresscallback);
IMPORT W commonwindow_updatebar(commonwindow_t *wscr);
IMPORT VOID commonwindow_weventbutdn(commonwindow_t *wscr, WEVENT *wev);
IMPORT VOID commonwindow_weventrequest(commonwindow_t *wscr, WEVENT *wev);
IMPORT VOID commonwindow_weventswitch(commonwindow_t *wscr, WEVENT *wev);
IMPORT VOID commonwindow_weventreswitch(commonwindow_t *wscr, WEVENT *wev);
IMPORT VOID commonwindow_scrollbyvalue(commonwindow_t *wscr, W dh, W dv);
IMPORT W commonwindow_setdrawrect(commonwindow_t *wscr, W l, W t, W r, W b);
IMPORT W commonwindow_setworkrect(commonwindow_t *wscr, W l, W t, W r, W b);

#endif
