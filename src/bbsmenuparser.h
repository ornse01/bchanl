/*
 * bbsmenuparser.h
 *
 * Copyright (c) 2009-2015 project bchan
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

#include    <basic.h>
#include    "bbsmenucache.h"

#ifndef __BBSMENUPARSER_H__
#define __BBSMENUPARSER_H__

struct bbsmnparser_item_t_ {
	TC *category;
	W category_len;
	UB *url;
	W url_len;
	TC *title;
	W title_len;
};
typedef struct bbsmnparser_item_t_ bbsmnparser_item_t;
typedef struct bbsmnparser_t_ bbsmnparser_t;

IMPORT VOID bbsmnparser_item_delete(bbsmnparser_item_t *item);
IMPORT Bool bbsmnparser_item_checkboradurl(bbsmnparser_item_t *item);
IMPORT VOID bbsmnparser_item_gethostboard(bbsmnparser_item_t *item, UB **host, W *host_len, UH *port, UB **board, W *board_len);

IMPORT bbsmnparser_t* bbsmnparser_new(bbsmncache_t *cache);
IMPORT VOID bbsmnparser_delete(bbsmnparser_t *parser);
IMPORT W bbsmnparser_getnextitem(bbsmnparser_t *parser, bbsmnparser_item_t **item);
IMPORT VOID bbsmnparser_clear(bbsmnparser_t *parser);
IMPORT bbsmnparser_item_t* bbsmnparser_newcategoryitem(bbsmnparser_t *parser, TC *category, W category_len);
IMPORT bbsmnparser_item_t* bbsmnparser_newboarditem(bbsmnparser_t *parser, TC *title, W title_len, UB *url, W url_len);

#endif
