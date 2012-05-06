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
#include	<btron/hmi.h>

#include    "util.h"

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
	TRAYREC *rec;
	W recnum;
	enum {
		TRAYDATA_TEXTITERATOR_STATE_READING,
		TRAYDATA_TEXTITERATOR_STATE_END,
	} state;
	W i_rec;
	W rec_read_len;
};
typedef struct traydata_textiterator_t_ traydata_textiterator_t;

LOCAL VOID traydata_textiterator_initialize(traydata_textiterator_t *iter, TRAYREC *rec, W recnum)
{
	iter->rec = rec;
	iter->recnum = recnum;
	iter->i_rec = 0;

	if (iter->i_rec < recnum) {
		if (rec[iter->i_rec].id == TS_INFO) {
			iter->i_rec++;
		}
	}
	if (iter->i_rec < recnum) {
		if (rec[iter->i_rec].id == TS_TEXT) {
			iter->state = TRAYDATA_TEXTITERATOR_STATE_READING;
		} else {
			iter->state = TRAYDATA_TEXTITERATOR_STATE_END;
		}
		iter->i_rec++;
	} else {
		iter->state = TRAYDATA_TEXTITERATOR_STATE_END;
	}
	iter->rec_read_len = 0;
}

LOCAL Bool traydata_textiterator_getnext(traydata_textiterator_t *iter, TC *ch)
{
	TC *ch0;
	LTADSEG *seg0;
	W skipsize;
	Bool ch_set;

	if (iter->state == TRAYDATA_TEXTITERATOR_STATE_END) {
		return False;
	}

	for (; iter->i_rec < iter->recnum;) {
		if (iter->rec[iter->i_rec].id != TR_TEXT && iter->rec[iter->i_rec].id != (TR_TEXT | TR_CONT)) {
			iter->i_rec++;
			continue;
		}

		ch_set = False;
		for (;iter->i_rec < iter->recnum;) {
			if (iter->rec_read_len >= iter->rec[iter->i_rec].len) {
				if (iter->rec[iter->i_rec].id == TR_TEXT) {
					break;
				}
				iter->i_rec++;
				iter->rec_read_len = 0;
				continue;
			}
			if (iter->rec_read_len == (iter->rec[iter->i_rec].len + 1)) {
				if (iter->rec[iter->i_rec].id == TR_TEXT) {
					/* tmp */
					iter->i_rec++;
					iter->rec_read_len = 0;
					break;
				}
				*ch = iter->rec[iter->i_rec].dt[iter->rec_read_len] << 8;
				iter->i_rec++;
				if (iter->rec[iter->i_rec].id != TR_TEXT && iter->rec[iter->i_rec].id != (TR_TEXT | TR_CONT)) {
					iter->rec_read_len = 0;
					break;
				}
				*ch |= iter->rec[iter->i_rec].dt[0];
				iter->rec_read_len = 1;
				ch_set = True;
			}
			ch0 = (TC*)(iter->rec[iter->i_rec].dt + iter->rec_read_len);
			if ((*ch0 & 0xFF00) == 0xFF00) {
				seg0 = (LTADSEG*)ch0;
				if (seg0->len == 0xffff) {
					skipsize = 2 + seg0->llen + 6;
				} else {
					skipsize = 2 + seg0->len + 2;
				}
				for (; skipsize > 0;) {
					if (iter->rec_read_len + skipsize > iter->rec[iter->i_rec].len) {
						skipsize -= iter->rec[iter->i_rec].len - iter->rec_read_len;
						iter->i_rec++;
						iter->rec_read_len = 0;
						break;
					} else {
						iter->rec_read_len += skipsize;
						skipsize = 0;
					}
				}
				break;
			} else {
				*ch = *ch0;
				iter->rec_read_len += sizeof(TC);
				if (iter->rec_read_len == iter->rec[iter->i_rec].len) {
					iter->i_rec++;
					iter->rec_read_len = 0;
					if (iter->rec[iter->i_rec].id != TR_TEXT && iter->rec[iter->i_rec].id != (TR_TEXT | TR_CONT)) {
						iter->state = TRAYDATA_TEXTITERATOR_STATE_READING;
					}
				}
				ch_set = True;
				break;
			}
		}

		if (ch_set != False) {
			break;
		}
	}

	if (iter->i_rec >= iter->recnum) {
		iter->state = TRAYDATA_TEXTITERATOR_STATE_END;
	}

	return True;
}

LOCAL VOID traydata_textiterator_finalize(traydata_textiterator_t *iter)
{
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
