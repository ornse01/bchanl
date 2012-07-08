/*
 * util.c
 *
 * Copyright (c) 2012 project bchan
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
#include	<bstdio.h>
#include	<bstdlib.h>
#include	<tcode.h>
#include	<tstring.h>
#include	<errcode.h>
#include	<btron/btron.h>
#include	<btron/hmi.h>

#include    "util.h"
#include	<tad/traydata_iterator.h>
#include	<tad/taditerator.h>
#include	<tad/tadstack.h>

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

EXPORT TCURL_CHECK_VALID_BBSURL tcurl_check_valid_bbsurl(CONST TC *url, W url_len)
{
	TC scheme[] = {TK_h, TK_t, TK_t, TK_p, TK_COLN, TK_SLSH, TK_SLSH, TNULL};
	TC *p;
	W cmp;

	cmp = tc_strncmp(url, scheme, 7);
	if (cmp != 0) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_SCHEME;
	}
	if (url_len == 7) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_HOST;
	}
	p = tc_strchr(url+7, TK_SLSH);
	if (p == NULL) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	}
	if ((p+1) == (url+url_len)) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	}
	if (p == (url+7)) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_HOST;
	}
	if ((p+1) == (url+url_len)) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	}
	p = tc_strchr(p+1, TK_SLSH);
	if (p == NULL) {
		return TCURL_CHECK_VALID_BBSURL_NO_LAST_SLSH;
	}
	if ((p+1) != (url+url_len)) {
		return TCURL_CHECK_VALID_BBSURL_INVALID_PATH;
	}

	return TCURL_CHECK_VALID_BBSURL_OK;
}

EXPORT W tray_pushstring(TC *str, W len)
{
	W err;
	TRAYREC trayrec[2];
	UB bin[4+24];
	TADSEG *base = (TADSEG*)bin;
	TEXTSEG *textseg = (TEXTSEG*)(bin + 4);

	base->id = 0xFFE1;
	base->len = 24;
	textseg->view = (RECT){{0, 0, 0, 0}};
	textseg->draw = (RECT){{0, 0, 0, 0}};
	textseg->h_unit = -120;
	textseg->v_unit = -120;
	textseg->lang = 0x21;
	textseg->bgpat = 0;

	trayrec[0].id = 0xE1;
	trayrec[0].len = 28;
	trayrec[0].dt = bin;
	trayrec[1].id = TR_TEXT;
	trayrec[1].len = len * sizeof(TC);
	trayrec[1].dt = (B*)str;

	err = tpsh_dat(trayrec, 2, NULL);
	if (err < 0) {
		DP_ER("tpsh_dat", err);
	}
	return err;
}

struct traydata_textiterator_t_ {
	traydata_iterator_t base;
};
typedef struct traydata_textiterator_t_ traydata_textiterator_t;

LOCAL VOID traydata_textiterator_initialize(traydata_textiterator_t *iter, TRAYREC *rec, W recnum)
{
	traydata_iterator_initialize(&iter->base, rec, recnum);
}

LOCAL Bool traydata_textiterator_getnext(traydata_textiterator_t *iter, TC *ch)
{
	traydata_iterator_result result;
	Bool cont;

	for (;;) {
		cont = traydata_iterator_next(&iter->base, &result);
		if (cont == False) {
			break;
		}
		if (result.type == TRAYDATA_ITERATOR_RESULTTYPE_FIXED_SEGMENT) {
			*ch = result.val.ch;
			return True;
		}
	}

	return False;
}

LOCAL VOID traydata_textiterator_finalize(traydata_textiterator_t *iter)
{
	traydata_iterator_finalize(&iter->base);
}

struct traydata_vobjiterator_t_ {
	traydata_iterator_t base;
	VLINK buf;
};
typedef struct traydata_vobjiterator_t_ traydata_vobjiterator_t;

LOCAL VOID traydata_vobjiterator_initialize(traydata_vobjiterator_t *iter, TRAYREC *rec, W recnum)
{
	traydata_iterator_initialize(&iter->base, rec, recnum);
}

LOCAL Bool traydata_vobjiterator_getnext(traydata_vobjiterator_t *iter, VLINK **lnk)
{
	traydata_iterator_result result;
	Bool cont;
	W writelen = 0, cpsize;

	for (;;) {
		cont = traydata_iterator_next(&iter->base, &result);
		if (cont == False) {
			break;
		}
		if (result.type == TRAYDATA_ITERATOR_RESULTTYPE_VOBJREC_CONT) {
			if (writelen + result.val.vobj.chunk_data_len > sizeof(VLINK)) {
				cpsize = sizeof(VLINK) - writelen;
			} else {
				cpsize = result.val.vobj.chunk_data_len;
			}
			memcpy(((UB*)&iter->buf) + writelen, result.val.vobj.chunk_data, cpsize);
			writelen += cpsize;
			if (cpsize >= sizeof(VLINK)) {
				*lnk = &iter->buf;
				return True;
			}
			continue;
		}
		if (result.type == TRAYDATA_ITERATOR_RESULTTYPE_VOBJREC) {
			if (writelen + result.val.vobj.chunk_data_len > sizeof(VLINK)) {
				cpsize = sizeof(VLINK) - writelen;
			} else {
				cpsize = result.val.vobj.chunk_data_len;
			}
			memcpy(((UB*)&iter->buf) + writelen, result.val.vobj.chunk_data, cpsize);
			writelen += cpsize;
			if (cpsize >= sizeof(VLINK)) {
				*lnk = &iter->buf;
				return True;
			}
			return False;
		}
	}

	return False;
}

LOCAL VOID traydata_vobjiterator_finalize(traydata_vobjiterator_t *iter)
{
	traydata_iterator_finalize(&iter->base);
}

EXPORT W tray_popstring(TC *str, W len)
{
	TRAYREC *data;
	W size, recs, err, i;
	traydata_textiterator_t iter;
	Bool cont;
	TC ch;

	err = tpop_dat(NULL, 0, &size, -1, NULL);
	if (err < 0) {
		DP_ER("tpop_dat: get size error", err);
		return err;
	}
	if (err == 0) {
		/* tray is empty */
		return 0;
	}
	data = (TRAYREC*)malloc(size);
	if (data == NULL) {
		return ER_NOMEM;
	}
	recs = tpop_dat(data, size, NULL, -1, NULL);
	if (recs < 0) {
		DP_ER("tpop_dat: get data error", recs);
		free(data);
		return recs;
	}

	traydata_textiterator_initialize(&iter, data, recs);
	for (i = 0; i < len - 1;) {
		cont = traydata_textiterator_getnext(&iter, &ch);
		if (cont == False) {
			break;
		}
		if (ch == TK_NL) {
			break;
		}
		if (str != NULL) {
			str[i++] = ch;
		}
	}
	traydata_textiterator_finalize(&iter);
	if (str != NULL) {
		str[i] = TNULL;
	}

	free(data);

	return i;
}

