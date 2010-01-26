/*
 * bbsmenufilter.c
 *
 * Copyright (c) 2010 project bchan
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

#include    "bbsmenufilter.h"
#include    "bbsmenuparser.h"

#ifdef BCHANL_CONFIG_DEBUG
# define DP(arg) printf arg
# define DP_ER(msg, err) printf("%s (%d/%x)\n", msg, err>>16, err)
#else
# define DP(arg) /**/
# define DP_ER(msg, err) /**/
#endif

struct bbsmnfilter_t_ {
	enum {
		STATE_REJECT_TITLE,
		STATE_HAVING_CATEGORY,
		STATE_HAVING_TITLE,
		STATE_END
	} state;
	bbsmnparser_item_t *category;
	bbsmnparser_item_t *title;
};

EXPORT VOID bbsmnfilter_inputitem(bbsmnfilter_t *filter, bbsmnparser_item_t *item)
{
	Bool ok;

	if (filter->state == STATE_REJECT_TITLE) {
		if (item == NULL) {
			filter->state = STATE_END;
			return;
		}
		if (item->title != NULL) {
			bbsmnparser_item_delete(item);
			return;
		}
		filter->category = item;
		filter->state = STATE_HAVING_CATEGORY;
		return;
	}
	if (filter->state == STATE_HAVING_CATEGORY) {
		if (item == NULL) {
			bbsmnparser_item_delete(filter->category);
			filter->category = NULL;
			filter->state = STATE_END;
			return;
		}
		if (item->category != NULL) {
			bbsmnparser_item_delete(filter->category);
			filter->category = item;
			return;
		}
		ok = bbsmnparser_item_checkboradurl(item);
		if (ok == False) {
			bbsmnparser_item_delete(item);
			return;
		}
		filter->title = item;
		filter->state = STATE_HAVING_TITLE;
		return;
	}
	if (filter->state == STATE_HAVING_TITLE) {
		if (item == NULL) {
			filter->state = STATE_END;
			return;
		}
		if (item->title != NULL) {
			ok = bbsmnparser_item_checkboradurl(item);
			if (ok == False) {
				bbsmnparser_item_delete(item);
				return;
			}
			filter->title = item;
			return;
		}
		filter->category = item;
		filter->state = STATE_HAVING_CATEGORY;
		return;
	}

	if (item != NULL) {
		bbsmnparser_item_delete(item);
	}
}

EXPORT W bbsmnfilter_outputitem(bbsmnfilter_t *filter, bbsmnparser_item_t **item)
{
	*item = NULL;

	switch (filter->state) {
	case STATE_REJECT_TITLE:
		return BBSMNFILTER_OUTPUTITEM_WAITNEXT;
	case STATE_HAVING_CATEGORY:
		if (filter->title != NULL) {
			*item = filter->title;
			filter->title = NULL;
		}
		return BBSMNFILTER_OUTPUTITEM_WAITNEXT;
	case STATE_HAVING_TITLE:
		if (filter->category != NULL) {
			*item = filter->category;
			filter->category = NULL;
			return BBSMNFILTER_OUTPUTITEM_CONTINUE;
		}
		if (filter->title != NULL) {
			*item = filter->title;
			filter->title = NULL;
		}
		return BBSMNFILTER_OUTPUTITEM_WAITNEXT;
	case STATE_END:
		if (filter->title != NULL) {
			*item = filter->title;
			filter->title = NULL;
		}
		return BBSMNFILTER_OUTPUTITEM_END;
	}

	return BBSMNFILTER_OUTPUTITEM_WAITNEXT;
}

EXPORT VOID bbsmnfilter_reset(bbsmnfilter_t *filter)
{
	if (filter->title != NULL) {
		bbsmnparser_item_delete(filter->title);
	}
	if (filter->category != NULL) {
		bbsmnparser_item_delete(filter->category);
	}
	filter->state = STATE_REJECT_TITLE;
}

EXPORT bbsmnfilter_t* bbsmnfilter_new()
{
	bbsmnfilter_t *filter;

	filter = (bbsmnfilter_t*)malloc(sizeof(bbsmnfilter_t));
	if (filter == NULL) {
		return NULL;
	}
	filter->state = STATE_REJECT_TITLE;
	filter->category = NULL;
	filter->title = NULL;

	return filter;
}

EXPORT VOID bbsmnfilter_delete(bbsmnfilter_t *filter)
{
	if (filter->title != NULL) {
		bbsmnparser_item_delete(filter->title);
	}
	if (filter->category != NULL) {
		bbsmnparser_item_delete(filter->category);
	}
	free(filter);
}

