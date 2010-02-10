/*
 * bbsmenufilter.h
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

#include    "bbsmenuparser.h"

#ifndef __BBSMENUFILTER_H__
#define __BBSMENUFILTER_H__

typedef struct bbsmnfilter_t_ bbsmnfilter_t;

IMPORT bbsmnfilter_t* bbsmnfilter_new();
IMPORT VOID bbsmnfilter_delete(bbsmnfilter_t *filter);
IMPORT VOID bbsmnfilter_inputitem(bbsmnfilter_t *filter, bbsmnparser_item_t *item);
#define BBSMNFILTER_OUTPUTITEM_CONTINUE 0
#define BBSMNFILTER_OUTPUTITEM_WAITNEXT 1
#define BBSMNFILTER_OUTPUTITEM_END      2
IMPORT W bbsmnfilter_outputitem(bbsmnfilter_t *filter, bbsmnparser_item_t **item);
IMPORT VOID bbsmnfilter_clear(bbsmnfilter_t *filter);

#endif
