/*
 * subjectparser.c
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

#include    "subjectparser.h"
#include    "parselib.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct sbjtparser_t_ {
	sbjtcache_t *cache;
	TF_CTX ctx;
	W i;
	/**/
	Bool isLSTN;
	enum {
		STATE_NUMBER,
		STATE_TITLE
	} state;
	enum {
		STRSTATE_START,
		STRSTATE_CHARREF
	} strstate;
	charreferparser_t charref;
	sbjtparser_thread_t *threadbuffer;
};

EXPORT sbjtparser_thread_t* sbjtparser_thread_new()
{
	sbjtparser_thread_t *thr;

	thr = malloc(sizeof(sbjtparser_thread_t));
	if (thr == NULL) {
		return NULL;
	}
	thr->number = malloc(sizeof(UB)*1);
	thr->number_len = 0;
	thr->number[0] = '\0';
	thr->title = NULL;
	thr->title_len = 0;

	return thr;
}

LOCAL VOID sbjtparser_thread_clear(sbjtparser_thread_t *thr)
{
	if (thr->number != NULL) {
		free(thr->number);
	}
	if (thr->title != NULL) {
		free(thr->title);
	}
	thr->number = malloc(sizeof(UB)*1);
	thr->number_len = 0;
	thr->number[0] = '\0';
	thr->title = NULL;
	thr->title_len = 0;
}

EXPORT VOID sbjtparser_thread_delete(sbjtparser_thread_t *thr)
{
	if (thr->number != NULL) {
		free(thr->number);
	}
	if (thr->title != NULL) {
		free(thr->title);
	}
	free(thr);
}

LOCAL W sbjtparser_convert_str(sbjtparser_t *parser, const UB *src, W slen, UW attr, TC **dest, W *dlen)
{
	TC ch;
	W dest_len = 1, err;

	err = tf_strtotcs(parser->ctx, src, slen, attr, &ch, &dest_len);
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
		return 0;
	}
	for (;;) {
		dest_len = 1;
		err = tf_strtotcs(parser->ctx, NULL, slen, attr, &ch, &dest_len);
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
			break;
		}
	}

	return 0;
}

LOCAL W sbjtparser_parsechar(sbjtparser_t *parser, UB ch, sbjtparser_thread_t *thr)
{
	TC **str;
	W *len;
	UB chref;
	charreferparser_result_t chref_result;

	str = &(thr->title);
	len = &(thr->title_len);

	switch(ch) {
	  case '\n':
		if (parser->state == STATE_TITLE) {
			sbjtparser_convert_str(parser, NULL, 0, TF_ATTR_SUPPRESS_FUSEN, str, len);
		}
		parser->state = STATE_NUMBER;
		parser->strstate = STRSTATE_START;
		return 1; /* TODO */
		break;
	  case '<':
		parser->isLSTN= True;
		break;
	  case '>':
		if (parser->isLSTN == False) {
			/* temporary */
			if (parser->state == STATE_TITLE) {
				sbjtparser_convert_str(parser, ">", 1, TF_ATTR_SUPPRESS_FUSEN, str, len);
			}
			break;
		} else {
			parser->isLSTN = False;
		}
		switch (parser->state) {
		  case STATE_NUMBER:
			parser->state = STATE_TITLE;
			break;
		  case STATE_TITLE:
			parser->state = STATE_NUMBER;
			break;
		}
		sbjtparser_convert_str(parser, NULL, 0, TF_ATTR_SUPPRESS_FUSEN, str, len);
		parser->strstate = STRSTATE_START;
		break;
	  case '&':
		if ((parser->state == STATE_TITLE)&&(parser->strstate == STRSTATE_START)) {
			charreferparser_resetstate(&(parser->charref));
			chref_result = charreferparser_parsechar(&(parser->charref), ch);
			if (chref_result != CHARREFERPARSER_RESULT_CONTINUE) {
				break;
			}
			parser->strstate = STRSTATE_CHARREF;
			break;
		}
	  default:
		if (parser->isLSTN == True) {
			parser->isLSTN = False;
			/* temporary */
			if (parser->state == STATE_TITLE) {
				sbjtparser_convert_str(parser, "<", 1, TF_ATTR_SUPPRESS_FUSEN, str, len);
			}
		}
		switch (parser->state) {
		  case STATE_NUMBER:
			if (!isdigit(ch)) {
				break;
			}
			thr->number_len++;
			thr->number = realloc(thr->number, thr->number_len + 1);
			strncat(thr->number, &ch, 1);
			break;
		  case STATE_TITLE:
			if (parser->strstate == STRSTATE_START) {
				sbjtparser_convert_str(parser, &ch, 1, TF_ATTR_CONT|TF_ATTR_SUPPRESS_FUSEN, str, len);
			} else if (parser->strstate == STRSTATE_CHARREF) {
				chref_result = charreferparser_parsechar(&(parser->charref), ch);
				if (chref_result == CHARREFERPARSER_RESULT_DETERMINE) {
					chref = charreferparser_getcharnumber(&(parser->charref));
					sbjtparser_convert_str(parser, &chref, 1, TF_ATTR_CONT|TF_ATTR_SUPPRESS_FUSEN, str, len);
					parser->strstate = STRSTATE_START;
				}
			}
			break;
		}
	}

	return 0; /* TODO */
}

EXPORT W sbjtparser_getnextthread(sbjtparser_t *parser, sbjtparser_thread_t **thr)
{
	sbjtcache_datareadcontext_t *context;
	Bool cont;
	UB *bin_cache;
	W len_cache, i, err = 0;

	*thr = NULL;

	context = sbjtcache_startdataread(parser->cache, parser->i);
	if (context == NULL) {
		return -1; /* TODO */
	}

	for (;;) {
		cont = sbjtcache_datareadcontext_nextdata(context, &bin_cache, &len_cache);
		if (cont == False) {
			break;
		}

		for (i = 0; i < len_cache; i++) {
			err = sbjtparser_parsechar(parser, bin_cache[i], parser->threadbuffer);
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
			*thr = parser->threadbuffer;
			parser->threadbuffer = sbjtparser_thread_new();
			break;
		}
	}

	sbjtcache_enddataread(parser->cache, context);

	return err;
}

EXPORT VOID sbjtparser_clear(sbjtparser_t *parser)
{
	TC ch;
	W len = 1, err;

	err = tf_strtotcs(parser->ctx, NULL, 0, TF_ATTR_START, &ch, &len);
	if (err != 0) {
		DP_ER("tf_strtotcs (clear)", err);
	}

	parser->i = 0;
	parser->isLSTN = False;
	parser->state = STATE_NUMBER;

	sbjtparser_thread_clear(parser->threadbuffer);
}

EXPORT sbjtparser_t* sbjtparser_new(sbjtcache_t *cache)
{
	sbjtparser_t *parser;
	W err, ctx_id;

	parser = malloc(sizeof(sbjtparser_t));
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
	parser->state = STATE_NUMBER;
	parser->strstate = STRSTATE_START;

	parser->threadbuffer = sbjtparser_thread_new();
	if (parser->threadbuffer == NULL) {
		tf_close_ctx(parser->ctx);
		free(parser);
		return NULL;
	}
	charreferparser_initialize(&(parser->charref));

	return parser;
}

EXPORT VOID sbjtparser_delete(sbjtparser_t *parser)
{
	charreferparser_finalize(&(parser->charref));
	sbjtparser_thread_delete(parser->threadbuffer);
	tf_close_ctx(parser->ctx);
	free(parser);
}
