/*
 * bchanl_menus.c
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

#include    "bchanl_menus.h"

#include	<basic.h>
#include	<bstdio.h>
#include	<bstdlib.h>
#include	<btron/hmi.h>
#include	<btron/vobj.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

#define BCHANL_MAINMENU_ITEMNUM_WINDOW 4
#define BCHANL_MAINMENU_ITEMNUM_GADGET (BCHANL_MAINMENU_ITEMNUM_WINDOW + 1)

EXPORT W bchanl_mainmenu_setup(bchanl_mainmenu_t *mainmenu, Bool subectjoptionenable, Bool extbbsmanageropen, Bool extbbsselected)
{
	/* [ɽ��] -> [�����������] */
	if (subectjoptionenable == False) {
		mchg_atr(mainmenu->mnid, (1 << 8)|2, M_NOSEL);
	} else {
		mchg_atr(mainmenu->mnid, (1 << 8)|2, M_SEL);
	}

	/* [������] -> [�����Ĵ���] */
	if (extbbsmanageropen == False) {
		mchg_atr(mainmenu->mnid, (3 << 8)|1, M_NOSEL);
	} else {
		mchg_atr(mainmenu->mnid, (3 << 8)|1, M_SEL);
	}

	/* [������] -> [���ɲ�] */
	if (extbbsmanageropen != False) {
		mchg_atr(mainmenu->mnid, (3 << 8)|3, M_ACT);
	} else {
		mchg_atr(mainmenu->mnid, (3 << 8)|3, M_INACT);
	}
	/* [������] -> [��ľ夲��] */
	/* [������] -> [��Ĳ�����] */
	/* [������] -> [���] */
	if (extbbsselected == False) {
		mchg_atr(mainmenu->mnid, (3 << 8)|4, M_INACT);
		mchg_atr(mainmenu->mnid, (3 << 8)|5, M_INACT);
		mchg_atr(mainmenu->mnid, (3 << 8)|6, M_INACT);
	} else {
		mchg_atr(mainmenu->mnid, (3 << 8)|4, M_ACT);
		mchg_atr(mainmenu->mnid, (3 << 8)|5, M_ACT);
		mchg_atr(mainmenu->mnid, (3 << 8)|6, M_ACT);
	}

	wget_dmn(&(mainmenu->mnitem[BCHANL_MAINMENU_ITEMNUM_WINDOW].ptr));
	mset_itm(mainmenu->mnid, BCHANL_MAINMENU_ITEMNUM_WINDOW, mainmenu->mnitem+BCHANL_MAINMENU_ITEMNUM_WINDOW);
	oget_men(0, NULL, &(mainmenu->mnitem[BCHANL_MAINMENU_ITEMNUM_GADGET].ptr), NULL, NULL);
	mset_itm(mainmenu->mnid, BCHANL_MAINMENU_ITEMNUM_GADGET, mainmenu->mnitem+BCHANL_MAINMENU_ITEMNUM_GADGET);

	return 0; /* tmp */
}

LOCAL W bchanl_mainmenu_select(bchanl_mainmenu_t *mainmenu, W i)
{
	W ret;

	switch(i >> 8) {
	case 0: /* [��λ] */
		ret = BCHANL_MAINMENU_SELECT_CLOSE;
		break;
	case 1: /* [ɽ��] */
		switch (i & 0xff) {
		case 1: /* [��ɽ��] */
			ret = BCHANL_MAINMENU_SELECT_REDISPLAY;
			break;
		case 2: /* [�����������] */
			ret = BCHANL_MAINMENU_SELECT_SUBJECTOPTION;
			break;
		default:
			ret = BCHANL_MAINMENU_SELECT_NOSELECT;
			break;
		}
		break;
	case 2:	/* [���] */
		switch (i & 0xff) {
		case 1: /* [�İ����Ƽ���] */
			ret = BCHANL_MAINMENU_SELECT_BBSMENUFETCH;
			break;
		default:
			ret = BCHANL_MAINMENU_SELECT_NOSELECT;
			break;
		}
		break;
	case 3:	/* [������] */
		switch (i & 0xff) {
		case 1: /* [�����Ĵ���] */
			ret = BCHANL_MAINMENU_SELECT_EXTBBS_MANAGER;
			break;
		case 3: /* [���ɲ�] */
			ret = BCHANL_MAINMENU_SELECT_EXTBBS_REGISTER;
			break;
		case 4: /* [��ľ夲��] */
			ret = BCHANL_MAINMENU_SELECT_EXTBBS_UP;
			break;
		case 5: /* [��Ĳ�����] */
			ret = BCHANL_MAINMENU_SELECT_EXTBBS_DOWN;
			break;
		case 6: /* [���] */
			ret = BCHANL_MAINMENU_SELECT_EXTBBS_DELETE;
			break;
		default:
			ret = BCHANL_MAINMENU_SELECT_NOSELECT;
			break;
		}
		break;
	case BCHANL_MAINMENU_ITEMNUM_WINDOW: /* [������ɥ�] */
		wexe_dmn(i);
		ret = BCHANL_MAINMENU_SELECT_NOSELECT;
		break;
	case BCHANL_MAINMENU_ITEMNUM_GADGET: /* [��ʪ] */
	    oexe_apg(0, i);
		ret = BCHANL_MAINMENU_SELECT_NOSELECT;
		break;
	default:
		ret = BCHANL_MAINMENU_SELECT_NOSELECT;
		break;
	}

	return ret;
}

EXPORT W bchanl_mainmenu_popup(bchanl_mainmenu_t *mainmenu, PNT pos)
{
	W i;
	gset_ptr(PS_SELECT, NULL, -1, -1);
	i = msel_men(mainmenu->mnid, pos);
	if (i < 0) {
		DP_ER("msel_men error:", i);
		return i;
	}
	if (i == 0) {
		return BCHANL_MAINMENU_SELECT_NOSELECT;
	}
	return bchanl_mainmenu_select(mainmenu, i);
}

EXPORT W bchanl_mainmenu_keyselect(bchanl_mainmenu_t *mainmenu, TC keycode)
{
	W i;
	i = mfnd_key(mainmenu->mnid, keycode);
	if (i < 0) {
		DP_ER("mfnd_key error:", i);
		return i;
	}
	if (i == 0) {
		return BCHANL_MAINMENU_SELECT_NOSELECT;
	}
	return bchanl_mainmenu_select(mainmenu, i);
}

EXPORT W bchanl_mainmenu_initialize(bchanl_mainmenu_t *mainmenu, W dnum)
{
	MENUITEM *mnitem_dbx, *mnitem;
	MNID mnid;
	W len, err;

	err = dget_dtp(8, dnum, (void**)&mnitem_dbx);
	if (err < 0) {
		DP_ER("dget_dtp error:", err);
		return err;
	}
	len = dget_siz((B*)mnitem_dbx);
	mnitem = malloc(len);
	if (mnitem == NULL) {
		DP_ER("mallod error", err);
		return -1;
	}
	memcpy(mnitem, mnitem_dbx, len);
	mnid = mcre_men(BCHANL_MAINMENU_ITEMNUM_WINDOW+2, mnitem, NULL);
	if (mnid < 0) {
		DP_ER("mcre_men error", mnid);
		free(mnitem);
		return -1;
	}

	mainmenu->mnid = mnid;
	mainmenu->mnitem = mnitem;

	return 0;
}

EXPORT VOID bchanl_mainmenu_finalize(bchanl_mainmenu_t *mainmenu)
{
	mdel_men(mainmenu->mnid);
	free(mainmenu->mnitem);
}
