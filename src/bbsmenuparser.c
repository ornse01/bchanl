/*
 * bbsmenuparser.c
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
#include	<bstdio.h>
#include	<bstdlib.h>
#include	<bstring.h>
#include	<bctype.h>
#include	<btron/tf.h>

#include    "bbsmenuparser.h"
#include    "parselib.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

LOCAL tokenchecker_valuetuple_t nList_element[] = {
  {NULL,0},
  {"A HREF", 1},
  {"B", 2},
  {NULL,0}
};
LOCAL B eToken_element[] = ">=";

struct bbsmnparser_t_ {
	bbsmncache_t *cache;
	TF_CTX ctx;
	W i;
	/**/
	Bool isLSTN;
	enum {
		STATE_SEARCHTAG,
		STATE_ELEMENTCHECK,
		STATE_READBOARDURL,
		STATE_READBOARDURL_2,
		STATE_READBOARDTITLE,
		STATE_READCATEGORY
	} state;
	tokenchecker_t tokenchecker;
	bbsmnparser_item_t *itembuffer;
};

EXPORT bbsmnparser_item_t* bbsmnparser_item_new()
{
	bbsmnparser_item_t *item;

	item = malloc(sizeof(bbsmnparser_item_t));
	if (item == NULL) {
		return NULL;
	}
	item->category = NULL;
	item->category_len = NULL;
	item->url = malloc(sizeof(UB)*1);
	item->url_len = 0;
	item->url[0] = '\0';
	item->title = NULL;
	item->title_len = 0;

	return item;
}

LOCAL VOID bbsmnparser_item_clear(bbsmnparser_item_t *item)
{
	if (item->category != NULL) {
		free(item->category);
	}
	if (item->url != NULL) {
		free(item->url);
	}
	if (item->title != NULL) {
		free(item->title);
	}
	item->url = malloc(sizeof(UB)*1);
	item->url_len = 0;
	item->url[0] = '\0';
	item->title = NULL;
	item->title_len = 0;
	item->category = NULL;
	item->category_len = 0;
}

EXPORT VOID bbsmnparser_item_delete(bbsmnparser_item_t *item)
{
	if (item->category != NULL) {
		free(item->category);
	}
	if (item->url != NULL) {
		free(item->url);
	}
	if (item->title != NULL) {
		free(item->title);
	}
	free(item);
}

LOCAL W bbsmnparser_convert_str(bbsmnparser_t *parser, const UB *src, W slen, UW attr, TC **dest, W *dlen)
{
	TC ch;
	W dest_len = 1, err;

	err = tf_strtotcs(parser->ctx, src, slen, attr, &ch, &dest_len);
	if (err < 0) {
		DP_ER("tf_strtotcs error", err);
		return err;
	}
	if (dest_len == 0) {
		return 0;
	}

	*dlen += 1;
	*dest = realloc(*dest, sizeof(TC)*(*dlen + 1));
	(*dest)[*dlen - 1] = ch;
	(*dest)[*dlen] = TNULL;

	if (err == 0) {
		return 0;
	}
	for (;;) {
		dest_len = 1;
		err = tf_strtotcs(parser->ctx, NULL, slen, attr, &ch, &dest_len);
		if (err < 0) {
			DP_ER("tf_strtotcs error:", err);
			return err;
		}
		if (dest_len == 0) {
			return 0;
		}

		*dlen += 1;
		*dest = realloc(*dest, sizeof(TC)*(*dlen + 1));
		(*dest)[*dlen - 1] = ch;
		(*dest)[*dlen] = TNULL;

		if (err == 0) {
			break;
		}
	}

	return 0;
}

LOCAL W bbsmnparser_parsechar(bbsmnparser_t *parser, UB ch, bbsmnparser_item_t *item)
{
	TC **str;
	W *len, ret, err;

	switch (parser->state) {
	case STATE_READBOARDTITLE:
		str = &(item->title);
		len = &(item->title_len);
		break;
	case STATE_READCATEGORY:
		str = &(item->category);
		len = &(item->category_len);
		break;
	default:
		str = NULL;
		len = NULL;
		break;
	}

	switch (parser->state) {
	case STATE_SEARCHTAG:
		if (ch == '<') {
			parser->state = STATE_ELEMENTCHECK;
		}
		break;
	case STATE_ELEMENTCHECK:
		ret = tokenchecker_inputcharacter(&(parser->tokenchecker), ch);
		if (ret == TOKENCHECK_CONTINUE) {
			break;
		}
		tokenchecker_resetstate(&(parser->tokenchecker));
		if (ret == TOKENCHECK_NOMATCH) {
		} else if (ret == 1) {
			/* "A HREF " */
			parser->state = STATE_READBOARDURL;
			break;
		} else if (ret == 2) {
			/* "B" */
			parser->state = STATE_READCATEGORY;
			break;
		} else {
			DP(("error\n"));
		}
		parser->state = STATE_SEARCHTAG;
		break;
	case STATE_READBOARDURL:
		if (ch == ' ') {
			parser->state = STATE_READBOARDURL_2;
			break;
		}
		if (ch == '>') {
			parser->state = STATE_READBOARDTITLE;
			break;
		}
		item->url_len++;
		item->url = realloc(item->url, item->url_len + 1);
		strncat(item->url, &ch, 1);
		break;
	case STATE_READBOARDURL_2:
		if (ch == '>') {
			parser->state = STATE_READBOARDTITLE;
			break;
		}
		break;
	case STATE_READBOARDTITLE:
		if (ch == '<') {
			parser->state = STATE_ELEMENTCHECK;
			err = bbsmnparser_convert_str(parser, NULL, 0, TF_ATTR_SUPPRESS_FUSEN, str, len);
			if (err < 0) {
				return err;
			}
			return 1;
		}
		err = bbsmnparser_convert_str(parser, &ch, 1, TF_ATTR_CONT|TF_ATTR_SUPPRESS_FUSEN, str, len);
		if (err < 0) {
			return err;
		}
		break;
	case STATE_READCATEGORY:
		if (ch == '<') {
			parser->state = STATE_ELEMENTCHECK;
			err = bbsmnparser_convert_str(parser, NULL, 0, TF_ATTR_SUPPRESS_FUSEN, str, len);
			if (err < 0) {
				return err;
			}
			return 1;
		}
		err = bbsmnparser_convert_str(parser, &ch, 1, TF_ATTR_CONT|TF_ATTR_SUPPRESS_FUSEN, str, len);
		if (err < 0) {
			return err;
		}
		break;
	}

	return 0; /* TODO */
}