EXPORT W tray_deletedata()
{
	return tdel_dat();
}

EXPORT Bool tray_isempty()
{
	W ret;
	ret = tsel_dat(-1);
	if (ret < 0) {
		DP_ER("tsel_dat", ret);
	}
	if (ret == 0) {
		return True;
	}
	return False;
}

LOCAL W tray_getfirstvobj(VLINK *lnk)
{
	TRAYREC *data;
	W size, recs, err;
	traydata_vobjiterator_t iter;
	VLINK *lnk0 = NULL;
	Bool cont;

	err = tget_dat(NULL, 0, &size, -1);
	if (err < 0) {
		DP_ER("tget_dat: get size error", err);
		return err;
	}
	if (err == 0) {
		/* tray is empty */
		return -1; /* TODO */
	}
	data = (TRAYREC*)malloc(size);
	if (data == NULL) {
		return -1; /* TODO */
	}
	recs = tget_dat(data, size, NULL, -1);
	if (recs < 0) {
		DP_ER("tget_dat: get data error", recs);
		free(data);
		return err;
	}

	traydata_vobjiterator_initialize(&iter, data, recs);
	for (;;) {
		cont = traydata_vobjiterator_getnext(&iter, &lnk0);
		if (cont == False) {
			break;
		}
		if (lnk0 != NULL) {
			*lnk = *lnk0;
			break;
		}
	}
	traydata_vobjiterator_finalize(&iter);

	free(data);

	if (lnk0 == NULL) {
		return -1;
	}

	return 0;
}

