/*
 * bchanl_menus.h
 *
 * Copyright (c) 2011-2012 project bchan
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

#ifndef __BCHANL_MENUS_H__
#define __BCHANL_MENUS_H__

struct bchanl_mainmenu_t_ {
	MENUITEM *mnitem;
	MNID mnid;
};
typedef struct bchanl_mainmenu_t_ bchanl_mainmenu_t;

IMPORT W bchanl_mainmenu_initialize(bchanl_mainmenu_t *mainmenu, W dnum);
IMPORT VOID bchanl_mainmenu_finalize(bchanl_mainmenu_t *mainmenu);
IMPORT W bchanl_mainmenu_setup(bchanl_mainmenu_t *mainmenu, Bool subectactive, Bool subectjoptionenable, Bool extbbsmanageropen, Bool extbbsselected, Bool fromtray, Bool totray, Bool resnumdisplay, Bool sincedisplay, Bool vigordisplay);
#define BCHANL_MAINMENU_SELECT_NOSELECT 0
#define BCHANL_MAINMENU_SELECT_CLOSE 1
#define BCHANL_MAINMENU_SELECT_REDISPLAY 2
#define BCHANL_MAINMENU_SELECT_BBSMENUFETCH 3
#define BCHANL_MAINMENU_SELECT_SUBJECTOPTION 4
#define BCHANL_MAINMENU_SELECT_EXTBBS_MANAGER 5
#define BCHANL_MAINMENU_SELECT_EXTBBS_REGISTER 6
#define BCHANL_MAINMENU_SELECT_EXTBBS_UP 7
#define BCHANL_MAINMENU_SELECT_EXTBBS_DOWN 8
#define BCHANL_MAINMENU_SELECT_EXTBBS_DELETE 9
#define BCHANL_MAINMENU_SELECT_EDIT_COPY_TO_TRAY 10
#define BCHANL_MAINMENU_SELECT_EDIT_COPY_FROM_TRAY 11
#define BCHANL_MAINMENU_SELECT_EDIT_MOVE_TO_TRAY 12
#define BCHANL_MAINMENU_SELECT_EDIT_MOVE_FROM_TRAY 13
#define BCHANL_MAINMENU_SELECT_EDIT_DELETE 14
#define BCHANL_MAINMENU_SELECT_DISPLAY_RESNUMBER 15
#define BCHANL_MAINMENU_SELECT_DISPLAY_SINCE 16
#define BCHANL_MAINMENU_SELECT_DISPLAY_VIGOR 17
IMPORT W bchanl_mainmenu_popup(bchanl_mainmenu_t *mainmenu, PNT pos);
IMPORT W bchanl_mainmenu_keyselect(bchanl_mainmenu_t *mainmenu, TC keycode);

#endif