EXPORT VOID bbsmnparser_item_gethostboard(bbsmnparser_item_t *item, UB **host, W *host_len, UB **board, W *board_len)
{
	W i = 0;
	UB *host0 = NULL, *board0 = NULL;
	W host_len0 = 0, board_len0 = 0;

	host0 = item->url + 7;
	for (i=7; i < item->url_len; i++) {
		if (item->url[i] == '/') {
			break;
		}
		host_len0++;
	}

	i++;
	board0 = item->url + i;
	for (; i < item->url_len; i++) {
		if (item->url[i] == '/') {
			break;
		}
		board_len0++;
	}

	*host = host0;
	*host_len = host_len0;
	*board = board0;
	*board_len = board_len0;
}

EXPORT Bool bbsmnparser_item_checkboradurl(bbsmnparser_item_t *item)
{
	W i = 0;
	UB *host, *board;
	W host_len = 0, board_len = 0;
	int cmp;

	if (item->category != NULL) {
		return True;
	}
	if (item->url == NULL) {
		/* error */
		return False;
	}

	if (item->url_len < 7) {
		/* this is not "http://". too short. */
		return False;
	}
	cmp = strncmp(item->url, "http://", 7);
	if (cmp != 0) {
		/* this is not "http://" */
		return False;
	}

	host = item->url + 7;
	for (i=7; i < item->url_len; i++) {
		if (item->url[i] == '/') {
			break;
		}
		host_len++;
	}
	if (host_len < 8) {
		return False;
	}
	cmp = strncmp(host + host_len - 8, ".2ch.net", 8);
	if (cmp != 0) {
		return False;
	}

	i++;
	board = item->url + i;
	for (; i < item->url_len; i++) {
		if (item->url[i] == '/') {
			break;
		}
		board_len++;
	}
	if (board_len <= 0) {
		return False;
	}

	if ((i+1) != item->url_len) {
		return False;
	}

	return True;
}

EXPORT W bbsmnparser_getnextitem(bbsmnparser_t *parser, bbsmnparser_item_t **item)
{
	bbsmncache_datareadcontext_t *context;
	Bool cont;
	UB *bin_cache;
	W len_cache, i, err = 0;

	*item = NULL;

	context = bbsmncache_startdataread(parser->cache, parser->i);
	if (context == NULL) {
		return -1; /* TODO */
	}

	for (;;) {
		cont = bbsmncache_datareadcontext_nextdata(context, &bin_cache, &len_cache);
		if (cont == False) {
			break;
		}

		for (i = 0; i < len_cache; i++) {
			err = bbsmnparser_parsechar(parser, bin_cache[i], parser->itembuffer);
			if (err < 0) {
				i++;
				break;
			}
			if (err == 1) {
				i++;
				break;
			}
		}

		parser->i += i;
		if (err < 0) {
			break;
		}
		if (err == 1) {
			*item = parser->itembuffer;
			parser->itembuffer = bbsmnparser_item_new();
			break;
		}
	}

	bbsmncache_enddataread(parser->cache, context);

	return err;
}

EXPORT VOID bbsmnparser_clear(bbsmnparser_t *parser)
{
	TC ch;
	W len = 1, err;

	err = tf_strtotcs(parser->ctx, NULL, 0, TF_ATTR_START, &ch, &len);
	if (err != 0) {
		DP_ER("tf_strtotcs (clear)", err);
	}

	parser->i = 0;
	parser->isLSTN = False;
	parser->state = STATE_SEARCHTAG;

	bbsmnparser_item_clear(parser->itembuffer);
}

EXPORT bbsmnparser_t* bbsmnparser_new(bbsmncache_t *cache)
{
	bbsmnparser_t *parser;
	W err, ctx_id;

	parser = malloc(sizeof(bbsmnparser_t));
	if (parser == NULL) {
		return NULL;
	}
	parser->cache = cache;

	tf_open_ctx(&parser->ctx);
	ctx_id = tf_to_id(TF_ID_PROFSET_CONVERTFROM, "Shift_JIS");
	if (ctx_id < 0) {
		tf_close_ctx(parser->ctx);
		free(parser);
		return NULL;
	}
	err = tf_set_profile(parser->ctx, ctx_id);
	if (err < 0) {
		tf_close_ctx(parser->ctx);
		free(parser);
		return NULL;
	}

	parser->i = 0;
	parser->isLSTN = False;
	parser->state = STATE_SEARCHTAG;

	parser->itembuffer = bbsmnparser_item_new();
	if (parser->itembuffer == NULL) {
		tf_close_ctx(parser->ctx);
		free(parser);
		return NULL;
	}

	tokenchecker_initialize(&(parser->tokenchecker), nList_element, eToken_element);

	return parser;
}

EXPORT VOID bbsmnparser_delete(bbsmnparser_t *parser)
{
	bbsmnparser_item_delete(parser->itembuffer);
	tf_close_ctx(parser->ctx);
	free(parser);
}