LOCAL W tray_getextbbsinf_readfiestline(UB *buf, W buflen, TC *url, W url_len)
{
	tadstack_t stack;
	taditerator_t iter;
	taditerator_result result;
	TADSTACK_RESULT stk_result;
	W len = 0;

	tadstack_initialize(&stack);
	taditerator_initialize(&iter, (TC*)buf, buflen/sizeof(TC));

	for (;;) {
		taditerator_next2(&iter, &result);
		if (result.type == TADITERATOR_RESULTTYPE_END) {
			break;
		}
		stk_result = TADSTACK_RESULT_OK;
		if (result.type == TADITERATOR_RESULTTYPE_CHARCTOR) {
			
			stk_result = tadstack_inputcharactor(&stack, result.segment);
			if (stk_result == TADSTACK_RESULT_OK) {
				if (result.segment == TK_NL) {
					break;
				}
				if ((url != NULL)&&(len < url_len)) {
					url[len] = result.segment;
				}
				len++;
			}
		} else if (result.type == TADITERATOR_RESULTTYPE_SEGMENT) {
			stk_result = tadstack_inputvsegment(&stack, result.segment, result.data, result.segsize);
		}
		if (stk_result == TADSTACK_RESULT_FORMAT_ERROR) {
			len = -1;
			break;
		}
	}

	taditerator_finalize(&iter);
	tadstack_finalize(&stack);

	return len;
}

LOCAL W tray_getextbbsinf_readfile(VLINK *vlnk, TC *url, W url_len)
{
	W fd, err, size;
	UB *buf;

	fd = opn_fil((LINK*)vlnk, F_READ, NULL);
	if (fd < 0) {
		return fd;
	}
	err = fnd_rec(fd, F_TOPEND, RM_TADDATA, 0, NULL);
	if (err < 0) {
		cls_fil(fd);
		return err;
	}
	err = rea_rec(fd, 0, NULL, 0, &size, NULL);
	if (err < 0) {
		cls_fil(fd);
		return err;
	}
	buf = malloc(size*sizeof(UB));
	if (buf == NULL) {
		cls_fil(fd);
		return -1; /* TODO */
	}
	err = rea_rec(fd, 0, buf, size, NULL, NULL);
	if (err < 0) {
		free(buf);
		cls_fil(fd);
		return err;
	}
	cls_fil(fd);

	err = tray_getextbbsinf_readfiestline(buf, size, url, url_len);

	free(buf);

	return err;
}

EXPORT W tray_getextbbsinfo(TC *name, W *name_len, TC *url, W *url_len)
{
	VLINK lnk;
	W err;
	TC fname[L_FNM + 1];

	err = tray_getfirstvobj(&lnk);
	if (err < 0) {
		return err;
	}

	err = fil_sts((LINK*)&lnk, fname, NULL, NULL);
	if (err < 0) {
		return err;
	}
	if (name != NULL) {
		tc_strncpy(name, fname, *name_len);
	} else if (name_len != NULL) {
		*name_len = tc_strlen(fname);
	}

	err = tray_getextbbsinf_readfile(&lnk, url, *url_len);
	if (err < 0) {
		return err;
	}
	if (url_len != NULL) {
		*url_len = err;
	}

	return 0;
}
